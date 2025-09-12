#pragma once

#include "components/Object.h"
#include "core/AssetManager.h"
#include "render/Mesh.h"
#include "render/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <memory>
#include <vector>
#include <string>

namespace APE::Render {

class Model {
public:
	// Type aliases
	using VertexType = TextureVertex;
	using IndexType = Uint32;
	using ModelMesh = Mesh<VertexType, IndexType>;

protected:
	// Data members
	std::vector<ModelMesh> m_meshes;
	std::filesystem::path m_model_path;
	TransformComponent m_transform;

public:
	Model() noexcept = default;

	Model(std::filesystem::path model_path,
		const TransformComponent& transform = {}) noexcept;

	virtual ~Model() noexcept { }

	void loadModel(std::filesystem::path model_path) noexcept;

	[[nodiscard]] std::filesystem::path getPath() const noexcept;

	std::vector<ModelMesh>& getMeshes() noexcept;

	[[nodiscard]] TransformComponent& getTransform() noexcept;

	void setTransform(const TransformComponent& transform) noexcept;

private:
	[[nodiscard]] static AssetHandle<Image> defaultImageHandle() noexcept;

	[[nodiscard]] static AssetHandle<Image> 
	makeImageHandle(std::unique_ptr<Image> texture,
		 const std::string& handle_index) noexcept;

	[[nodiscard]] static TransformComponent convertAiTransform(
		const aiMatrix4x4 ai_transform) noexcept;

	[[nodiscard]] static AssetHandle<Image> convertAiMaterial(
		const aiMaterial* ai_mat,
		const aiScene* scene,
		std::filesystem::path model_path) noexcept;

	void processNode(
		const aiNode* node,
		const aiScene* scene,
		std::filesystem::path model_path) noexcept;

	[[nodiscard]] Model::ModelMesh processAiMesh(
		const aiMesh* ai_mesh,
		const AssetHandle<Image>& texture_handle,
		const TransformComponent& transform) const noexcept;
};

};	// end of namespace

