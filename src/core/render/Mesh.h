#pragma once

#include "core/components/Object.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Image.h"
#include "core/render/SafeGPU.h"

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

	using Triangle = std::tuple<glm::vec3, glm::vec3, glm::vec3>;
	[[nodiscard]] std::vector<Triangle> triangles() const noexcept
	{
		std::vector<Triangle> tris;
		for (size_t i = 0; i < indices.size(); i += 3) {
			tris.push_back({ 
       				vertices[indices[i]].pos,
				vertices[indices[i+1]].pos,
				vertices[indices[i+2]].pos
			});
		}
		return tris;
	}
};

};	// end of namespace

