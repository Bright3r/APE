#pragma once

#include "core/ecs/Registry.h"
#include "physics/PhysicsWorld.h"

namespace APE::Physics {

struct RigidBodyComponent {
	PhysicsWorld* physics_world;
	ECS::EntityHandle physics_ent;

	RigidBodyComponent(PhysicsWorld* physics_world = nullptr, ECS::EntityHandle ent = {}) noexcept
		: physics_world(physics_world)
		, physics_ent(ent)
	{

	}

	template <typename Component>
	Component& get() noexcept
	{
		return physics_world->get<Component>(physics_ent);
	}
};

};	// end of namespace
