#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>

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
};

};	// end of namespace

