#pragma once

#include "ecs/Registry.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>

namespace APE {

struct HierarchyComponent {
	ECS::EntityHandle parent;
};

struct TagComponent {
	std::string name;
};

struct TransformComponent {
	glm::vec3 position;
	glm::vec3 scale;
	glm::quat rotation;

	TransformComponent(glm::vec3 position = { 0, 0, 0 },
		glm::vec3 scale = { 1, 1, 1 },
	   	glm::quat rotation = { 1, 0, 0, 0 } ) noexcept
		: position(position)
		, scale(scale)
		, rotation(rotation)
	{

	}

	[[nodiscard]] glm::mat4 getModelMatrix() const noexcept
	{
		glm::mat4 T { glm::translate(glm::mat4(1.f), position) };
		glm::mat4 R = glm::mat4_cast(rotation);
		glm::mat4 S { glm::scale(glm::mat4(1.f), scale) };

		return T * R * S;
	}

	[[nodiscard]] TransformComponent operator*(
		const TransformComponent& other) const noexcept
	{
		glm::vec3 scale = this->scale * other.scale;
		glm::quat rot = this->rotation * other.rotation;
		glm::vec3 pos = this->position + 
			(this->rotation * this->scale * other.position);

		return TransformComponent(pos, scale, rot);
	}
};

};	// end of namespace

