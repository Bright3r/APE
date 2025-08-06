#pragma once

#include "render/SafeGPU.h"
#include "render/Vertex.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include <vector>

namespace APE {
namespace Render {

using VertexIndex = Uint16;

class Mesh {
	friend class Renderer;
private:
	// Mesh data
	std::vector<PositionColorVertex> m_vertices;
	std::vector<VertexIndex> m_indices;

	// Rendering data
	SafeGPU::UniqueGPUBuffer m_vertex_buffer;
	SafeGPU::UniqueGPUBuffer m_index_buffer;


	// Private methods
	SDL_GPUBuffer* getVertexBuffer() const;

	void setVertexBuffer(SafeGPU::UniqueGPUBuffer buf);

	SDL_GPUBuffer* getIndexBuffer() const;

	void setIndexBuffer(SafeGPU::UniqueGPUBuffer buf);

public:
	Mesh() = default;

	Mesh(const std::vector<PositionColorVertex>& vertices, 
      		const std::vector<VertexIndex>& indices);

	~Mesh() = default;

	// not copyable because of unique_ptr
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	// moveable
	Mesh(Mesh&&) noexcept = default;
	Mesh& operator=(Mesh&&) noexcept = default;


	std::vector<PositionColorVertex> getVertices() const;

	std::vector<VertexIndex> getIndices() const;

	void changeTopology(const std::vector<VertexIndex> indices);

	void changeMesh(const std::vector<PositionColorVertex>& vertices, 
      		const std::vector<VertexIndex>& indices);
};

};	// end of namespace Render
};	// end of namespace APE
