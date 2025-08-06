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

using VertexType = PositionColorVertex;
using IndexType = Uint16;
using MeshType = Mesh<VertexType, IndexType>;

class Model {
private:
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
	void processNode(const aiNode* node, const aiScene* scene);

	MeshType processAiMesh(const aiMesh* ai_mesh) const;
};

};	// end of namespace Render
};	// end of namespace APE
