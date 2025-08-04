#include "render/Mesh.h"
#include <utility>

namespace APE {
namespace Render {

Mesh::Mesh(const std::vector<PositionColorVertex>& vertices, 
	const std::vector<VertexIndex>& indices,
	const Transform& transform)
	: m_vertices(vertices)
	, m_indices(indices)
	, m_transform(transform)
	, m_vertex_buffer(nullptr)
	, m_index_buffer(nullptr)
{

}

SDL_GPUBuffer* Mesh::getVertexBuffer() const
{
	return m_vertex_buffer.get();
}

void Mesh::setVertexBuffer(SafeGPU::UniqueGPUBuffer buf)
{
	m_vertex_buffer = std::move(buf);
}

SDL_GPUBuffer* Mesh::getIndexBuffer() const
{
	return m_index_buffer.get();
}

void Mesh::setIndexBuffer(SafeGPU::UniqueGPUBuffer buf)
{
	m_index_buffer = std::move(buf);
}

std::vector<PositionColorVertex> Mesh::getVertices() const
{
	return m_vertices;
}

std::vector<VertexIndex> Mesh::getIndices() const
{
	return m_indices;
}

void Mesh::changeTopology(const std::vector<VertexIndex> indices)
{
	m_indices = indices;
	m_index_buffer = nullptr;
}

void Mesh::changeMesh(const std::vector<PositionColorVertex>& vertices, 
	const std::vector<VertexIndex>& indices)
{
	m_vertices = vertices;
	m_indices = indices;

	m_vertex_buffer = nullptr;
	m_index_buffer = nullptr;
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
