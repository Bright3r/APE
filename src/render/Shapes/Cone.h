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
	Cone(float radius = 1.f, float height = 1.f)
		: Model(CONE_PATH)
	{
		m_transform.scale = { radius, height, radius };
	}

	float getRadius() const
	{
		return m_transform.scale.x;
	}

	void setRadius(float radius)
	{
		m_transform.scale.x = radius;
		m_transform.scale.z = radius;
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
