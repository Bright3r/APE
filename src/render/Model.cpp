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

Model::MeshType Model::processAiMesh(const aiMesh* ai_mesh) const
{
	// Get vertices
	std::vector<VertexType> vertices;
	vertices.reserve(ai_mesh->mNumVertices);
	for (size_t i = 0; i < ai_mesh->mNumVertices; ++i) {
		aiVector3D ai_vertex = ai_mesh->mVertices[i];

		// Get vertex pos
		glm::vec3 pos { 
			ai_vertex.x, 
			ai_vertex.y, 
			ai_vertex.z 
		};

		// Get texture coords
		glm::vec2 uv = { 0, 0 };
		if (ai_mesh->HasTextureCoords(0)) {
			APE_TRACE("HAS TEXTURE COORDS");
			uv[0] = ai_mesh->mTextureCoords[0][i].x;
			uv[1] = ai_mesh->mTextureCoords[0][i].y;
		}
		else APE_TRACE("NO TEXTURE COORDS");

		// Get vertex normal
		glm::vec3 normal = { 0, 0, 0 };
		if (ai_mesh->HasNormals()) {
			normal.x = ai_mesh->mNormals[i].x;
			normal.y = ai_mesh->mNormals[i].y;
			normal.z = ai_mesh->mNormals[i].z;
		}

		// Add vertex
		vertices.push_back({
			.pos = pos,
			.normal = normal,
			.uv = uv,
		});
	}

	// Get indices for each face
	std::vector<IndexType> indices;
	for (size_t i = 0; i < ai_mesh->mNumFaces; ++i) {
		// Process face's indices
		aiFace face = ai_mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return MeshType(vertices, indices);
}

std::vector<Model::MeshType>& Model::getMeshes()
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

