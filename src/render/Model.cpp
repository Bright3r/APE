#include "render/Model.h"
#include "util/Logger.h"
#include <SDL3/SDL_stdinc.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <cstddef>
#include <cstdlib>
#include <vector>

namespace APE {
namespace Render {

Model::Model(std::filesystem::path model_path, const Transform& transform) noexcept
	: m_meshes()
	, m_transform(transform)
{
	loadModel(model_path);
}

void Model::loadModel(std::filesystem::path model_path) noexcept
{
	m_meshes.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		model_path,
		aiProcess_Triangulate | 
		aiProcess_FlipUVs | 
		aiProcess_FixInfacingNormals
	);

	bool succ_load_model = !(!scene 
		|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE 
		|| !scene->mRootNode
	);
	if (!succ_load_model) {
		APE_ERROR(
			"Model::Model(const std::filesystem::path& model_path) Failed - {}",
			importer.GetErrorString()
		);
		return;
	}

	processNode(scene->mRootNode, scene, model_path);
}

Transform Model::convertAiTransform(const aiMatrix4x4 ai_transform) noexcept
{
	aiVector3f ai_pos;
	aiVector3f ai_scale;
	aiQuaternion ai_rot;
	ai_transform.Decompose(ai_scale, ai_rot, ai_pos);

	return Transform(
		glm::vec3(ai_pos.x, ai_pos.y, ai_pos.z),
		glm::vec3(ai_scale.x, ai_scale.y, ai_scale.z),
		glm::quat(ai_rot.w, ai_rot.x, ai_rot.y, ai_rot.z)
	);
}

std::shared_ptr<Image> Model::convertAiMaterial(
	const aiMaterial* ai_mat,
	const aiScene* scene,
	std::filesystem::path model_path) noexcept
{
	// Check for diffuse texture
	if (ai_mat->GetTextureCount(aiTextureType_DIFFUSE) <=  0) {
		// Return default texture on failure
		APE_ERROR(
			"Model::convertAiMaterial Failed - diffuse texture not found."
		);
		return std::make_shared<Image>();
	}

	// Get diffuse texture path
	aiString path;
	if (ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
		// Check if texture is embedded in model file
		if (path.length > 0 && path.data[0] == '*') {
			int tex_idx = std::atoi(path.C_Str() + 1);
			aiTexture* ai_tex = scene->mTextures[tex_idx];

			return std::make_shared<Image>(
				ai_tex->mWidth,
				ai_tex->mHeight,
				reinterpret_cast<std::byte*>(ai_tex->pcData)
			);
		}
		// Otherwise create texture from file
		else {
			std::string tex_path { 
				model_path.parent_path().append(path.C_Str())
			};
			return std::make_shared<Image>(tex_path);
		}
	}

	// Return default texture on failure
	APE_ERROR(
		"Model::convertAiMaterial Failed - could not load diffuse texture."
	);
	return std::make_shared<Image>();
}

void Model::processNode(
	const aiNode* node,
	const aiScene* scene,
	std::filesystem::path model_path) noexcept
{
	Transform local_transform = convertAiTransform(node->mTransformation);

	// Convert nodes aiMeshes into our own Meshes
	for (size_t i = 0; i < node->mNumMeshes; ++i) {
		// Get aiMesh
		unsigned int mesh_idx = node->mMeshes[i];
		aiMesh* ai_mesh = scene->mMeshes[mesh_idx];

		// Get texture for current mesh
		aiMaterial* ai_mat = scene->mMaterials[ai_mesh->mMaterialIndex];
		std::shared_ptr<Image> texture = 
			convertAiMaterial(ai_mat, scene, model_path);

		m_meshes.emplace_back(processAiMesh(
			ai_mesh,
			texture,
			local_transform
		));
	}

	// Process child nodes
	for (size_t i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene, model_path);
	}
}

Model::ModelMesh Model::processAiMesh(
	const aiMesh* ai_mesh,
	std::shared_ptr<Image> texture,
	const Transform& transform) const noexcept
{
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
			uv[0] = ai_mesh->mTextureCoords[0][i].x;
			uv[1] = ai_mesh->mTextureCoords[0][i].y;
		}

		// Get vertex normal
		glm::vec3 normal = { 0, 0, 0 };
		if (ai_mesh->HasNormals()) {
			normal.x = ai_mesh->mNormals[i].x;
			normal.y = ai_mesh->mNormals[i].y;
			normal.z = ai_mesh->mNormals[i].z;
		}

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

	return ModelMesh(vertices, indices, texture, transform);
}

std::vector<Model::ModelMesh>& Model::getMeshes() noexcept
{
	return m_meshes;
}

Transform& Model::getTransform() noexcept
{
	return m_transform;
}

void Model::setTransform(const Transform& transform) noexcept
{
	m_transform = transform;
}


};	// end of namespace Render
};	// end of namespace APE

