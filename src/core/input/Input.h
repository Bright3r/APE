#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keycode.h>

#include <SDL3/SDL_stdinc.h>
#include <unordered_map>
#include <vector>

// Forward declare Engine
namespace APE {
	class Engine;
};	// end of namespace


namespace APE::Input {

struct KeyState {
	bool down = false;
	Uint64 time_pressed = 0;
};

class State {
	friend class APE::Engine;

	std::unordered_map<SDL_Keycode, KeyState> m_key_state;
	std::vector<SDL_MouseButtonEvent> m_mouse_buttons;
	std::vector<SDL_MouseMotionEvent> m_mouse_motions;
	Uint64 m_timestamp;

public:
	[[nodiscard]] bool isKeyDown(SDL_Keycode key) const noexcept;

	[[nodiscard]] bool isKeyUp(SDL_Keycode key) const noexcept;

	[[nodiscard]] bool isFirstFramePressed(SDL_Keycode key) const noexcept;

	[[nodiscard]] const std::vector<SDL_MouseButtonEvent>& mouseButtonEvents() const noexcept;
	
	[[nodiscard]] const std::vector<SDL_MouseMotionEvent>& mouseMotionEvents() const noexcept;

protected:
	void nextFrame() noexcept;

	void keyPress(const SDL_KeyboardEvent& event) noexcept;

	void mouseButton(const SDL_MouseButtonEvent& event) noexcept;

	void mouseMotion(const SDL_MouseMotionEvent& event) noexcept;

	void flush() noexcept;
};

}	// end of namespace

