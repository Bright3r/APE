#pragma once

#include "components/Object.h"
#include "scene/AssetManager.h"
#include "render/Image.h"
#include "render/SafeGPU.h"

#include <glm/glm.hpp>

#include <vector>

namespace APE::Render {

template <typename VertexType, typename IndexType>
struct Mesh {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;
	TransformComponent transform;
	AssetHandle<Image> texture_handle;
	SafeGPU::UniqueGPUBuffer vertex_buffer;
	SafeGPU::UniqueGPUBuffer index_buffer;

	Mesh() noexcept = default;

	Mesh(const std::vector<VertexType>& vertices,
		const std::vector<IndexType>& indices,
		const TransformComponent& transform,
		const AssetHandle<Image>& texture_handle) noexcept
		: vertices(vertices)
		, indices(indices)
		, transform(transform)
		, texture_handle(texture_handle)
		, vertex_buffer(nullptr)
		, index_buffer(nullptr)
	{

	}
};

};	// end of namespace

