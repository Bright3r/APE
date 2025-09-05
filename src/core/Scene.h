#pragma once

#include "components/Object.h"
#include "ecs/Registry.h"
#include "render/Model.h"

#include <filesystem>
#include <string>

namespace APE {

struct Scene {
	ECS::Registry registry;
	ECS::EntityHandle root;

	Scene() noexcept
	{
		root = registry.createEntity();
		registry.emplaceComponent<HierarchyComponent>(
			root,
			registry.tombstone(),
			"Root Node"
		);
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
			root,
			"HierarchyNeedsName"
		);
		registry.emplaceComponent<TransformComponent>(
			par,
			model->getTransform()
		);

		size_t child_num = 1;
		for (auto& mesh : model->getMeshes()) {
			ECS::EntityHandle ent = registry.createEntity();
			registry.emplaceComponent<HierarchyComponent>(
				ent,
				par,
				std::to_string(child_num++)
			);
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

