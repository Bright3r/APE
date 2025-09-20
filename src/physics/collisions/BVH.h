#pragma once

#include "physics/collisions/Colliders.h"
#include "physics/collisions/Collisions.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/extended_min_max.hpp>

#include <vector>

namespace APE::Physics::Collider {

struct BVHNode {
	std::vector<BVHNode> children;
	std::vector<Triangle> triangles;
	AABB aabb;

	BVHNode(const AABB& aabb = AABB(glm::vec3(0), glm::vec3(0))) noexcept
		: aabb(aabb)
	{

	}

	BVHNode(const std::vector<Triangle>& tris, int max_depth = 3) noexcept 
	{
		// Get AABB of triangles
		glm::vec3 min_bounds(std::numeric_limits<float>::max());
		glm::vec3 max_bounds(-std::numeric_limits<float>::max());
		for (auto& tri : tris) {
			min_bounds = glm::min(min_bounds, tri.v0, tri.v1, tri.v2);
			max_bounds = glm::max(max_bounds, tri.v0, tri.v1, tri.v2);
		}
		aabb = AABB(min_bounds, max_bounds);

		// Set triangles
		triangles = tris;

		// Subdivide bvh tree
		split(max_depth);

		// Shake bvh tree
		shake();
	}

	// Determine if this BVH node is a leaf
	//
	bool isLeaf() const noexcept 
	{
		return children.empty() && !triangles.empty();
	}

	// Count all triangles in this node and its children
	//
	int getTriangleCount() const noexcept 
	{
		// Leaf node only has its own triangles
		if (isLeaf()) return triangles.size();

		// Internal nodes contain triangles from all children
		int count = 0;
		for (const BVHNode& child : children) {
			count += child.getTriangleCount();
		}
		return count;
	}

	// Split this node into 8 children based on spatial octants
	//
	void split(int max_depth, int depth = 0) noexcept 
	{
		// Only leaf nodes can be split
		if (!isLeaf()) return;

		// Limit the depth of the tree
		if (depth >= max_depth) return;

		// Split the node into 8 children
		std::vector<BVHNode> new_children;
		glm::vec3 center = aabb.center();
		glm::vec3 extents = aabb.extents();

		// Top Front Left
		glm::vec3 tfl = center + glm::vec3(-extents.x, extents.y, -extents.z);
		// Top Front Right
		glm::vec3 tfr = center + glm::vec3(extents.x, extents.y, -extents.z);
		// Top Back Left
		glm::vec3 tbl = center + glm::vec3(-extents.x, extents.y, extents.z);
		// Top Back Right
		glm::vec3 tbr = center + glm::vec3(extents.x, extents.y, extents.z);
		// Bottom Front Left
		glm::vec3 bfl = center + glm::vec3(-extents.x, -extents.y, -extents.z);
		// Bottom Front Right
		glm::vec3 bfr = center + glm::vec3(extents.x, -extents.y, -extents.z);
		// Bottom Back Left
		glm::vec3 bbl = center + glm::vec3(-extents.x, -extents.y, extents.z);
		// Bottom Back Right
		glm::vec3 bbr = center + glm::vec3(extents.x, -extents.y, extents.z);

		new_children.emplace_back(BVHNode(AABB(tfl, center)));
		new_children.emplace_back(BVHNode(AABB(tfr, center)));
		new_children.emplace_back(BVHNode(AABB(tbl, center)));
		new_children.emplace_back(BVHNode(AABB(tbr, center)));
		new_children.emplace_back(BVHNode(AABB(bfl, center)));
		new_children.emplace_back(BVHNode(AABB(bfr, center)));
		new_children.emplace_back(BVHNode(AABB(bbl, center)));
		new_children.emplace_back(BVHNode(AABB(bbr, center)));

		// Add triangles to intersecting child nodes
		for (auto& tri : triangles) {
			for (BVHNode& child : new_children) {
				if (Collisions::intersects(child.aabb, tri)) {
					child.triangles.emplace_back(tri);
				}
			}
		}

		// Make current node a non-leaf
		children = new_children;
		triangles.clear();

		// Recursively split
		if (!children.empty()) {
			for (BVHNode& child : children) {
				child.split(max_depth, depth + 1);
			}
		}
	}

	// Shake the subtree at this node to remove any empty child nodes
	// Creates a sparse BVH, more optimized than a dense octree
	void shake() noexcept 
	{
		if (!children.empty()) {
			// Remove nodes with no triangles
			for (int i = 0; i < children.size(); i++) {
				if (children[i].getTriangleCount() == 0) {
					children.erase(children.begin() + i);
					--i;
				}
			}

			// Shake children
			for (BVHNode& child : children) {
				child.shake();
			}
		}
	}

	// Check for collision between a ray and the triangles in the BVH subtree
	//
	bool checkCollision(
		const Ray& ray,
		float& t_nearest,
		Triangle& collision_tri) const noexcept 
	{
		// Check if ray intersects this node's bounding box
		if (!Collisions::intersects(aabb, ray)) {
			return false;
		}

		// Check if ray intersects with children
		bool b_collision = false;
		if (!children.empty()) {
			for (auto& child : children) {
				if (child.checkCollision(ray, t_nearest, collision_tri)) {
					b_collision = true;
				}
			}
			if (b_collision) return true;
		}

		// Check if ray intersects with triangles if this is a leaf node
		if (!isLeaf()) return b_collision;
		for (auto& tri : triangles) {
			float t;
			glm::vec2 bary_pos;
			bool b_intersect = glm::intersectRayTriangle(
				ray.pos,
				glm::normalize(ray.dir),
				tri.v0, tri.v1, tri.v2,
				bary_pos,
				t
			);
			if (b_intersect) {
				// Skip if point of intersection is behind ray's origin
				if (t < -0.01f) continue;

				// Set intersection to closest point of intersection
				if (t < t_nearest) {
					b_collision = true;
					t_nearest = t;
					collision_tri = tri;
				}
			}
		}
		return b_collision;
	}
};


// Container class for BVH tree
// Acts as an interface for BVH root
struct BVH {
	BVHNode root;

	BVH(const std::vector<Triangle>& triangles = {}, int max_depth = 3) noexcept 
	{
		root = BVHNode(triangles, max_depth);
	}

	bool checkCollision(
		const Ray& ray,
		float& t_nearest,
		Triangle& collision_tri) const noexcept 
	{
		t_nearest = std::numeric_limits<float>::max();
		return root.checkCollision(ray, t_nearest, collision_tri);
	}
};

};	// end of namespace

