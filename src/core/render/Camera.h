#pragma once

#include "util/Logger.h"
#include "core/render/Context.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace APE::Render 
{

struct Camera 
{
	glm::vec3 m_position;
	float m_pitch;		// rotation around x-axis
	float m_yaw;		// rotation around y-axis
	float m_fov;		// field of view in degrees
	float m_sensitivity;
	float m_near_plane;
	bool m_is_locked;

	Camera(
		const glm::vec3& pos = { 0, 0, 1 },
		float pitch = 0.f,
		float yaw = -90.f,
		float fov = 45.f,
		float sensitivity = 0.3f,
		float near_clip = 0.1f
	) noexcept
		: m_position(pos)
		, m_pitch(pitch)
		, m_yaw(yaw)
		, m_fov(fov)
		, m_sensitivity(sensitivity)
		, m_near_plane(near_clip)
		, m_is_locked(false)
	{ 

	}

	[[nodiscard]] glm::vec3 getPosition() const noexcept
	{
		return m_position;
	}

	void setPosition(const glm::vec3& pos) noexcept
	{
		m_position = pos;
	}

	[[nodiscard]] float getPitch() const noexcept
	{
		return m_pitch;
	}

	void setPitch(float pitch) noexcept
	{
		m_pitch = pitch;
	}

	[[nodiscard]] float getYaw() const noexcept
	{
		return m_yaw;
	}

	void setYaw(float yaw) noexcept
	{
		m_yaw = yaw;
	}

	[[nodiscard]] float getFOV() const noexcept
	{
		return m_fov;
	}

	void setFOV(float fov) noexcept
	{
		m_fov = fov;
	}

	[[nodiscard]] float getSensitivity() const noexcept
	{
		return m_sensitivity;
	}

	void setSensitivity(float sensitivity) noexcept
	{
		m_sensitivity = sensitivity;
	}

	[[nodiscard]] float getNearPlane() const noexcept
	{
		return m_near_plane;
	}

	void setNearPlane(float near_clip) noexcept
	{
		m_near_plane = near_clip;
	}

	[[nodiscard]] bool isLocked() const noexcept
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
		while (m_yaw > 360.f) 
		{
			m_yaw -= 360.f;
		}
		while (m_yaw < -360.f) 
		{
			m_yaw += 360.f;
		}
	}

	[[nodiscard]] glm::mat4 getViewMatrix() const noexcept
	{
		glm::vec3 front = getForwardVector();
		return glm::lookAt(m_position, m_position + front, getUpVector());
	}

	[[nodiscard]] glm::mat4 getProjectionMatrix(
		float aspect_ratio
	) const noexcept
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
		const glm::vec3 forward_vec
	) const noexcept
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
		const glm::vec3 right_vec
	) const noexcept
	{
		return glm::normalize(
			glm::cross(right_vec, forward_vec)
		);
	}

	[[nodiscard]] glm::vec3 screenToWorld(
		const glm::vec2& screen_coords,
		const APE::Render::Context& context
	) noexcept
	{
		// Screen coords to ndc
		glm::vec3 ndc = {
			(2.f * screen_coords.x) / context.window_width - 1.f,
			1.f - (2.f * screen_coords.y) / context.window_height,
			1.f
		};

		// Ndc to view space
		glm::mat4 inv_proj = glm::inverse(
			getProjectionMatrix(context.getAspectRatio())
		);

		glm::vec4 clip(ndc, 1.f);
		glm::vec4 eye = inv_proj * clip;
		glm::vec4 view(eye / eye.w);

		// View to world
		glm::mat4 inv_view = glm::inverse(getViewMatrix());
		glm::vec4 world = inv_view * view;

		return glm::vec3(world);
	}

	void print() noexcept 
	{
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

