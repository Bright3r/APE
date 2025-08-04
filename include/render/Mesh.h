#pragma once

#include "render/SafeGPU.h"
#include "render/Transform.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>

namespace APE {
namespace Render {

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

using VertexIndex = Uint16;

class Mesh {
	friend class Renderer;
private:
	// Mesh data
	std::vector<PositionColorVertex> m_vertices;
	std::vector<VertexIndex> m_indices;
	Transform m_transform;

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
      		const std::vector<VertexIndex>& indices,
      		const Transform& transform = {});

	std::vector<PositionColorVertex> getVertices() const;

	std::vector<VertexIndex> getIndices() const;

	void changeTopology(const std::vector<VertexIndex> indices);

	void changeMesh(const std::vector<PositionColorVertex>& vertices, 
      		const std::vector<VertexIndex>& indices);

	Transform& getTransform();

	void setTransform(const Transform& transform);
};

};	// end of namespace Render
};	// end of namespace APE
