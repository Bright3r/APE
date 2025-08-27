#pragma once

#include "physics/State.h"

namespace APE::Physics {

struct Integrator {
	virtual State integrate(
		const State& start,
		glm::vec3 forces,
		float dt) const noexcept = 0;
};

struct Euler : public Integrator {
	State integrate(
		const State& start,
		glm::vec3 forces,
		float dt) const noexcept
	{
		glm::vec3 accel = forces / start.mass;
		glm::vec3 vel_next = start.vel + accel * dt;

		glm::vec3 vel_avg = (start.vel + vel_next) / 2.f;
		glm::vec3 pos_next = start.pos + vel_avg * dt;
		return {
			pos_next,
			vel_next,
			start.mass
		};
	}
};

};	// end of namespace

