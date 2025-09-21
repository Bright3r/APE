#pragma once

#include "physics/collisions/Colliders.h"

#include <functional>
#include <utility>

namespace APE::Physics::Collisions {

/*
* Function Table for dispatching collision queries
*/
constexpr size_t NUM_COLLIDERS = static_cast<size_t>(ColliderType::Size);
using IntersectFn = std::function<bool(const Collider& a, const Collider& b)>;
using IntersectFnTable = std::array<std::array<IntersectFn, NUM_COLLIDERS>, NUM_COLLIDERS>;

bool AABBvsAABB(const Collider& a, const Collider& b) noexcept;

static inline IntersectFnTable s_intersect_fn_table = {
	{ &AABBvsAABB }
};

[[nodiscard]] bool intersects(const Collider& a, const Collider& b) noexcept;


/*
* Helper/special case intersect functions
*/
using MinMax = std::pair<float, float>;
MinMax projectTriangle(const glm::vec3& axis, const Triangle& tri) noexcept;
MinMax projectAABB(const glm::vec3& axis, const AABB& box) noexcept;

bool overlapOnAxis(const AABB& box, const Triangle& tri, const glm::vec3& axis) noexcept;

bool intersects(const AABB& box, const Triangle& tri) noexcept;

float getIntersectDist(const AABB& box, const Ray& raycast) noexcept;

bool intersects(const AABB& box, const Ray& raycast) noexcept;

};	// end of namespace

