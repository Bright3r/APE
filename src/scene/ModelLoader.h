#pragma once

#include "render/Model.h"
#include "scene/AssetHandle.h"

#include "assimp/Importer.hpp"
#include <assimp/material.h>
#include <assimp/scene.h>

namespace APE {

class ModelLoader {
public:
	[[nodiscard]] static AssetHandle<Render::Model> 
	load(std::filesystem::path model_path) noexcept;

	[[nodiscard]] static AssetHandle<Render::Model> 
	load(AssetKey asset_key) noexcept;

	[[nodiscard]] static AssetHandle<Render::Model> 
	defaultModel() noexcept;

private:
	[[nodiscard]] static TransformComponent 
	convertAiTransform(const aiMatrix4x4 ai_transform) noexcept;

	[[nodiscard]] static AssetHandle<Render::Image> 
	convertAiMaterial(
		const aiMaterial* ai_mat,
		const aiScene* scene,
		std::filesystem::path model_path) noexcept;

	void static processNode(
		const aiNode* node,
		const aiScene* scene,
		Render::Model& model,
		std::filesystem::path model_path) noexcept;

	[[nodiscard]] static Render::Model::ModelMesh 
	processAiMesh(
		const aiMesh* ai_mesh,
		const AssetHandle<Render::Image>& texture_handle,
		const TransformComponent& transform) noexcept;
};

};	// end of namespace

