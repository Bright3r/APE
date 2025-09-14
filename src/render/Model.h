#pragma once

#include "components/Object.h"
#include "render/Mesh.h"
#include "render/Vertex.h"

#include <filesystem>
#include <vector>
#include <string_view>

namespace APE::Render {

struct Model {
	static inline std::string_view DEFAULT_MODEL_PATH = "res/models/cube.obj";

	// Type aliases
	using VertexType = TextureVertex;
	using IndexType = Uint32;
	using ModelMesh = Mesh<VertexType, IndexType>;

	// Data members
	std::vector<ModelMesh> meshes;
	std::filesystem::path model_path;
	TransformComponent transform;

	Model() noexcept = default;

	Model(std::filesystem::path model_path) noexcept
		: model_path(model_path)
	{

	}
};

};	// end of namespace

