#pragma once

#include "render/Image.h"
#include "render/Model.h"

#include <memory>
#include <string_view>

namespace APE {
namespace Render {

class Sphere : public Model {
private:
	static constexpr std::string_view SPHERE_PATH =
		"res/models/textured-sphere.obj";

public:
	Sphere(float radius = 1.f)
		: Model(SPHERE_PATH)
	{
		m_transform.scale = { radius, radius, radius };
	}

	float getRadius() const
	{
		return m_transform.scale.x;
	}

	void setRadius(float radius)
	{
		m_transform.scale = { radius, radius, radius };
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
