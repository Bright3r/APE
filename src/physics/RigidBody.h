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
};

};	// end of namespace

