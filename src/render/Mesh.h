#pragma once

#include "components/Object.h"
#include "scene/AssetManager.h"
#include "render/Image.h"
#include "render/SafeGPU.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include <vector>
#include <memory>

namespace APE::Render {

template <typename VertexType, typename IndexType>
class Mesh {
private:
	// Mesh data
	std::vector<VertexType> m_vertices;
	std::vector<IndexType> m_indices;
	TransformComponent m_transform;
	AssetHandle<Image> m_texture_handle;
	SafeGPU::UniqueGPUBuffer m_vertex_buffer;
	SafeGPU::UniqueGPUBuffer m_index_buffer;

public:
	Mesh() noexcept = default;

	Mesh(const std::vector<VertexType>& vertices,
		const std::vector<IndexType>& indices,
		const TransformComponent& transform,
		const AssetHandle<Image>& texture_handle) noexcept
		: m_vertices(vertices)
		, m_indices(indices)
		, m_transform(transform)
		, m_texture_handle(texture_handle)
		, m_vertex_buffer(nullptr)
		, m_index_buffer(nullptr)
	{

	}

	[[nodiscard]] std::vector<VertexType> getVertices() const noexcept
	{
		return m_vertices;
	}

	[[nodiscard]] std::vector<IndexType> getIndices() const noexcept
	{
		return m_indices;
	}

	[[nodiscard]] AssetHandle<Image> getTextureHandle() const noexcept
	{
		return m_texture_handle;
	}

	void setTextureHandle(const AssetHandle<Image>& texture_handle) noexcept
	{
		m_texture_handle = texture_handle;
	}

	[[nodiscard]] TransformComponent& transform() noexcept
	{
		return m_transform;
	}

	[[nodiscard]] SafeGPU::UniqueGPUBuffer& vertexBuffer() noexcept
	{
		return m_vertex_buffer;
	}

	[[nodiscard]] SafeGPU::UniqueGPUBuffer& indexBuffer() noexcept
	{
		return m_index_buffer;
	}

	void changeTopology(const std::vector<IndexType> indices) noexcept
	{
		m_indices = indices;
	}

	void changeMesh(const std::vector<VertexType>& vertices, 
      		const std::vector<IndexType>& indices) noexcept
	{
		m_vertices = vertices;
		m_indices = indices;
	}
};

};	// end of namespace

