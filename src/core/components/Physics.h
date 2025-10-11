#pragma once

#include "core/ecs/Registry.h"
#include "physics/PhysicsWorld.h"
#include "physics/collisions/Colliders.h"

namespace APE::Physics {

struct RigidBodyComponent {
	static constexpr const char* Name = "RigidBodyHandle";

	ECS::EntityHandle physics_ent;

	explicit RigidBodyComponent(ECS::EntityHandle ent = {}) noexcept
		: physics_ent(ent)
	{

	}

	Collisions::AABB& collider(PhysicsWorld& phys_world) noexcept
	{
		return phys_world.get<Collisions::AABB>(physics_ent);
	}

	RigidBody& get(PhysicsWorld& phys_world) noexcept
	{
		return phys_world.get<RigidBody>(physics_ent);
	}
};

};	// end of namespace

