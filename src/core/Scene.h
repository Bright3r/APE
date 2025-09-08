#pragma once

#include "components/Object.h"
#include "ecs/Registry.h"
#include "render/Model.h"

#include <filesystem>
#include <format>

namespace APE {

struct Scene {
	ECS::Registry registry;
	ECS::EntityHandle root;

	Scene() noexcept
	{
		root = registry.createEntity();
		registry.emplaceComponent<HierarchyComponent>(
			root,
			"Root Node"
		);
		setParent(root, { registry.tombstone() });
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

	glm::mat4 getModelMatrix(APE::ECS::EntityHandle ent) noexcept
	{
		std::vector<glm::mat4> stack;
		while (registry.hasAllComponents<
			APE::TransformComponent, APE::HierarchyComponent>(ent)) 
		{
			auto [transform, hierarchy] = registry.getComponents<
				APE::TransformComponent, 
				APE::HierarchyComponent>(ent);

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

	ECS::EntityHandle addModel(std::filesystem::path model_path,
		const TransformComponent& transform) noexcept
	{
		Render::Model model(model_path, transform);
		return addModel(&model);
	}

	ECS::EntityHandle addModel(Render::Model* model) noexcept
	{
		ECS::EntityHandle par = registry.createEntity();
		registry.emplaceComponent<HierarchyComponent>(
			par,
			std::format("Model {}", par.id)
		);
		setParent(par, root);

		registry.emplaceComponent<TransformComponent>(
			par,
			model->getTransform()
		);

		size_t child_num = 1;
		for (auto& mesh : model->getMeshes()) {
			ECS::EntityHandle ent = registry.createEntity();
			registry.emplaceComponent<HierarchyComponent>(
				ent,
				std::format("Mesh {}", child_num++)
			);
			setParent(ent, par);

			registry.emplaceComponent<Render::MeshComponent>(
				ent,
				mesh.toComponent()
			);
			registry.emplaceComponent<Render::MaterialComponent>(
				ent,
				mesh.getTexture()
			);
			registry.emplaceComponent<TransformComponent>(
				ent,
				mesh.getTransform()
			);
		}
		return par;
	}
};

};	// end of namespace

