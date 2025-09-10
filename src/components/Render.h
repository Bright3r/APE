#pragma once

#include "render/Vertex.h"
#include "render/Image.h"
#include "render/SafeGPU.h"

#include <glm/glm.hpp>

#include <vector>

namespace APE::Render {

using VertexType = TextureVertex;
using IndexType = Uint32;

struct MeshComponent {
	std::vector<VertexType> vertices;
	std::vector<IndexType> indices;

	SafeGPU::UniqueGPUBuffer vertex_buffer;
	SafeGPU::UniqueGPUBuffer index_buffer;

	MeshComponent(std::vector<VertexType> vertices,
		std::vector<IndexType> indices) noexcept
		: vertices(vertices)
		, indices(indices)
		, vertex_buffer(nullptr)
		, index_buffer(nullptr)
	{

	}
};

struct MaterialComponent {
	std::shared_ptr<Image> texture;
	SafeGPU::UniqueGPUTexture texture_buffer;

	MaterialComponent(std::shared_ptr<Image> texture) noexcept
		: texture(texture)
		, texture_buffer(nullptr)
	{

	}
};

enum LightType {
	Point = 0,
	Direction,
	Spot,
	Area,
	Size
};

enum class AreaLightShape {
	Rectangle,
	Disk
};

struct LightComponent {
	LightType type;
	float intensity;
	glm::vec3 color;

	// Spotlight only
	float cutoff_angle;

	// Area light only
	AreaLightShape shape;
	glm::vec2 extent;

	// Point/Spot/Directional light
	LightComponent(LightType type = LightType::Point,
		float intensity = 10.f,
		glm::vec3 color = glm::vec3(1.f),
		float cutoff_angle = 45.f) noexcept
		: type(type)
		, intensity(intensity)
		, color(color)
		, cutoff_angle(cutoff_angle)
		, shape(AreaLightShape::Rectangle)
		, extent(glm::vec2(5.f))
	{

	}

	// Area light
	LightComponent(LightType type = LightType::Point,
		float intensity = 10.f,
		glm::vec3 color = glm::vec3(1.f),
		AreaLightShape shape = AreaLightShape::Rectangle,
		glm::vec2 extent = glm::vec2(5.f)) noexcept
		: type(type)
		, intensity(intensity)
		, color(color)
		, shape(shape)
		, extent(extent)
		, cutoff_angle(45.f)
	{

	}
};

};

