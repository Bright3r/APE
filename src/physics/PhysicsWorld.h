#pragma once

#include "core/ecs/Registry.h"
#include "physics/Integrator.h"
#include "physics/RigidBody.h"
#include "physics/collisions/Collisions.h"
#include <memory>
#include <utility>

namespace APE::Physics {

class PhysicsWorld {
	ECS::Registry world;
	std::unique_ptr<Integrator> integrator;

public:
	PhysicsWorld(
		std::unique_ptr<Integrator> integrator = std::make_unique<EulerIntegrator>()) noexcept
		: integrator(std::move(integrator))
	{
		
	}


	using ColliderHandle = std::shared_ptr<Collisions::Collider>;

	void stepSimulation(float dt) noexcept
	{
		// Collision Detection/Resolution
		auto view = world.view<RigidBody, ColliderHandle>().each();
		for (size_t i = 0; i < view.size(); ++i) {
			auto [ent_a, rbd_a, collider_a] = view[i];
			auto& a = *collider_a.get();
			a.pos = rbd_a.pos;
			for (size_t j = i+1; j < view.size(); ++j) {
				auto [ent_b, rbd_b, collider_b] = view[j];
				auto& b = *collider_b.get();
				b.pos = rbd_b.pos;

				if (Collisions::intersects(a, b)) {
					APE_TRACE("{} and {} collide.", ent_a.id, ent_b.id);
				}
			}
		}

		// Solve Constraints
		
		// Apply gravity - special case for now
		applyGravity();
		
		// Integrate
		integrate(dt);
	}

	ECS::EntityHandle createRigidBody(const RigidBody& rbd) noexcept
	{
		auto ent = world.createEntity();
		world.emplaceComponent<RigidBody>(ent, rbd);
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

private:
	void applyGravity() noexcept
	{
		glm::vec3 gravity_force(0, -9.8f, 0);

		auto view = world.view<RigidBody>();
		for (auto [ent, rbd] : view.each()) {
			rbd.forces += gravity_force;
		}
	}

	void integrate(float dt) noexcept
	{	
		auto view = world.view<RigidBody>();
		for (auto [ent, rbd] : view.each()) {
			integrator->integrate(rbd, dt);
		}
	}
};

};	// end of namespace

