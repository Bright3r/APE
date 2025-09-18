#pragma once

#include "physics/State.h"

namespace APE::Physics {

struct RigidBodyComponent {
	Physics::State phys_state;

	RigidBodyComponent(
		const std::vector<Collider::Triangle>& tris) noexcept
		: phys_state(tris)
	{

	}
};

};	// end of namespace
