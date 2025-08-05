#include "render/Model.h"
#include "util/Logger.h"
#include <SDL3/SDL_stdinc.h>
#include <cstdlib>
#include <vector>

namespace APE {
namespace Render {

Model::Model(std::filesystem::path model_path, const Transform& transform)
	: m_meshes()
	, m_transform(transform)
{
	loadModel(model_path);
}

void Model::loadModel(std::filesystem::path model_path)
{
	m_meshes.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		model_path,
		aiProcess_Triangulate | aiProcess_FlipUVs
	);

	bool succ_load_model = !(!scene 
		|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE 
		|| !scene->mRootNode);
	if (!succ_load_model) {
		APE_ERROR(
			"Model::Model(const std::filesystem::path& model_path) \
			Failed - {}",
			importer.GetErrorString()
		);
		return;
	}

	processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode* node, const aiScene* scene)
{
	// Convert nodes aiMeshes into our own Meshes
	for (size_t i = 0; i < node->mNumMeshes; ++i) {
		// Get aiMesh
		unsigned int mesh_idx = node->mMeshes[i];
		aiMesh* ai_mesh = scene->mMeshes[mesh_idx];

		// Add Mesh to model
		m_meshes.emplace_back(processAiMesh(ai_mesh));
	}

	// Process child nodes
	for (size_t i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processAiMesh(const aiMesh* ai_mesh) const
{
	// Get vertices
	std::vector<PositionColorVertex> vertices;
	for (size_t i = 0; i < ai_mesh->mNumVertices; ++i) {
		// Get vertex pos
		aiVector3D ai_vertex = ai_mesh->mVertices[i];

		// Get texture coords
		float u {};
		float v {};
		if (ai_mesh->mTextureCoords[0]) {
			u = ai_mesh->mTextureCoords[0][i].x;
			v = ai_mesh->mTextureCoords[0][i].y;
		}

		// Add vertex
		Uint8 r = rand() % 255;
		Uint8 g = rand() % 255;
		Uint8 b = rand() % 255;
		Uint8 a = 255;
		vertices.push_back({
			ai_vertex.x,
			ai_vertex.y,
			ai_vertex.z,
			r, g, b, a
		});
	}

	// Get indices for each face
	std::vector<VertexIndex> indices;
	for (size_t i = 0; i < ai_mesh->mNumFaces; ++i) {
		// Process face's indices
		aiFace face = ai_mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return Mesh(vertices, indices);
}

std::vector<Mesh>& Model::getMeshes()
{
	return m_meshes;
}

Transform& Model::getTransform()
{
	return m_transform;
}

void Model::setTransform(const Transform& transform)
{
	m_transform = transform;
}


};	// end of namespace Render
};	// end of namespace APE

