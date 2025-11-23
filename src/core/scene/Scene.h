#pragma once

#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/render/Model.h"
#include "phys/Physics.h"

#include <Jolt/Physics/Body/BodyID.h>

#include <format>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

namespace APE {

struct Scene {
	Phys::PhysicsSystem phys_system;
	ECS::Registry registry;
	ECS::EntityHandle root;

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

	void setParent(ECS::EntityHandle child, ECS::EntityHandle parent) noexcept
	{
		auto& h_child = registry.getComponent<HierarchyComponent>(child);
		
		// Remove child from old parent
		auto old_par = h_child.parent;
		if (registry.hasComponent<HierarchyComponent>(old_par)) {
			auto& h_old_par = 
				registry.getComponent<HierarchyComponent>(old_par);

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
			auto& h_par = 
				registry.getComponent<HierarchyComponent>(parent);

			h_par.children.push_back(child);
		}

		// Assign new parent to child
		h_child.parent = parent;
	}

	glm::mat4 getModelMatrix(ECS::EntityHandle ent) noexcept
	{
		std::vector<glm::mat4> stack;
		while (registry.hasAllComponents<
			TransformComponent, HierarchyComponent>(ent)) 
		{
			auto [transform, hierarchy] = registry.getComponents<
				TransformComponent, 
				HierarchyComponent>(ent);

			stack.emplace_back(transform.getModelMatrix());
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

		ECS::EntityHandle par = registry.createEntity();
		registry.emplaceComponent<HierarchyComponent>(
			par,
			std::format("Model {}", par.id)
		);
		setParent(par, root);

		registry.emplaceComponent<TransformComponent>(
			par,
			transform
		);

		auto& model = model_handle.data;
		for (size_t idx = 0; idx < model->meshes.size(); ++idx) {
			auto& mesh = model->meshes[idx];

			ECS::EntityHandle ent = registry.createEntity();
			registry.emplaceComponent<HierarchyComponent>(
				ent,
				std::format("Mesh {}", idx)
			);
			setParent(ent, par);

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
		return par;
	}

	void registerPhysicsBody(ECS::EntityHandle ent, JPH::BodyID body_id) noexcept
	{
		registry.emplaceComponent<Phys::PhysicsComponent>(ent, body_id);
		pbody_to_ent[body_id] = ent;
	}

	ECS::EntityHandle getPhysicsBodyEntity(JPH::BodyID body_id) const noexcept
	{
		return pbody_to_ent.at(body_id);
	}
};

};	// end of namespace

