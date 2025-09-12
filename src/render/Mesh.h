#pragma once

#include "components/Object.h"
#include "render/Image.h"

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
	std::shared_ptr<Image> m_texture;

public:
	Mesh() noexcept = default;

	Mesh(const std::vector<VertexType>& vertices,
		const std::vector<IndexType>& indices,
		const TransformComponent& transform,
		std::shared_ptr<Image> texture) noexcept
		: m_vertices(vertices)
		, m_indices(indices)
		, m_transform(transform)
		, m_texture(texture)
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

	[[nodiscard]] std::shared_ptr<Image> getTexture() const noexcept
	{
		return m_texture;
	}

	[[nodiscard]] TransformComponent& getTransform() noexcept
	{
		return m_transform;
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

	void changeTexture(std::shared_ptr<Image> texture) noexcept
	{
		m_texture = texture;
	}
};

};	// end of namespace

