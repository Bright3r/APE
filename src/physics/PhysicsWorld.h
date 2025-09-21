#pragma once

#include "core/ecs/Registry.h"
#include "physics/RigidBody.h"

namespace APE::Physics {

class PhysicsWorld {
	ECS::Registry world;

public:
	using ColliderHandle = std::shared_ptr<Collisions::Collider>;

	void stepSimulation(float dt) noexcept
	{
		// Collision Detection/Resolution

		// Solve Constraints
		
		// Integrate

	}

	ECS::EntityHandle createRigidBody(const RigidBody& rbd = {}) noexcept
	{
		auto ent = world.createEntity();
		world.emplaceComponent<RigidBody>(ent);
		return ent;
	}

	void addCollider(
		const ECS::EntityHandle& ent,
		ColliderHandle collider) noexcept
	{
		world.emplaceComponent<ColliderHandle>(ent, collider);
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

