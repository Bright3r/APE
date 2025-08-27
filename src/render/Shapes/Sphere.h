#pragma once

#include "render/Image.h"
#include "render/Model.h"

#include <memory>
#include <string_view>

namespace APE::Render {

class Sphere : public Model {
private:
	static constexpr std::string_view SPHERE_PATH =
		"res/models/textured-sphere.obj";

public:
	Sphere(float radius = 1.f) noexcept
		: Model(SPHERE_PATH)
	{
		m_transform.scale = { radius, radius, radius };
	}

	[[nodiscard]] float getRadius() const noexcept
	{
		return m_transform.scale.x;
	}

	void setRadius(float radius) noexcept
	{
		m_transform.scale = { radius, radius, radius };
	}

	[[nodiscard]] std::shared_ptr<Image> getTexture() const noexcept
	{
		return m_meshes[0].getTexture();
	}

	void setTexture(std::shared_ptr<Image> img) noexcept
	{
		m_meshes[0].changeTexture(img);
	}
};

};	// end of namespace

