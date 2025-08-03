#pragma once

#include "render/SafeGPU.h"
#include "render/Transform.h"

#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>

namespace APE {
namespace Render {

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

class Mesh {
	friend class Renderer;
private:
	// Mesh data
	std::vector<PositionColorVertex> m_vertices;
	glm::mat4 m_model_matrix;
	Transform m_transform;

	// Rendering data
	SafeGPU::SharedGPUBuffer m_vertex_buffer;


	// Private methods
	SafeGPU::SharedGPUBuffer getVertexBuffer() const;

	void setVertexBuffer(SafeGPU::SharedGPUBuffer buf);

public:
	Mesh();
	Mesh(const std::vector<PositionColorVertex>& vertices, 
      		const glm::mat4& model_matrix);

	std::vector<PositionColorVertex> getVertices() const;
	void setVertices(const std::vector<PositionColorVertex>& vertices);

	glm::mat4 getModelMatrix() const;
	void setModelMatrix(glm::mat4 model_mat);

	Transform getTransform() const;
	void setTransform(const Transform& transform);
};

};	// end of namespace Render
};	// end of namespace APE
