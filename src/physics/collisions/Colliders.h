#pragma once

#include <glm/glm.hpp>

namespace APE::Physics::Collisions {

struct Collider {
	virtual ~Collider() noexcept = default;
};

struct Ray : public Collider {
	glm::vec3 pos;
	glm::vec3 dir;

	Ray(const glm::vec3& pos = glm::vec3(0), const glm::vec3& dir = glm::vec3(0)) noexcept
		: pos(pos)
		, dir(dir)
	{

	}

	[[nodiscard]] glm::vec3 eval(float t) const noexcept
	{
		return pos + (t * dir);
	}
};

struct AABB : public Collider {
	glm::vec3 min;
	glm::vec3 max;

	AABB(const glm::vec3& min = glm::vec3(0), const glm::vec3& max = glm::vec3(0)) noexcept
		: min(min)
		, max(max)
	{

	}

	[[nodiscard]] glm::vec3 center() const noexcept 
	{
		return (min + max) * 0.5f;
	}

	[[nodiscard]] glm::vec3 extents() const noexcept 
	{
		return (max - min) * 0.5f;
	}
};

struct TriangleCollider : public Collider  {
	glm::vec3 v0, v1, v2;

	TriangleCollider(
		const glm::vec3& v0 = glm::vec3(0),
		const glm::vec3& v1 = glm::vec3(0),
		const glm::vec3& v2 = glm::vec3(0)) noexcept
		: v0(v0)
		, v1(v1)
		, v2(v2)
	{

	}

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

