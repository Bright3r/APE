#pragma once

#include "physics/collisions/Colliders.h"

#include <utility>

namespace APE::Physics::Collisions {

using MinMax = std::pair<float, float>;

MinMax projectTriangle(const glm::vec3& axis, const TriangleCollider& tri) noexcept;

MinMax projectAABB(const glm::vec3& axis, const AABB& box) noexcept;

bool overlapOnAxis(
	const AABB& box,
	const TriangleCollider& tri,
	const glm::vec3& axis) noexcept;

bool intersects(const AABB& a, const AABB& b) noexcept;

bool intersects(const AABB& box, const TriangleCollider& tri) noexcept;

float getIntersectDist(
	const AABB& box,
	const Ray& raycast) noexcept;

bool intersects(const AABB& box, const Ray& raycast) noexcept;

};	// end of namespace

