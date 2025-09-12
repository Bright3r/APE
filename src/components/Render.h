#pragma once

#include "core/AssetManager.h"
#include "render/Model.h"
#include "render/Image.h"

#include <glm/glm.hpp>

namespace APE::Render {

struct MeshComponent {
	AssetHandle<Model> model_handle;
	size_t mesh_index;

	MeshComponent(AssetHandle<Model> model_handle, size_t mesh_index) noexcept
		: model_handle(model_handle)
		, mesh_index(mesh_index)
	{

	}
};

struct MaterialComponent {
	AssetHandle<Image> texture_handle;

	MaterialComponent(AssetHandle<Image> texture_handle) noexcept
		: texture_handle(texture_handle)
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

