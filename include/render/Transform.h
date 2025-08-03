#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

namespace APE {
namespace Render {

struct Transform {
	glm::vec3 position;
	glm::vec3 scale;
	glm::quat rotation;

	Transform(glm::vec3 position = { 0, 0, 0 },
		glm::vec3 scale = { 1, 1, 1 },
	   	glm::quat rotation = { } )
		: position(position)
		, scale(scale)
		, rotation(rotation)
	{

	}

	glm::mat4 getModelMatrix() const
	{
		glm::mat4 T { glm::translate(glm::mat4(1.f), position) };
		glm::mat4 R = glm::mat4_cast(rotation);
		glm::mat4 S { glm::scale(glm::mat4(1.f), scale) };

		return T * R * S;
	}
};

};	// end of namespace Render
};	// end of namespace APE
