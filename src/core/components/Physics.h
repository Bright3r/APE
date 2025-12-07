#pragma once

#include "core/ecs/Registry.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

namespace APE::Phys
{

struct PhysicsComponent 
{
	static constexpr const char* Name = "PhysicsObject";

	JPH::BodyID body_id;

	PhysicsComponent(JPH::BodyID body_id = {}) noexcept
		: body_id(body_id)
	{

	}
};

};	// end of namespace APE::Phys

