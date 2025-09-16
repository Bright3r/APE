#pragma once

#include <glm/glm.hpp>

namespace APE::Physics::Collider {

struct Ray {
	glm::vec3 pos;
	glm::vec3 dir;

	[[nodiscard]] glm::vec3 eval(float t) const noexcept
	{
		return pos + (t * dir);
	}
};

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

struct Triangle {
	glm::vec3 v0, v1, v2;

	[[nodiscard]] glm::vec3 getNormal() const noexcept 
	{
		return glm::normalize(glm::cross(v1 - v0, v2 - v0));
	}

	[[nodiscard]] glm::vec3 edge0() const noexcept
	{
		return v1 - v0;
	}

	[[nodiscard]] glm::vec3 edge1() const noexcept
	{
		return v2 - v1;
	}

	[[nodiscard]] glm::vec3 edge2() const noexcept
	{
		return v0 - v2;
	}
};

};	// end of namespace

