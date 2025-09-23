#include "core/input/Input.h"
#include <algorithm>

namespace APE::Input {

bool State::isKeyDown(SDL_Keycode key) const noexcept
{
	if (m_key_state.contains(key)) {
		return m_key_state.at(key).down;
	}
	return false;
}

bool State::isKeyUp(SDL_Keycode key) const noexcept
{
	return !isKeyDown(key);
}

bool State::isFirstFramePressed(SDL_Keycode key) const noexcept
{
	if (m_key_state.contains(key)) {
		return m_key_state.at(key).time_pressed == m_timestamp;
	}
	return false;
}

const std::vector<SDL_MouseButtonEvent>& State::mouseButtonEvents() const noexcept
{
	return m_mouse_buttons;
}

const std::vector<SDL_MouseMotionEvent>& State::mouseMotionEvents() const noexcept
{
	return m_mouse_motions;
}

void State::nextFrame() noexcept
{
	++m_timestamp;
}

void State::keyPress(const SDL_KeyboardEvent& event) noexcept
{
	// Update global timestamp
	m_timestamp = std::max(m_timestamp, event.timestamp);

	// Skip repeat events
	if (event.down == isKeyDown(event.key)) return;

	// Update key state
	KeyState key_state = {
		.down = event.down,
		.time_pressed = event.timestamp,
	};
	m_key_state[event.key] = key_state;
}

void State::mouseButton(const SDL_MouseButtonEvent& event) noexcept
{
	m_mouse_buttons.push_back(event);
}

void State::mouseMotion(const SDL_MouseMotionEvent& event) noexcept
{
	m_mouse_motions.push_back(event);
}

void State::flush() noexcept
{
	m_mouse_buttons.clear();
	m_mouse_motions.clear();
}

}	// end of namespace

