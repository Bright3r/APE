#pragma once

#include "render/Vertex.h"
#include "render/SafeGPU.h"
#include "render/Image.h"

#include <vector>

namespace APE::Render {

// Type aliases
using VertexType = TextureVertex;
using IndexType = Uint32;

struct MeshComponent {
	std::vector<VertexType> m_vertices;
	std::vector<IndexType> m_indices;

	SafeGPU::UniqueGPUBuffer m_vertex_buffer;
	SafeGPU::UniqueGPUBuffer m_index_buffer;
};

struct MaterialComponent {
	std::shared_ptr<Image> m_texture;
	SafeGPU::UniqueGPUTexture m_texture_buffer;
};

};

