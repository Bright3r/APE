#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace APE {
namespace Render {

struct Camera {
	glm::vec3 position;

	 // rotation around x-axis
	float pitch;

	// rotation around y-axis (initialize to -90 so +z is forward)
	float yaw;

	 // field of view in degrees
	float fov;

	Camera(const glm::vec3 pos = glm::vec3(0, 0, 1),
		float pitch = 0.f,
		float yaw = -90.f,
		float fov = 45.f)
		: position(pos)
		, pitch(pitch)
		, yaw(yaw)
		, fov(fov)
	{ }

	glm::mat4 getViewMatrix() const 
	{
		glm::vec3 front = getForwardVector();
		return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
	}

	glm::mat4 getProjectionMatrix(float aspect_ratio) const 
	{
		return glm::perspective(
			glm::radians(fov),
			aspect_ratio,
			0.1f,
			100.0f
		);
	}

	void moveForward(float speed, float delta_time) 
	{
		position += getForwardVector() * speed * delta_time;
	}

	void moveRight(float speed, float delta_time)
	{
		position += getRightVector() * speed * delta_time;
	}

	void moveUp(float speed, float delta_time)
	{
		position += getUpVector() * speed * delta_time;
	}

	glm::vec3 getForwardVector() const 
	{
		glm::vec3 front(
			cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
			sin(glm::radians(pitch)),
			sin(glm::radians(yaw)) * cos(glm::radians(pitch))
		);

		return glm::normalize(front);
	}

	glm::vec3 getRightVector() const 
	{
		return glm::normalize(
			glm::cross(getForwardVector(), glm::vec3(0, 1, 0))
		);
	}

	glm::vec3 getRightVector(const glm::vec3 forward_vec) const 
	{
		return glm::normalize(
			glm::cross(forward_vec, glm::vec3(0, 1, 0))
		);
	}

	glm::vec3 getUpVector() const 
	{
		return glm::normalize(
			glm::cross(getRightVector(), getForwardVector())
		);
	}

	glm::vec3 getUpVector(
		const glm::vec3 forward_vec, const glm::vec3 right_vec) const 
	{
		return glm::normalize(
			glm::cross(right_vec, forward_vec)
		);
	}
};

};	// end of namespace Render
};	// end of namespace APE
