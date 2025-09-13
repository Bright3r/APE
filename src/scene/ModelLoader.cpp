#include "ModelLoader.h"
#include "AssetManager.h"

#include <assimp/postprocess.h>

#include <utility>

namespace APE {

AssetHandle<Render::Model> ModelLoader::load(
	std::filesystem::path model_path) noexcept
{
	AssetKey key { model_path };
	if (AssetManager::contains(key)) {
		return AssetManager::get<Render::Model>(key);
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(
		model_path,
		aiProcess_Triangulate | 
		aiProcess_FlipUVs | 
		aiProcess_FixInfacingNormals
	);

	bool succ_load_model = !(
		!scene 
		|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE 
		|| !scene->mRootNode
	);
	APE_CHECK(succ_load_model,
		"ModelLoader::load(const std::filesystem::path& model_path) Failed - {}",
		importer.GetErrorString()
	);

	auto m = std::make_unique<Render::Model>(model_path);
	processNode(scene->mRootNode, scene, *m, model_path);

	auto handle = AssetManager::upload<Render::Model>(
		key,
		AssetClass::Model,
		std::move(m)
	);
	return handle;
}

TransformComponent ModelLoader::convertAiTransform(
	const aiMatrix4x4 ai_transform) noexcept
{
	aiVector3f ai_pos;
	aiVector3f ai_scale;
	aiQuaternion ai_rot;
	ai_transform.Decompose(ai_scale, ai_rot, ai_pos);

	return TransformComponent(
		glm::vec3(ai_pos.x, ai_pos.y, ai_pos.z),
		glm::vec3(ai_scale.x, ai_scale.y, ai_scale.z),
		glm::quat(ai_rot.w, ai_rot.x, ai_rot.y, ai_rot.z)
	);
}

AssetHandle<Render::Image> ModelLoader::defaultImageHandle() noexcept
{
	AssetKey key { Render::Image::DEFAULT_IMG_PATH };
	if (!AssetManager::contains(key)) {
		AssetManager::upload<Render::Image>(
			key,
			AssetClass::Texture,
			std::make_unique<Render::Image>()
		);
	}
	return AssetManager::get<Render::Image>(key);
}

AssetHandle<Render::Image> ModelLoader::makeImageHandle(
	std::unique_ptr<Render::Image> texture,
	const std::string& handle_index) noexcept
{
	AssetKey key = { texture->getPath() };
	if (!AssetManager::contains(key)) {
		AssetManager::upload<Render::Image>(
			key,
			AssetClass::Texture,
			std::move(texture)
		);
	}
	return AssetManager::get<Render::Image>(key);
}

AssetHandle<Render::Image> ModelLoader::convertAiMaterial(
	const aiMaterial* ai_mat,
	const aiScene* scene,
	std::filesystem::path model_path) noexcept
{
	// Check for diffuse texture
	if (ai_mat->GetTextureCount(aiTextureType_DIFFUSE) <=  0) {
		// Return default texture on failure
		APE_ERROR(
			"ModelLoader::convertAiMaterial() Failed: Diffuse texture not found."
		);
		return defaultImageHandle();
	}

	// Get diffuse texture path
	aiString path;
	if (ai_mat->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
		// Check if texture is embedded in model file
		if (path.length > 0 && path.data[0] == '*') {
			int tex_idx = std::atoi(path.C_Str() + 1);
			aiTexture* ai_tex = scene->mTextures[tex_idx];

			return makeImageHandle(std::make_unique<Render::Image>(
				model_path,
				ai_tex->mWidth,
				ai_tex->mHeight,
				reinterpret_cast<std::byte*>(ai_tex->pcData)
			), ai_mat->GetName().C_Str());
		}
		// Otherwise create texture from file
		else {
			std::string tex_path { 
				model_path.parent_path().append(path.C_Str())
			};
			return makeImageHandle(std::make_unique<Render::Image>(tex_path), "");
		}
	}

	// Return default texture on failure
	APE_ERROR(
		"ModelLoader::convertAiMaterial() Failed: Could not load diffuse texture."
	);
	return defaultImageHandle();
}

void ModelLoader::processNode(
	const aiNode* node,
	const aiScene* scene,
	Render::Model& model,
	std::filesystem::path model_path) noexcept
{
	TransformComponent local_transform = convertAiTransform(node->mTransformation);

	// Convert nodes aiMeshes into our own Meshes
	for (size_t i = 0; i < node->mNumMeshes; ++i) {
		// Get aiMesh
		unsigned int mesh_idx = node->mMeshes[i];
		aiMesh* ai_mesh = scene->mMeshes[mesh_idx];

		// Get texture for current mesh
		aiMaterial* ai_mat = scene->mMaterials[ai_mesh->mMaterialIndex];
		AssetHandle<Render::Image> texture_handle = 
			convertAiMaterial(ai_mat, scene, model_path);

		model.meshes.emplace_back(processAiMesh(
			ai_mesh,
			texture_handle,
			local_transform
		));
	}

	// Process child nodes
	for (size_t i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], scene, model, model_path);
	}
}

Render::Model::ModelMesh ModelLoader::processAiMesh(
	const aiMesh* ai_mesh,
	const AssetHandle<Render::Image>& texture_handle,
	const TransformComponent& transform) noexcept
{
	std::vector<Render::Model::VertexType> vertices;
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
	std::vector<Render::Model::IndexType> indices;
	for (size_t i = 0; i < ai_mesh->mNumFaces; ++i) {
		// Process face's indices
		aiFace face = ai_mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; ++j) {
			indices.push_back(face.mIndices[j]);
		}
	}

	return Render::Model::ModelMesh(
		vertices,
		indices,
		transform,
		texture_handle
	);
}

};	// end of namespace

