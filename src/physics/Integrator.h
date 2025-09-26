#pragma once

#include "physics/RigidBody.h"

namespace APE::Physics {

struct Integrator {
	virtual void integrate(RigidBody& rb, float dt) const noexcept = 0;
};

struct EulerIntegrator : public Integrator {
	void integrate(RigidBody& rb, float dt) const noexcept
	{
		integrateAcceleration(rb, dt);
		integrateVelocity(rb, dt);
	}

private:
	void integrateAcceleration(RigidBody& rb, float dt) const noexcept
	{
		// Linear Motion
		{
			glm::vec3 accel = rb.inv_mass * rb.forces;

			// Integrate acceleration
			glm::vec3 delta_vel = accel * dt;

			// Apply change in velocity
			rb.vel_linear += delta_vel;

			// Reset rigid body forces for next frame
			rb.forces = glm::vec3(0);
		}

		// Rotational Motion
		{
			glm::mat3 inv_intertia_tensor = 
				glm::inverse(rb.inertiaTensorWorld());
			glm::vec3 accel_angular = inv_intertia_tensor * rb.torques;

			// Integrate angular acceleration
			glm::vec3 delta_vel = accel_angular * dt;

			// Apply change in angular velocity
			rb.vel_angular += delta_vel;

			// Reset rigid body torques for next frame
			rb.torques = glm::vec3(0);
		}
	}

	void integrateVelocity(RigidBody& rb, float dt, float damping_constant = 0.f) const noexcept
	{
		damping_constant = 1.f - damping_constant;
		float damping_factor = std::powf(damping_constant, dt);

		// Linear Motion
		{
			// Integrate velocity
			glm::vec3 delta_pos = rb.vel_linear * dt;

			// Apply change in position
			rb.pos += delta_pos;

			// Dampen velocity
			rb.vel_linear *= damping_factor;
		}

		// Rotational Motion
		{
			glm::vec3 ang_vel = rb.vel_angular;
			glm::quat spin(0.f, ang_vel.x, ang_vel.y, ang_vel.z);

			// Integrate angular velocity
			glm::quat delta_orientation = (spin * rb.orientation) * (dt * 0.5f);
			
			// Apply change in orientation
			rb.orientation = glm::normalize(rb.orientation + delta_orientation); 
			// Dampen angular velocity
			rb.vel_angular *= damping_factor;
		}
	}
};

};	// end of namespace

