#pragma once

#include "App.h"
#include "render/Renderer.h"
#include "util/timing.h"

#include <memory>

#include <SDL3/SDL_events.h>

namespace AppRunner {
	// Main Functions
	//
	void init(int window_width, int window_height);
	bool pollEvents(SDL_Event& event);
	bool stepGameloop(SDL_Event& event);
	void run();


	// Utility Functions
	//
	void setFrameRate(int fps);
	int frameRate();
	APE::Timing::seconds lastFrameTimeSec();
	APE::Timing::millis lastFrameTimeMS();

	// Application State
	//
	static std::unique_ptr<App> app;
	static std::unique_ptr<Renderer> renderer;

	static int framerate;
	static APE::Timing::millis last_frame_time;
};
