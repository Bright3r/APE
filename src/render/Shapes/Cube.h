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
	Cube(float width = 1.f, float height = 1.f, float depth = 1.f) noexcept
		: Model(CUBE_PATH)
	{
		m_transform.scale = { width, height, depth };
	}

	[[nodiscard]] float getWidth() const noexcept
	{
		return m_transform.scale.x;
	}

	void setWidth(float width) noexcept
	{
		m_transform.scale.x = width;
	}

	[[nodiscard]] float getHeight() const noexcept
	{
		return m_transform.scale.y;
	}

	void setHeight(float height) noexcept
	{
		m_transform.scale.y = height;
	}

	[[nodiscard]] float getDepth() const noexcept
	{
		return m_transform.scale.z;
	}

	void setDepth(float depth)
	{
		m_transform.scale.z = depth;
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
