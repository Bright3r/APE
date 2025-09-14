#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

class App {
public:
	virtual ~App() = default;

	virtual void setup() = 0;
	virtual void update() = 0;
	virtual void drawGUI() = 0;
	virtual void onKeyDown(SDL_KeyboardEvent key) = 0;
	virtual void onKeyUp(SDL_KeyboardEvent key) = 0;
	virtual void onMouseDown(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseUp(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseMove(SDL_MouseMotionEvent mEvent) = 0;
};

