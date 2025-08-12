#pragma once

#include "render/SafeGPU.h"
#include "render/Image.h"
#include "render/Transform.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include <vector>
#include <memory>

namespace APE {
namespace Render {

template <typename VertexType, typename IndexType>
class Mesh {
	friend class Renderer;
private:
	// Mesh data
	std::vector<VertexType> m_vertices;
	std::vector<IndexType> m_indices;
	std::shared_ptr<Image> m_texture;
	Transform m_transform;

	// Rendering data
	SafeGPU::UniqueGPUBuffer m_vertex_buffer;
	SafeGPU::UniqueGPUBuffer m_index_buffer;
	SafeGPU::UniqueGPUTexture m_texture_buffer;


	// Private methods
	SDL_GPUBuffer* getVertexBuffer() const
	{
		return m_vertex_buffer.get();
	}

	void setVertexBuffer(SafeGPU::UniqueGPUBuffer buf)
	{
		m_vertex_buffer = std::move(buf);
	}

	SDL_GPUBuffer* getIndexBuffer() const
	{
		return m_index_buffer.get();
	}

	void setIndexBuffer(SafeGPU::UniqueGPUBuffer buf)
	{
		m_index_buffer = std::move(buf);
	}

	SDL_GPUTexture* getTextureBuffer() const
	{
		return m_texture_buffer.get();
	}

	void setTextureBuffer(SafeGPU::UniqueGPUTexture tex)
	{
		m_texture_buffer = std::move(tex);
	}

public:
	Mesh() = default;

	Mesh(const std::vector<VertexType>& vertices, 
		const std::vector<IndexType>& indices,
      		const std::shared_ptr<Image>& texture,
      		const Transform& transform)
		: m_vertices(vertices)
		, m_indices(indices)
		, m_texture(texture)
		, m_transform(transform)
		, m_vertex_buffer(nullptr)
		, m_index_buffer(nullptr)
		, m_texture_buffer(nullptr)
	{

	}

	~Mesh() = default;

	// not copyable because of unique_ptr
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// moveable
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;


	std::vector<VertexType> getVertices() const
	{
		return m_vertices;
	}

	std::vector<IndexType> getIndices() const
	{
		return m_indices;
	}

	std::shared_ptr<Image> getTexture() const
	{
		return m_texture;
	}

	Transform& getTransform()
	{
		return m_transform;
	}

	void changeTopology(const std::vector<IndexType> indices)
	{
		m_indices = indices;
		m_index_buffer = nullptr;
	}

	void changeMesh(const std::vector<VertexType>& vertices, 
      		const std::vector<IndexType>& indices)
	{
		m_vertices = vertices;
		m_indices = indices;

		m_vertex_buffer = nullptr;
		m_index_buffer = nullptr;
	}

	void changeTexture(std::shared_ptr<Image> texture)
	{
		m_texture = texture;

		m_texture_buffer = nullptr;
	}
};

};	// end of namespace Render
};	// end of namespace APE

