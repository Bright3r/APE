#pragma once

#include "render/Mesh.h"
#include "render/Transform.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <vector>

namespace APE {
namespace Render {

class Model {
private:
	std::vector<Mesh> m_meshes;
	Transform m_transform;

public:
	Model() = default;

	Model(std::filesystem::path model_path, const Transform& transform = {});

	void loadModel(std::filesystem::path model_path);

	std::vector<Mesh>& getMeshes();

	Transform& getTransform();

	void setTransform(const Transform& transform);

private:
	void processNode(const aiNode* node, const aiScene* scene);

	Mesh processAiMesh(const aiMesh* ai_mesh) const;
};

};	// end of namespace Render
};	// end of namespace APE
