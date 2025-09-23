#pragma once

#include "core/ecs/Registry.h"
#include "physics/RigidBody.h"
#include "physics/collisions/Collisions.h"
#include <cmath>

namespace APE::Physics {

class PhysicsWorld {
	ECS::Registry world;

public:
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
		integrateAcceleration(dt);
		integrateVelocity(dt);
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

	void integrateAcceleration(float dt) noexcept
	{
		auto view = world.view<RigidBody>();
		for (auto [ent, rbd] : view.each()) {
			float inv_mass = 1.f / rbd.mass;
			glm::vec3 accel = inv_mass * rbd.forces;

			// Integrate acceleration
			glm::vec3 delta_vel = accel * dt;

			// Apply change in velocity
			rbd.vel_linear += delta_vel;

			// Reset rigid body forces for next frame
			rbd.forces = glm::vec3(0);
		}
	}

	void integrateVelocity(float dt) noexcept
	{
		float damping_constant = 1.f - 0.95f;
		float damping_factor = std::powf(damping_constant, dt);

		auto view = world.view<RigidBody>();
		for (auto [ent, rbd] : view.each()) {
			// Integrate velocity
			glm::vec3 delta_pos = rbd.vel_linear * dt;

			// Apply change in position
			rbd.pos += delta_pos;

			// Dampen velocity
			// rbd.vel_linear *= damping_factor;
		}
	}
};

};	// end of namespace

