#pragma once

#include "physics/Colliders.h"
#include "physics/RigidBody.h"
#include "physics/BVH.h"

namespace APE::Physics {

struct State {
	RigidBody rbd;
	Collider::BVH bvh;

	State(const std::vector<Collider::Triangle>& tris) noexcept
		: rbd(RigidBody())
		, bvh(Collider::BVH(tris, 3))
	{

	}
};

};
