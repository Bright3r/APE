#pragma once

#include "physics/Colliders.h"

#include <utility>

namespace APE::Physics::Collisions {

using MinMax = std::pair<float, float>;

MinMax projectTriangle(const glm::vec3& axis, const Collider::Triangle tri) noexcept;

MinMax projectAABB(const glm::vec3& axis, const Collider::AABB& box) noexcept;

bool overlapOnAxis(
	const Collider::AABB& box,
	const Collider::Triangle& tri,
	const glm::vec3& axis) noexcept;

bool intersects(const Collider::AABB& a, const Collider::AABB& b) noexcept;

bool intersects(const Collider::AABB& box, const Collider::Triangle& tri) noexcept;

float getIntersectDist(
	const Collider::AABB& box,
	const Collider::Ray& raycast) noexcept;

bool intersects(const Collider::AABB& box, const Collider::Ray& raycast) noexcept;

};	// end of namespace

