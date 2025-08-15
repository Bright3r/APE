#pragma once

#include "util/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace APE {
namespace Render {

class Camera {
private:
	glm::vec3 m_position;
	float m_pitch;		// rotation around x-axis
	float m_yaw;		// rotation around y-axis
	float m_fov;		// field of view in degrees
	float m_sensitivity;
	bool m_is_locked;

public:
	Camera(const glm::vec3 pos = { 0, 0, 1 },
		float pitch = 0.f,
		float yaw = -90.f,
		float fov = 45.f,
		float sensitivity = .3f)
		: m_position(pos)
		, m_pitch(pitch)
		, m_yaw(yaw)
		, m_fov(fov)
		, m_sensitivity(sensitivity)
		, m_is_locked(false)
	{ }

	bool getLocked()
	{
		return m_is_locked;
	}

	void setLocked(bool is_locked)
	{
		m_is_locked = is_locked;
	}

	void moveUp(float speed, float delta_time)
	{
		if (m_is_locked) return;
		m_position += getUpVector() * speed * delta_time;
	}

	void moveDown(float speed, float delta_time)
	{
		if (m_is_locked) return;
		m_position -= getUpVector() * speed * delta_time;
	}

	void moveRight(float speed, float delta_time)
	{
		if (m_is_locked) return;
		m_position += getRightVector() * speed * delta_time;
	}

	void moveLeft(float speed, float delta_time)
	{
		if (m_is_locked) return;
		m_position -= getRightVector() * speed * delta_time;
	}

	void moveForward(float speed, float delta_time) 
	{
		if (m_is_locked) return;
		m_position += getForwardVector() * speed * delta_time;
	}

	void moveBackward(float speed, float delta_time) 
	{
		if (m_is_locked) return;
		m_position -= getForwardVector() * speed * delta_time;
	}

	void rotate(float delta_x, float delta_y)
	{
		if (m_is_locked) return;

		m_yaw += delta_x * m_sensitivity;
		m_pitch -= delta_y * m_sensitivity;

		// Clamp pitch
		if (m_pitch > 89.f) m_pitch = 89.f;
		if (m_pitch < -89.f) m_pitch = -89.f;

		// Clamp yaw
		while (m_yaw > 360.f) {
			m_yaw -= 360.f;
		}
		while (m_yaw < -360.f) {
			m_yaw += 360.f;
		}
	}

	glm::mat4 getViewMatrix() const 
	{
		glm::vec3 front = getForwardVector();
		return glm::lookAt(m_position, m_position + front, getUpVector());
	}

	glm::mat4 getProjectionMatrix(float aspect_ratio) const 
	{
		return glm::perspective(
			glm::radians(m_fov),
			aspect_ratio,
			0.1f,
			100.0f
		);
	}

	glm::vec3 getForwardVector() const 
	{
		return glm::normalize(glm::vec3 {
			cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
			sin(glm::radians(m_pitch)),
			sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
		});
	}

	glm::vec3 getRightVector() const 
	{
		return glm::normalize(
			glm::cross(getForwardVector(), getUpVector())
		);
	}

	glm::vec3 getRightVector(const glm::vec3 forward_vec) const 
	{
		return glm::normalize(
			glm::cross(forward_vec, getUpVector())
		);
	}

	constexpr glm::vec3 getUpVector() const 
	{
		return glm::vec3(0, 1, 0);
	}

	glm::vec3 getUpVector(
		const glm::vec3 forward_vec,
		const glm::vec3 right_vec) const 
	{
		return glm::normalize(
			glm::cross(right_vec, forward_vec)
		);
	}

	void print() {
		APE_TRACE(
			"Camera state:\n"
			"  Position: ({}, {}, {})\n"
			"  Pitch: {}\n"
			"  Yaw: {}\n"
			"  FOV: {}\n"
			"  Sensitivity: {}"
			"  Is_Locked: {}",
			m_position.x, m_position.y, m_position.z,
			m_pitch,
			m_yaw,
			m_fov,
			m_sensitivity,
			m_is_locked
		);
	}
};

};	// end of namespace Render
};	// end of namespace APE
