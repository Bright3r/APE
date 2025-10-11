#pragma once

#include "core/ecs/Registry.h"
#include "physics/Integrator.h"
#include "physics/RigidBody.h"
#include "physics/collisions/Collisions.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace APE::Physics {

struct PhysicsWorld {
	static constexpr const char* Name = "PhysicsWorld";

	ECS::Registry world;
	std::unique_ptr<Integrator> integrator;

	explicit PhysicsWorld(std::unique_ptr<Integrator> integrator = 
		std::make_unique<EulerIntegrator>()) noexcept
		: integrator(std::move(integrator))
	{
		
	}

	void stepSimulation(float dt) noexcept
	{
		// Collision Detection/Resolution
		auto view = world.view<RigidBody, Collisions::AABB>().each();
		for (size_t i = 0; i < view.size(); ++i) {
			auto [ent_a, rbd_a, collider_a] = view[i];
			auto& a = collider_a;
			a.pos = rbd_a.pos;
			for (size_t j = i+1; j < view.size(); ++j) {
				auto [ent_b, rbd_b, collider_b] = view[j];
				auto& b = collider_b;
				b.pos = rbd_b.pos;

				Collisions::CollisionInfo collision;
				if (Collisions::intersects(a, b, collision)) {
					collision.a = &rbd_a;
					collision.b = &rbd_b;
					resolveCollision(collision);
					APE_TRACE("{} and {} collide.", ent_a.id, ent_b.id);
				}
			}
		}

		// Solve Constraints
		
		// Apply gravity - special case for now
		// applyGravity();
		
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
		const Collisions::AABB& collider) noexcept
	{
		world.emplaceComponent<Collisions::AABB>(ent, collider);
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

	void resolveCollision(Collisions::CollisionInfo& collision) noexcept
	{
		auto& rbd_a = *collision.a;
		auto& rbd_b = *collision.b;
		auto& p = collision.contact;

		// Make collision normal point from A --> B
		if (glm::dot(p.normal, (rbd_b.pos - rbd_a.pos)) < 0) {
			p.normal *= -1;
		}

		// Separate out objects (projection)
		float total_inv_mass = rbd_a.inv_mass + rbd_b.inv_mass;
		rbd_a.pos -= p.normal * p.penetration * (rbd_a.inv_mass / total_inv_mass);
		rbd_b.pos += p.normal * p.penetration * (rbd_b.inv_mass / total_inv_mass);

		// Conserve momentum
		glm::vec3 relative_a = p.pos - rbd_a.pos;
		glm::vec3 relative_b = p.pos - rbd_b.pos;

		glm::vec3 vel_angular_a = glm::cross(rbd_a.vel_angular, relative_a);
		glm::vec3 vel_angular_b = glm::cross(rbd_b.vel_angular, relative_b);

		glm::vec3 full_vel_a = rbd_a.vel_linear + vel_angular_a;
		glm::vec3 full_vel_b = rbd_b.vel_linear + vel_angular_b;
		glm::vec3 contact_vel = full_vel_b - full_vel_a;

		// Calculate impulse
		float impulse_force = glm::dot(contact_vel, p.normal);

		glm::vec3 inertia_a = glm::cross(rbd_a.inverseInertiaTensorWorld() * 
			glm::cross(relative_a, p.normal), relative_a);

		glm::vec3 inertia_b = glm::cross(rbd_b.inverseInertiaTensorWorld() * 
			glm::cross(relative_b, p.normal), relative_b);

		float angular_effect = glm::dot(inertia_a + inertia_b, p.normal);
		// float angular_effect = 0.f;

		float restitution = std::min(rbd_a.restitution, rbd_b.restitution);
		float j = ( -(1.f + restitution) * impulse_force) / (total_inv_mass + angular_effect);
		glm::vec3 impulse = p.normal * j;

		// Apply impulse
		rbd_a.applyLinearImpulse(-impulse);
		rbd_b.applyLinearImpulse(impulse);

		rbd_a.applyAngularImpulse(glm::cross(relative_a, -impulse));
		rbd_b.applyAngularImpulse(glm::cross(relative_b, impulse));
	}

};

};	// end of namespace

