#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace APE::Physics {

struct RigidBody {
	glm::vec3 pos;
	glm::quat orientation;
	glm::vec3 vel_linear;
	glm::vec3 vel_angular;

	// Properties
	float mass;
	float restitution;
	glm::mat3 moment;

	// Impulses
	glm::vec3 forces;
	glm::vec3 torques;

	RigidBody(const glm::vec3& pos = glm::vec3(0),
		float mass = 1,
		float restitution = 1,
		const glm::quat& orientation = {},
		const glm::mat3& moment = {},
		const glm::vec3& vel_linear = glm::vec3(0),
		const glm::vec3& vel_angular = glm::vec3(0),
		const glm::vec3& forces = glm::vec3(0),
		const glm::vec3& torques = glm::vec3(0))
		: pos(pos)
		, orientation(orientation)
		, vel_linear(vel_linear)
		, vel_angular(vel_angular)
		, mass(mass)
		, restitution(restitution)
		, moment(moment)
		, forces(forces)
		, torques(torques)
	{

	}

	void addForce(const glm::vec3& force) noexcept
	{
		forces += force;
	}

	void addForce(const glm::vec3& force, const glm::vec3& position) noexcept
	{
		forces += force;
		torques += glm::cross(pos, force);
	}

	[[nodiscard]] glm::mat3 inertiaTensorLocal() const noexcept
	{
		// Hard coded for 1x1x1 box
		float w = 1;
		float h = 1;
		float d = 1;
		float ix = (1.f / 12) * mass * (h*h + d*d);
		float iy = (1.f / 12) * mass * (w*w + d*d);
		float iz = (1.f / 12) * mass * (w*w + h*h);
		glm::mat3 local_tensor(
			ix, 0,  0,
			0,  iy, 0,
			0,  0,  iz
		);
		return local_tensor;
	}

	[[nodiscard]] glm::mat3 inertiaTensorWorld() const noexcept
	{
		glm::mat3 local_tensor = inertiaTensorLocal();

		glm::mat3 orientation_mat = glm::mat3_cast(orientation);
		return orientation_mat * local_tensor * glm::transpose(orientation_mat);
	}
};

};	// end of namespace

