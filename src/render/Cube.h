#pragma once

#include "render/Image.h"
#include "render/Model.h"

#include <memory>
#include <string_view>

namespace APE {
namespace Render {

class Cube : public Model {
	static constexpr std::string_view CUBE_PATH = "res/models/textured-cube.obj";
public:
	Cube(float width = 1.f, float height = 1.f, float depth = 1.f)
		: Model(CUBE_PATH)
	{
		m_transform.scale = { width, height, depth };
	}

	float getWidth() const
	{
		return m_transform.scale.x;
	}

	void setWidth(float width)
	{
		m_transform.scale.x = width;
	}

	float getHeight() const
	{
		return m_transform.scale.y;
	}

	void setHeight(float height)
	{
		m_transform.scale.y = height;
	}

	float getDepth() const
	{
		return m_transform.scale.z;
	}

	void setDepth(float depth)
	{
		m_transform.scale.z = depth;
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
