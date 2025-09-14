#pragma once

#include "core/scene/Scene.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

namespace APE {

struct Application {
	Scene world;

	virtual ~Application() = default;

	virtual void setup() = 0;
	virtual void update() = 0;
	virtual void drawGUI() = 0;
	virtual void onKeyDown(SDL_KeyboardEvent key) = 0;
	virtual void onKeyUp(SDL_KeyboardEvent key) = 0;
	virtual void onMouseDown(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseUp(SDL_MouseButtonEvent mButton) = 0;
	virtual void onMouseMove(SDL_MouseMotionEvent mEvent) = 0;
};

};	// end of namespace
