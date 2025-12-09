#pragma once

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

struct Scene 
{
	ECS::Registry registry;
	ECS::EntityHandle root;

	Phys::PhysicsSystem phys_system;
	std::unordered_map<JPH::BodyID, ECS::EntityHandle> pbody_to_ent;

	Scene() noexcept
	{
		root = registry.createEntity();
		registry.emplaceComponent<HierarchyComponent>(
			root,
			"Root Node"
		);
		setParent(root, { registry.tombstone() });
	}

	Scene(Scene&& other) noexcept
	{
		registry = std::move(other.registry);
		root = std::move(other.root);
	}

	Scene& operator=(Scene&& other) noexcept
	{
		registry = std::move(other.registry);
		root = std::move(other.root);

		return *this;
	}

	ECS::EntityHandle createEntity(
		ECS::EntityHandle parent,
		std::string tag = "") noexcept
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

	void setParent(ECS::EntityHandle child, ECS::EntityHandle parent) noexcept
	{
		auto& h_child = registry.getComponent<HierarchyComponent>(child);
		
		// Remove child from old parent
		auto old_par = h_child.parent;
		if (registry.hasComponent<HierarchyComponent>(old_par)) {
			auto& h_old_par = registry.getComponent<HierarchyComponent>(old_par);

			std::vector<ECS::EntityHandle> rem_children;
			for (auto par_child : h_old_par.children) {
				if (par_child != child) {
					rem_children.push_back(par_child);
				}
			}
			h_old_par.children = rem_children;
		}

		// Add child to new parent
		if (registry.hasComponent<HierarchyComponent>(parent)) {
			auto& h_par = registry.getComponent<HierarchyComponent>(parent);

			h_par.children.push_back(child);
		}

		// Assign new parent to child
		h_child.parent = parent;
	}

	void setTag(ECS::EntityHandle ent, const std::string& tag) noexcept
	{
		auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
		hierarchy.tag = tag;
	}

	ECS::EntityHandle getParent(ECS::EntityHandle ent) noexcept
	{
		auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
		return hierarchy.parent;
	}

	std::vector<ECS::EntityHandle> getChildren(ECS::EntityHandle ent) noexcept
	{
		auto& hierarchy = registry.getComponent<HierarchyComponent>(ent);
		return hierarchy.children;
	}

	TransformComponent getWorldTransform(ECS::EntityHandle ent) noexcept
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

	glm::mat4 getModelMatrix(ECS::EntityHandle ent) noexcept
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
		while (!stack.empty()) {
			model_mat *= stack.back();
			stack.pop_back();
		}
		return model_mat;
	}

	ECS::EntityHandle addModel(
		AssetHandle<Render::Model> model_handle,
		const TransformComponent& transform = {}) noexcept
	{
		APE_CHECK((model_handle.data != nullptr),
			"Scene::addModel() Failed: model_handle data is null."
		);

		ECS::EntityHandle model_ent = createEntity(root);
		auto& model_hierarchy = registry.getComponent<HierarchyComponent>(model_ent);
		model_hierarchy.tag = std::format("Model {}", model_ent.id);
		// registry.replaceComponent<HierarchyComponent>(
		// 	model_ent,
		// 	std::format("Model {}", model_ent.id)
		// );

		registry.emplaceComponent<TransformComponent>(
			model_ent,
			transform
		);

		auto& model = model_handle.data;
		for (size_t idx = 0; idx < model->meshes.size(); ++idx) {
			auto& mesh = model->meshes[idx];

			ECS::EntityHandle ent = createEntity(model_ent, std::format("Mesh {}", idx));
			// registry.replaceComponent<HierarchyComponent>(
			// 	ent,
			// 	std::format("Mesh {}", idx)
			// );

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

	JPH::BodyID createPhysicsBody(JPH::BodyCreationSettings body_settings) noexcept
	{
		auto& body_if = phys_system.phys_system.GetBodyInterface();
		auto body_id = body_if.CreateAndAddBody(
			body_settings,
			JPH::EActivation::Activate
		);

		return body_id;
	}

	JPH::BodyID createAndRegisterPhysicsBody(
		ECS::EntityHandle ent,
		JPH::BodyCreationSettings body_settings
	) noexcept
	{
		auto body_id = createPhysicsBody(body_settings);

		pbody_to_ent[body_id] = ent;
		registry.emplaceComponent<Phys::PhysicsComponent>(ent, body_id);

		return body_id;
	}

	ECS::EntityHandle getPhysicsBodyEntity(JPH::BodyID body_id) const noexcept
	{
		return pbody_to_ent.at(body_id);
	}
};

};	// end of namespace

