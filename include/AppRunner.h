#pragma once

#include "App.h"
#include "render/Renderer.h"

#include <memory>
#include <chrono>

#include <SDL3/SDL_events.h>

namespace AppRunner {
	// Main Functions
	//
	void init(int windowWidth, int windowHeight);
	void run();


	// Utility Functions
	//
	void setFrameRate(int fps);
	int getFrameRate();

	float getLastFrameTimeSec();
	float getLastFrameTimeMS();

	// Application State
	//
	static std::unique_ptr<App> app;
	static std::unique_ptr<Renderer> renderer;

	static int framerate;
	static float lastFrameTimeSec;
};
