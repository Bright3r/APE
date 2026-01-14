#include "core/scene/Scene.h"
#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/render/Model.h"
#include "phys/Physics.h"

#include <format>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

namespace APE 
{

Scene::Scene() noexcept
{
	root = registry.createEntity();
	registry.emplaceComponent<HierarchyComponent>(
		root,
		"Root Node"
	);
	setParent(root, { registry.tombstone() });

	phys_system = std::make_unique<Phys::PhysicsSystem>();
}

Scene::Scene(Scene&& other) noexcept
	: registry(std::move(other.registry))
	, root(std::move(other.root))
	, pbody_to_ent(std::move(other.pbody_to_ent))
	, phys_system(std::move(other.phys_system))
{

}

Scene& Scene::operator=(Scene&& other) noexcept
{
	registry = std::move(other.registry);
	root = std::move(other.root);
	pbody_to_ent = std::move(other.pbody_to_ent);
	phys_system = std::move(other.phys_system);

	return *this;
}

ECS::EntityHandle Scene::createEntity(
	ECS::EntityHandle parent,
	std::string tag
) noexcept
{
	auto par_tag = registry.getComponent<HierarchyComponent>(parent);
	if (tag.empty())
	{
		tag = "Child " + std::to_string(par_tag.children.size());
	}

	auto ent = registry.createEntity();
	registry.emplaceComponent<HierarchyComponent>(ent, tag);
	setParent(ent, parent);

	return ent;
}

void Scene::setParent(ECS::EntityHandle child, ECS::EntityHandle parent) noexcept
{
	auto& h_child = registry.getComponent<HierarchyComponent>(child);
	
	// Remove child from old parent
	auto old_par = h_child.parent;
	if (registry.hasComponent<HierarchyComponent>(old_par)) 
	{
		auto& h_old_par = registry.getComponent<HierarchyComponent>(old_par);

		std::vector<ECS::EntityHandle> rem_children;
		for (auto par_child : h_old_par.children) 
		{
			if (par_child != child) 
			{
				rem_children.push_back(par_child);
			}
		}
		h_old_par.children = rem_children;
	}

	// Add child to new parent
	if (registry.hasComponent<HierarchyComponent>(parent)) 
	{
		auto& h_par = registry.getComponent<HierarchyComponent>(parent);

		h_par.children.push_back(child);
	}

	// Assign new parent to child
	h_child.parent = parent;
}

void Scene::setTag(ECS::EntityHandle ent, const std::string& tag) noexcept
{
	auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
	hierarchy.tag = tag;
}

ECS::EntityHandle Scene::getParent(ECS::EntityHandle ent) noexcept
{
	auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
	return hierarchy.parent;
}

std::vector<ECS::EntityHandle> Scene::getChildren(ECS::EntityHandle ent) noexcept
{
	auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
	return hierarchy.children;
}

TransformComponent Scene::getWorldTransform(ECS::EntityHandle ent) noexcept
{
	auto model = getModelMatrix(ent);

	TransformComponent transform {};
	transform.position = glm::vec3(model[3]);
	transform.scale = glm::vec3(
		glm::length(glm::vec3(model[0][0], model[0][1], model[0][2])),
		glm::length(glm::vec3(model[1][0], model[1][1], model[1][2])),
		glm::length(glm::vec3(model[2][0], model[2][1], model[2][2]))
	);

	glm::mat3 r {};
	r[0] = glm::vec3(model[0][0], model[0][1], model[0][2]) / transform.scale.x;
	r[1] = glm::vec3(model[1][0], model[1][1], model[1][2]) / transform.scale.y;
	r[2] = glm::vec3(model[2][0], model[2][1], model[2][2]) / transform.scale.z;
	transform.rotation = glm::quat_cast(r);

	return transform;
}

glm::mat4 Scene::getModelMatrix(ECS::EntityHandle ent) noexcept
{
	std::vector<glm::mat4> stack;
	while (registry.hasComponent<HierarchyComponent>(ent))
	{
		TransformComponent transform {};
		if (registry.hasComponent<TransformComponent>(ent))
		{
			transform = registry.getComponent<TransformComponent>(ent);
		}
		stack.emplace_back(transform.getModelMatrix());

		auto hierarchy = registry.getComponent<HierarchyComponent>(ent);
		ent = hierarchy.parent;
	}

	glm::mat4 model_mat(1.f);
	while (!stack.empty()) 
	{
		model_mat *= stack.back();
		stack.pop_back();
	}
	return model_mat;
}

ECS::EntityHandle Scene::addModel(
	AssetHandle<Render::Model> model_handle,
	const TransformComponent& transform
) noexcept
{
	APE_CHECK((model_handle.data != nullptr),
		"Scene::addModel() Failed: model_handle data is null."
	);

	ECS::EntityHandle model_ent = createEntity(root);
	auto& model_hierarchy = registry.getComponent<HierarchyComponent>(model_ent);
	model_hierarchy.tag = std::format("Model {}", model_ent.id);

	registry.emplaceComponent<TransformComponent>(
		model_ent,
		transform
	);

	auto& model = model_handle.data;
	for (size_t idx = 0; idx < model->meshes.size(); ++idx) 
	{
		auto& mesh = model->meshes[idx];

		ECS::EntityHandle ent = createEntity(model_ent, std::format("Mesh {}", idx));

		registry.emplaceComponent<Render::MeshComponent>(
			ent,
			model_handle,
			idx
		);
		registry.emplaceComponent<Render::MaterialComponent>(
			ent,
			mesh.texture_handle
		);
		registry.emplaceComponent<TransformComponent>(
			ent,
			mesh.transform
		);
	}

	return model_ent;
}

JPH::BodyID Scene::createPhysicsBody(JPH::BodyCreationSettings body_settings) noexcept
{
	auto& body_if = phys_system->phys_system.GetBodyInterface();
	auto body_id = body_if.CreateAndAddBody(
		body_settings,
		JPH::EActivation::Activate
	);

	return body_id;
}

JPH::BodyID Scene::createAndRegisterPhysicsBody(
	ECS::EntityHandle ent,
	JPH::BodyCreationSettings body_settings
) noexcept
{
	auto body_id = createPhysicsBody(body_settings);

	pbody_to_ent[body_id] = ent;
	registry.emplaceOrReplaceComponent<Phys::PhysicsComponent>(ent, body_id);

	return body_id;
}

void Scene::removePhysicsBody(ECS::EntityHandle ent) noexcept
{
	auto phys_comp = registry.getComponent<Phys::PhysicsComponent>(ent);

	auto& body_if = phys_system->phys_system.GetBodyInterface();
	body_if.RemoveBody(phys_comp.body_id);
	body_if.DestroyBody(phys_comp.body_id);

	registry.removeComponent<Phys::PhysicsComponent>(ent);
}

ECS::EntityHandle Scene::getPhysicsBodyEntity(JPH::BodyID body_id) const noexcept
{
	auto ent = pbody_to_ent.at(body_id);
	APE_TRACE("Selected Entity: {}", ent.id);

	return ent;
}

ECS::EntityHandle Scene::addPlayer(
	AssetHandle<Render::Model> model_handle,
	const TransformComponent& transform,
	JPH::Shape *shape
) noexcept
{
	JPH::CharacterVirtualSettings settings;
	settings.mShape = shape;
	settings.mInnerBodyLayer = Phys::Layers::MOVING;
	Phys::PlayerComponent player_comp { 
		Phys::PlayerController(
			settings,
			transform,
			*phys_system.get()
		)
	};

	auto player_ent = addModel(model_handle, transform);
	setTag(player_ent, "Player");
	registry.emplaceComponent<Phys::PlayerComponent>(
		player_ent,
		std::move(player_comp)
	);

	return player_ent;
}

std::pair<ECS::EntityHandle, Phys::PlayerComponent*> Scene::getPlayer() noexcept
{
	auto view = registry.view<Phys::PlayerComponent>().each();

	if (view.empty()) return { registry.tombstone(), nullptr };

	auto& [player_ent, player] = view.at(0);
	return { player_ent, &player };
}

std::pair<ECS::EntityHandle, Render::CameraComponent*> Scene::getCamera(
	ECS::EntityHandle parent
) noexcept
{
	ECS::EntityHandle cam_ent {};
	bool has_cam = false;
	auto children = getChildren(parent);
	for (auto& child : children)
	{
		if (registry.hasComponent<Render::CameraComponent>(child))
		{
			cam_ent = child;
			has_cam = true;
			break;
		}
	}

	if (!has_cam) return { registry.tombstone(), nullptr };

	auto& cam_comp = registry.getComponent<Render::CameraComponent>(cam_ent);
	return { cam_ent, &cam_comp };
}

void Scene::stepPhysics(float dt) noexcept
{
	phys_system->update(dt);
}

void Scene::stepPlayer(float dt) noexcept
{
	auto [player_ent, player] = getPlayer();
	player->controller.update(dt, *phys_system);
}

void Scene::syncWithPhysicsState() noexcept
{
	auto& body_if = phys_system->phys_system.GetBodyInterface();

	// Sync transforms
	auto view = registry.view<TransformComponent, Phys::PhysicsComponent>();
	for (auto& [ent, transform, pbody] : view.each())
	{
		auto pos = body_if.GetPosition(pbody.body_id);
		transform.position.x = pos.GetX();
		transform.position.y = pos.GetY();
		transform.position.z = pos.GetZ();

		auto rot = body_if.GetRotation(pbody.body_id);
		transform.rotation.x = rot.GetX();
		transform.rotation.y = rot.GetY();
		transform.rotation.z = rot.GetZ();
		transform.rotation.w = rot.GetW();
	}

	// Try to sync player
	auto [player_ent, player] = getPlayer();
	if (player_ent != registry.tombstone())
	{
		auto player_pos = player->controller.body->GetPosition();
		auto player_rot = player->controller.body->GetRotation();

		auto& player_transform = registry.getComponent<TransformComponent>(player_ent);
		player_transform.position.x = player_pos.GetX();
		player_transform.position.y = player_pos.GetY();
		player_transform.position.z = player_pos.GetZ();

		player_transform.rotation.x = player_rot.GetX();
		player_transform.rotation.y = player_rot.GetY();
		player_transform.rotation.z = player_rot.GetZ();
		player_transform.rotation.w = player_rot.GetW();
	}
}

void Scene::syncPlayerCamera() noexcept
{
	auto [player_ent, player] = getPlayer();
	auto [cam_ent, player_cam] = getCamera(player_ent);

	auto cam_transform = getWorldTransform(cam_ent);
	player_cam->camera->setPosition(cam_transform.position);
}

};	// end of namespace

