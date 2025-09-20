#include "physics/collisions/Collisions.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <array>

namespace APE::Physics::Collisions {

MinMax projectTriangle(const glm::vec3& axis, const Collider::Triangle tri) noexcept
{
	float min = glm::dot(tri.v0, axis);
	float max = min;

	float d1 = glm::dot(tri.v1, axis);
	float d2 = glm::dot(tri.v2, axis);

	min = std::min(min, std::min(d1, d2));
	max = std::max(max, std::max(d1, d2));
	return { min, max };
}

MinMax projectAABB(const glm::vec3& axis, const Collider::AABB& box) noexcept
{
	std::array<glm::vec3, 8> corners = {
		glm::vec3 { box.min.x, box.min.y, box.min.z },
		glm::vec3 { box.max.x, box.min.y, box.min.z },
		glm::vec3 { box.min.x, box.max.y, box.min.z },
		glm::vec3 { box.min.x, box.min.y, box.max.z },
		glm::vec3 { box.max.x, box.max.y, box.min.z },
		glm::vec3 { box.max.x, box.min.y, box.max.z },
		glm::vec3 { box.min.x, box.max.y, box.max.z },
		glm::vec3 { box.max.x, box.max.y, box.max.z }
	};

	float min = glm::dot(corners[0], axis);
	float max = min;
	for (int i = 1; i < 8; ++i) {
		float val = glm::dot(corners[i], axis);
		min = std::min(min, val);
		max = std::max(max, val);
	}
	return { min, max };
}	

bool overlapOnAxis(
	const Collider::AABB& box,
	const Collider::Triangle& tri,
	const glm::vec3& axis) noexcept
{
	// Skip axes near zero
	constexpr float epsilon = 1e-6f;
	if (glm::length2(axis) < epsilon) return true;

	glm::vec3 normAxis = glm::normalize(axis);
	auto [minAABB, maxAABB] = projectAABB(normAxis, box);
	auto [minTri, maxTri] = projectTriangle(normAxis, tri);
	return !(maxAABB < minTri || maxTri < minAABB);
}

bool intersects(const Collider::AABB& a, const Collider::AABB& b) noexcept
{
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}

bool intersects(const Collider::AABB& box, const Collider::Triangle& tri) noexcept 
{
	// 13 axes to test for overlap
	glm::vec3 axes[13];

	// Normals of the AABB
	axes[0] = glm::vec3(1, 0, 0);
	axes[1] = glm::vec3(0, 1, 0);
	axes[2] = glm::vec3(0, 0, 1);

	// Triangle normal
	axes[3] = tri.getNormal();

	constexpr std::array<glm::vec3, 3> world_axes = {
		glm::vec3(1,0,0), 
		glm::vec3(0,1,0), 
		glm::vec3(0,0,1) 
	};

	glm::vec3 e0 = tri.edge0();
	glm::vec3 e1 = tri.edge1();
	glm::vec3 e2 = tri.edge2();
	int i = 4;
	// Cross products between triangle edges and AABB axes
	for (auto& axis : world_axes) {
		axes[i++] = glm::cross(e0, axis);
		axes[i++] = glm::cross(e1, axis);
		axes[i++] = glm::cross(e2, axis);
	}

	// Check for a separating axis
	for (int j = 0; j < 13; ++j) {
		// Found a separating axis
		if (!overlapOnAxis(box, tri, axes[j])) {
			return false;
		}
	}
	return true;
};	

float getIntersectDist(
	const Collider::AABB& box,
	const Collider::Ray& ray) noexcept
{
	float t1 = (box.min.x - ray.pos.x) / ray.dir.x;
	float t2 = (box.max.x - ray.pos.x) / ray.dir.x;
	float t3 = (box.min.y - ray.pos.y) / ray.dir.y;
	float t4 = (box.max.y - ray.pos.y) / ray.dir.y;
	float t5 = (box.min.z - ray.pos.z) / ray.dir.z;
	float t6 = (box.max.z - ray.pos.z) / ray.dir.z;

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

bool intersects(const Collider::AABB& box, const Collider::Ray& raycast) noexcept 
{
	float t = getIntersectDist(box, raycast);
	return t >= 0;
}

};	// end of namespace

