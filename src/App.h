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
	std::unique_ptr<APE::Render::Model> model;
};

