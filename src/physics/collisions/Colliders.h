#pragma once

#include <glm/glm.hpp>

namespace APE::Physics::Collisions {

enum class ColliderType {
	AABB = 0,
	Size
};

struct Collider {
	ColliderType type;
	glm::vec3 pos;

	Collider(ColliderType type = ColliderType::Size) noexcept
		: type(type)
		, pos({})
	{

	}

	virtual ~Collider() noexcept = default;
};

struct AABB : public Collider {
	glm::vec3 min;
	glm::vec3 max;

	AABB(const glm::vec3& min = glm::vec3(0), const glm::vec3& max = glm::vec3(0)) noexcept
		: Collider(ColliderType::AABB) 
		, min(min)
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

struct Ray {
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

	float getIntersectDist(const AABB& box) noexcept
	{
		float t1 = (box.min.x - pos.x) / dir.x;
		float t2 = (box.max.x - pos.x) / dir.x;
		float t3 = (box.min.y - pos.y) / dir.y;
		float t4 = (box.max.y - pos.y) / dir.y;
		float t5 = (box.min.z - pos.z) / dir.z;
		float t6 = (box.max.z - pos.z) / dir.z;

		float tmin = glm::max(
			glm::max(glm::min(t1, t2), glm::min(t3, t4)), 
			glm::min(t5, t6)
		);
		float tmax = glm::min(
			glm::min(glm::max(t1, t2), glm::max(t3, t4)), 
			glm::max(t5, t6)
		);

		if (tmax < 0 || tmin > tmax) return -1;

		if (tmin < 0.0f) {
			return tmax;
		}
		return tmin;
	}

	bool intersects(const AABB& box, float& t) noexcept 
	{
		t = getIntersectDist(box);
		return t >= 0;
	}
};

struct Triangle {
	glm::vec3 v0, v1, v2;

	Triangle(
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

