#pragma once

#include "physics/RigidBody.h"

namespace APE::Physics {

struct Integrator {
	virtual void integrate(
		RigidBody& rb,
		float dt) const noexcept = 0;
};

struct EulerIntegrator : public Integrator {
	void integrate(
		RigidBody& rb,
		float dt) const noexcept
	{
		glm::vec3 accel = rb.forces / rb.mass;
		glm::vec3 next_vel_linear = rb.vel_linear + accel * dt;

		glm::vec3 vel_avg = (rb.vel_linear + next_vel_linear) / 2.f;
		rb.pos += vel_avg * dt;
		rb.vel_linear = next_vel_linear;
	}
};

};	// end of namespace

