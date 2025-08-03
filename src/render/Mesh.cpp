#include "render/Mesh.h"

namespace APE {
namespace Render {

Mesh::Mesh() = default;

Mesh::Mesh(const std::vector<PositionColorVertex>& vertices, 
	const glm::mat4& model_matrix)
	: m_vertices(vertices)
	, m_model_matrix(model_matrix)
	, m_vertex_buffer(nullptr)
{

}

SafeGPU::SharedGPUBuffer Mesh::getVertexBuffer() const
{
	return m_vertex_buffer;
}

void Mesh::setVertexBuffer(SafeGPU::SharedGPUBuffer buf)
{
	m_vertex_buffer = buf;
}

std::vector<PositionColorVertex> Mesh::getVertices() const 
{ 
	return m_vertices; 
}

void Mesh::setVertices(const std::vector<PositionColorVertex>& vertices)
{ 
	m_vertices = vertices;

	// invalidate gpu buffer
	m_vertex_buffer.reset();
}

Transform& Mesh::getTransform()
{
	return m_transform;
}

void Mesh::setTransform(const Transform& transform)
{
	m_transform = transform;
}

};	// end of namespace Render
};	// end of namespace APE
