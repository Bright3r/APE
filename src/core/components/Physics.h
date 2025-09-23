#pragma once

#include "core/ecs/Registry.h"
#include "physics/PhysicsWorld.h"
#include "physics/collisions/Colliders.h"

namespace APE::Physics {

struct RigidBodyComponent {
	PhysicsWorld* physics_world;
	ECS::EntityHandle physics_ent;

	RigidBodyComponent(
		PhysicsWorld* physics_world = nullptr,
		ECS::EntityHandle ent = {}) noexcept
		: physics_world(physics_world)
		, physics_ent(ent)
	{

	}

	Collisions::Collider* collider() noexcept
	{
		return physics_world->get<PhysicsWorld::ColliderHandle>(physics_ent).get();
	}

	RigidBody& get() noexcept
	{
		return physics_world->get<RigidBody>(physics_ent);
	}
};

};	// end of namespace

