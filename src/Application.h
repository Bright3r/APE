#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

namespace APE {

struct Application {
	virtual ~Application() = default;

	virtual void setup() = 0;
	virtual void update() = 0;
	virtual void draw() = 0;
	virtual void drawGUI() = 0;
	virtual void onMouseDown(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseUp(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseMove(SDL_MouseMotionEvent mEvent) = 0;
};

};	// end of namespace
