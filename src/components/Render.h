#pragma once

#include "render/Vertex.h"
#include "render/Image.h"
#include "render/SafeGPU.h"

#include <vector>

namespace APE::Render {

using VertexType = TextureVertex;
using IndexType = Uint32;

struct MeshComponent {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;

	SafeGPU::UniqueGPUBuffer vertex_buffer;
	SafeGPU::UniqueGPUBuffer index_buffer;

	MeshComponent(std::vector<VertexType> vertices,
		std::vector<IndexType> indices) noexcept
		: vertices(vertices)
		, indices(indices)
		, vertex_buffer(nullptr)
		, index_buffer(nullptr)
	{

	}
};

struct MaterialComponent {
	std::shared_ptr<Image> texture;
	SafeGPU::UniqueGPUTexture texture_buffer;

	MaterialComponent(std::shared_ptr<Image> texture) noexcept
		: texture(texture)
		, texture_buffer(nullptr)
	{

	}
};

};

