#include "core/input/Input.h"

namespace APE::Input {

bool State::isKeyDown(SDL_Keycode key) const noexcept
{
	if (m_key_state.contains(key)) {
		return m_key_state.at(key);
	}
	return false;
}

bool State::isKeyUp(SDL_Keycode key) const noexcept
{
	return !isKeyDown(key);
}

const std::vector<SDL_MouseButtonEvent>& State::mouseButtonEvents() const noexcept
{
	return m_mouse_buttons;
}

const std::vector<SDL_MouseMotionEvent>& State::mouseMotionEvents() const noexcept
{
	return m_mouse_motions;
}

void State::keyDown(SDL_Keycode key) noexcept
{
	m_key_state[key] = true;
}

void State::keyUp(SDL_Keycode key) noexcept
{
	m_key_state[key] = false;
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

