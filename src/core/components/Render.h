#pragma once

#include "core/render/Camera.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Model.h"
#include "core/render/Image.h"
#include "core/scene/ImageLoader.h"

#include <glm/glm.hpp>

namespace APE::Render 
{

struct MeshComponent 
{
	static constexpr const char* Name = "Mesh";
	AssetHandle<Model> model_handle;
	size_t mesh_index;

	MeshComponent(
		AssetHandle<Model> model_handle = {}, 
		size_t mesh_index = 0
	) noexcept
		: model_handle(model_handle)
		, mesh_index(mesh_index)
	{

	}
};

struct MaterialComponent 
{
	static constexpr const char* Name = "Material";
	AssetHandle<Image> texture_handle;

	MaterialComponent(
		AssetHandle<Image> texture_handle = ImageLoader::defaultImage()) noexcept
		: texture_handle(texture_handle)
	{

	}
};

struct CameraComponent
{
	static constexpr const char* Name = "Camera";
	std::shared_ptr<Camera> camera;

	CameraComponent(std::shared_ptr<Camera> camera = nullptr) noexcept
		: camera(camera)
	{

	}
};

enum LightType 
{
	Point = 0,
	Direction,
	Spot,
	Area,
	Size
};

struct LightComponent 
{
	static constexpr const char* Name = "Light";
	LightType type;
	glm::vec4 ambient_color;
	glm::vec4 diffuse_color;
	glm::vec4 specular_color;
	glm::vec4 attenuation;

	LightComponent(
		const LightType& type = LightType::Direction,
		const glm::vec4& ambient_color = glm::vec4(1.f),
		const glm::vec4& diffuse_color = glm::vec4(1.f),
		const glm::vec4& specular_color = glm::vec4(1.f),
		const glm::vec4& attenuation = glm::vec4(1.f, 0.09f, 0.032f, 0.f)
	) noexcept
		: type(type)
		, ambient_color(ambient_color)
		, diffuse_color(diffuse_color)
		, specular_color(specular_color)
		, attenuation(attenuation)
	{

	}
};

struct RenderLight
{
	glm::vec4 position;
	glm::vec4 attenuation;
	glm::vec4 ambient_color;
	glm::vec4 diffuse_color;
	glm::vec4 specular_color;
	LightType type;
	glm::vec3 dir;

	RenderLight(
		const glm::vec4& position = glm::vec4(0),
		const glm::vec4& attenuation = glm::vec4(1.f, 0.09f, 0.032f, 0.f),
		const glm::vec4& ambient_color = glm::vec4(1.f),
		const glm::vec4& diffuse_color = glm::vec4(1.f),
		const glm::vec4& specular_color = glm::vec4(1.f),
		const LightType& type = LightType::Direction,
		const glm::vec3& dir = glm::vec3(0, -1, 0)
	) noexcept
		: position(position)
		, attenuation(attenuation)
		, ambient_color(ambient_color)
		, diffuse_color(diffuse_color)
		, specular_color(specular_color)
		, type(type)
		, dir(dir)
	{

	}
};

};

