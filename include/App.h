#pragma once

#include "render/Shader.h"
#include "render/Model.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <memory>

class App {
public:
	// Template Methods
	void setup();
	void update();
	void draw();
	void onKeyDown(SDL_KeyboardEvent key);
	void onKeyUp(SDL_KeyboardEvent key);
	void onMouseDown(SDL_MouseButtonEvent mButton);
	void onMouseUp(SDL_MouseButtonEvent mButton);
	void onMouseMove(SDL_MouseMotionEvent mEvent);

private:
	// App Data
	std::unique_ptr<APE::Render::Shader> shader;
	APE::Render::Model model;

	bool b_quit = false;
	bool b_move_left = false;
	bool b_move_right = false;
	bool b_move_forward = false;
	bool b_move_backward = false;
	bool b_move_up = false;
	bool b_move_down = false;
	bool b_displace_right = false;
	bool b_displace_left = false;
};

