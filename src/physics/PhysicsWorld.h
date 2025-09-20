#pragma once

#include "core/ecs/Registry.h"
#include "physics/RigidBody.h"

namespace APE::Physics {

class PhysicsWorld {
	ECS::Registry world;

public:
	void stepSimulation(float dt) noexcept
	{
		// Integrate Velocities
		
		// Collision Detection/Resolution

		// Solve Constraints
		
		// Integrate Positions
	}

	ECS::EntityHandle createRigidBody(const RigidBody& rbd = {}) noexcept
	{
		auto ent = world.createEntity();
		world.emplaceComponent<RigidBody>(ent);
		return ent;
	}

	void addCollider(const ECS::EntityHandle& ent, const Collider::BVH& bvh) noexcept
	{
		world.emplaceComponent<Collider::BVH>(ent, bvh);
	}

	void addJoint() noexcept
	{

	}

	template <typename Component>
	Component& get(const ECS::EntityHandle& ent) noexcept
	{
		return world.getComponent<Component>(ent);
	}
};

};	// end of namespace

