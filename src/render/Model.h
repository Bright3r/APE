#pragma once

#include "components/Object.h"
#include "render/Mesh.h"
#include "render/Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <filesystem>
#include <vector>

namespace APE::Render {

struct Model {
	// Type aliases
	using VertexType = TextureVertex;
	using IndexType = Uint32;
	using ModelMesh = Mesh<VertexType, IndexType>;

	// Data members
	std::vector<ModelMesh> meshes;
	std::filesystem::path model_path;
	TransformComponent transform;

	Model() noexcept = default;

	Model(std::filesystem::path model_path) noexcept;

	virtual ~Model() noexcept { }
};

};	// end of namespace

