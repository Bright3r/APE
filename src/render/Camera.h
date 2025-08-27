#pragma once

#include "util/Logger.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace APE::Render {

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
		float sensitivity = .3f) noexcept
		: m_position(pos)
		, m_pitch(pitch)
		, m_yaw(yaw)
		, m_fov(fov)
		, m_sensitivity(sensitivity)
		, m_is_locked(false)
	{ }

	[[nodiscard]] bool getLocked() noexcept
	{
		return m_is_locked;
	}

	void setLocked(bool is_locked) noexcept
	{
		m_is_locked = is_locked;
	}

	void moveUp(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position += getUpVector() * speed * delta_time;
	}

	void moveDown(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position -= getUpVector() * speed * delta_time;
	}

	void moveRight(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position += getRightVector() * speed * delta_time;
	}

	void moveLeft(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position -= getRightVector() * speed * delta_time;
	}

	void moveForward(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position += getForwardVector() * speed * delta_time;
	}

	void moveBackward(float speed, float delta_time) noexcept
	{
		if (m_is_locked) return;
		m_position -= getForwardVector() * speed * delta_time;
	}

	void rotate(float delta_x, float delta_y) noexcept
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

	[[nodiscard]] glm::mat4 getViewMatrix() const noexcept
	{
		glm::vec3 front = getForwardVector();
		return glm::lookAt(m_position, m_position + front, getUpVector());
	}

	[[nodiscard]] glm::mat4 getProjectionMatrix(
		float aspect_ratio) const noexcept
	{
		return glm::perspective(
			glm::radians(m_fov),
			aspect_ratio,
			0.1f,
			100.0f
		);
	}

	[[nodiscard]] glm::vec3 getForwardVector() const noexcept
	{
		return glm::normalize(glm::vec3 {
			cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
			sin(glm::radians(m_pitch)),
			sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
		});
	}

	[[nodiscard]] glm::vec3 getRightVector() const noexcept
	{
		return glm::normalize(
			glm::cross(getForwardVector(), getUpVector())
		);
	}

	[[nodiscard]] glm::vec3 getRightVector(
		const glm::vec3 forward_vec) const noexcept
	{
		return glm::normalize(
			glm::cross(forward_vec, getUpVector())
		);
	}

	[[nodiscard]] constexpr glm::vec3 getUpVector() const noexcept
	{
		return glm::vec3(0, 1, 0);
	}

	[[nodiscard]] glm::vec3 getUpVector(
		const glm::vec3 forward_vec,
		const glm::vec3 right_vec) const noexcept
	{
		return glm::normalize(
			glm::cross(right_vec, forward_vec)
		);
	}

	void print() noexcept {
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

};	// end of namespace

