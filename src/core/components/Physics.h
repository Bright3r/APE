#pragma once

#include "core/components/Object.h"
#include "physics/State.h"

namespace APE::Physics {

struct RigidBodyComponent {
	Physics::State phys_state;

	// Update entity's transform to match physic's system's internal rigid body
	TransformComponent* ent_transform;

	RigidBodyComponent(
		const std::vector<Collider::Triangle>& tris,
		TransformComponent* transform) noexcept
		: phys_state(tris)
		, ent_transform(transform)
	{

	}
};

};	// end of namespace
