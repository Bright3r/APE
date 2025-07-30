#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace APE {
namespace Render {

struct Camera {
	glm::vec3 position;
	float pitch;	// rotation around x-axis
	float yaw;	// rotation around y-axis
	float fov;	// field of view in degrees
	float sensitivity;

	Camera(const glm::vec3 pos = { 0, 0, 1 },
		float pitch = 0.f,
		float yaw = -90.f,
		float fov = 45.f,
		float sensitivity = .3f)
		: position(pos)
		, pitch(pitch)
		, yaw(yaw)
		, fov(fov)
		, sensitivity(sensitivity)
	{ }

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

	void rotate(float delta_x, float delta_y)
	{
		yaw += delta_x * sensitivity;
		pitch -= delta_y * sensitivity;

		// Clamp pitch
		if (pitch > 89.f) pitch = 89.f;
		if (pitch < -89.f) pitch = -89.f;

		// Clamp yaw
		while (yaw > 360.f) {
			yaw += 360.f;
		}
		while (yaw < -360.f) {
			yaw += 360.f;
		}
	}

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

	void print() {
		std::cout << "Camera state:" << std::endl;
		std::cout << "  Position: (" 
		      << position.x << ", " 
		      << position.y << ", " 
		      << position.z << ")" << std::endl;
		std::cout << "  Pitch: " << pitch << std::endl;
		std::cout << "  Yaw: " << yaw << std::endl;
		std::cout << "  FOV: " << fov << std::endl;
		std::cout << "  Sensitivity: " << sensitivity << std::endl;
	}
};

};	// end of namespace Render
};	// end of namespace APE
