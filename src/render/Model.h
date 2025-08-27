#pragma once

#include "render/Mesh.h"
#include "render/Transform.h"
#include "render/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <vector>

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
	Transform m_transform;

public:
	Model() noexcept = default;

	Model(std::filesystem::path model_path,
		const Transform& transform = {}) noexcept;

	virtual ~Model() noexcept { }

	void loadModel(std::filesystem::path model_path) noexcept;

	std::vector<ModelMesh>& getMeshes() noexcept;

	[[nodiscard]] Transform& getTransform() noexcept;

	void setTransform(const Transform& transform) noexcept;

private:
	[[nodiscard]] static Transform convertAiTransform(
		const aiMatrix4x4 ai_transform) noexcept;

	[[nodiscard]] static std::shared_ptr<Image> convertAiMaterial(
		const aiMaterial* ai_mat,
		const aiScene* scene,
		std::filesystem::path model_path) noexcept;

	void processNode(
		const aiNode* node,
		const aiScene* scene,
		std::filesystem::path model_path) noexcept;

	[[nodiscard]] Model::ModelMesh processAiMesh(
		const aiMesh* ai_mesh,
		std::shared_ptr<Image> texture,
		const Transform& transform) const noexcept;
};

};	// end of namespace

