#pragma once

#include "render/Mesh.h"
#include "render/Transform.h"
#include "render/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <vector>

namespace APE {
namespace Render {

class Model {
public:
	// Type aliases
	using VertexType = TextureVertex;
	using IndexType = Uint16;
	using MeshType = Mesh<VertexType, IndexType>;

private:
	// Data members
	std::vector<MeshType> m_meshes;
	Transform m_transform;

public:
	Model() = default;

	Model(std::filesystem::path model_path, const Transform& transform = {});

	void loadModel(std::filesystem::path model_path);

	std::vector<MeshType>& getMeshes();

	Transform& getTransform();

	void setTransform(const Transform& transform);

private:
	static Transform convertAiTransform(const aiMatrix4x4 ai_transform);

	static std::shared_ptr<Image> convertAiMaterial(
		const aiMaterial* ai_mat,
		const aiScene* scene,
		std::filesystem::path model_path);

	void processNode(
		const aiNode* node,
		const aiScene* scene,
		const Transform& parent_transform,
		std::filesystem::path model_path);

	Model::MeshType processAiMesh(
		const aiMesh* ai_mesh,
		std::shared_ptr<Image> texture,
		const Transform& transform) const;
};

};	// end of namespace Render
};	// end of namespace APE
