#pragma once

#include "core/scene/AssetHandle.h"
#include "core/render/Model.h"
#include "core/render/Image.h"
#include "core/scene/ImageLoader.h"

#include <glm/glm.hpp>

namespace APE::Render {

struct MeshComponent {
	static constexpr const char* Name = "Mesh";
	AssetHandle<Model> model_handle;
	size_t mesh_index;

	MeshComponent(
		AssetHandle<Model> model_handle = {}, 
		size_t mesh_index = 0) noexcept
		: model_handle(model_handle)
		, mesh_index(mesh_index)
	{

	}
};

struct MaterialComponent {
	static constexpr const char* Name = "Material";
	AssetHandle<Image> texture_handle;

	MaterialComponent(
		AssetHandle<Image> texture_handle = ImageLoader::defaultImage()) noexcept
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
	static constexpr const char* Name = "Light";
	LightType type;
	float intensity;
	glm::vec3 color;

	// Spotlight only
	float cutoff_angle;

	// Area light only
	AreaLightShape shape;
	glm::vec2 extent;

	LightComponent(LightType type = LightType::Direction,
		float intensity = 10.f,
		glm::vec3 color = glm::vec3(1.f),
		float cutoff_angle = 45.f,
		AreaLightShape shape = AreaLightShape::Rectangle,
		glm::vec2 extent = glm::vec2(5.f)) noexcept
		: type(type)
		, intensity(intensity)
		, color(color)
		, cutoff_angle(cutoff_angle)
		, shape(shape)
		, extent(extent)
	{

	}
};

};

