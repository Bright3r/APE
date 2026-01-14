#pragma once

#include "core/ecs/Registry.h"
#include "phys/PlayerController.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <memory>
#include <utility>

namespace APE::Phys
{

struct PhysicsComponent 
{
	static constexpr const char *Name = "PhysicsObject";

	JPH::BodyID body_id;

	PhysicsComponent(JPH::BodyID body_id = {}) noexcept
		: body_id(body_id)
	{

	}
};

struct PlayerComponent
{
	static constexpr const char *Name = "PlayerComponent";

	Phys::PlayerController controller;

	PlayerComponent(
		Phys::PlayerController controller = {}
	) noexcept
		: controller(std::move(controller))
	{

	}
};

};	// end of namespace APE::Phys

