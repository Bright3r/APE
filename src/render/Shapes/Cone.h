#pragma once

#include "render/Image.h"
#include "render/Model.h"

#include <memory>
#include <string_view>

namespace APE {
namespace Render {

class Cone : public Model {
private:
	static constexpr std::string_view CONE_PATH =
		"res/models/textured-cone.obj";

public:
	Cone(float radius = 0.5f, float height = 1.f) noexcept
		: Model(CONE_PATH)
	{
		m_transform.scale = { 2*radius, height, 2*radius };
	}

	[[nodiscard]] float getRadius() const noexcept
	{
		return m_transform.scale.x;
	}

	void setRadius(float radius) noexcept
	{
		m_transform.scale.x = 2*radius;
		m_transform.scale.z = 2*radius;
	}

	[[nodiscard]] float getHeight() const noexcept
	{
		return m_transform.scale.y;
	}

	void setHeight(float height) noexcept
	{
		m_transform.scale.y = height;
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

};	// end of namespace Render
};	// end of namespace APE
