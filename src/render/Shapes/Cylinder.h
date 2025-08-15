#pragma once

#include "render/Image.h"
#include "render/Model.h"

#include <memory>
#include <string_view>

namespace APE {
namespace Render {

class Cylinder : public Model {
	static constexpr std::string_view CYLINDER_PATH =
		"res/models/textured-cylinder.obj";
public:
	Cylinder(float radius = 0.5f, float height = 1.f)
		: Model(CYLINDER_PATH)
	{
		m_transform.scale = { 2*radius, height, 2*radius };
	}

	float getRadius() const
	{
		return m_transform.scale.x;
	}

	void setRadius(float radius)
	{
		m_transform.scale.x = 2*radius;
		m_transform.scale.z = 2*radius;
	}

	float getHeight() const
	{
		return m_transform.scale.y;
	}

	void setHeight(float height)
	{
		m_transform.scale.y = height;
	}

	std::shared_ptr<Image> getTexture() const
	{
		return m_meshes[0].getTexture();
	}

	void setTexture(std::shared_ptr<Image> img)
	{
		m_meshes[0].changeTexture(img);
	}
};

};	// end of namespace Render
};	// end of namespace APE
