#pragma once

#include <glm/glm.hpp>

namespace APE::Physics {

struct State {
	glm::vec3 pos;
	glm::vec3 vel;
	float mass;
};

};	// end of namespace
