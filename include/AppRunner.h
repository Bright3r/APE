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
	void pollEvents();
	void stepGameloop();
	void run();


	// Utility Functions
	//
	bool quit();
	void setQuit(bool quit);
	int framerate();
	void setFramerate(int fps);
	APE::Timing::seconds lastFrameTimeSec();
	APE::Timing::millis lastFrameTimeMS();


	// Application State
	//
	static std::unique_ptr<App> m_app;
	static std::unique_ptr<Renderer> m_renderer;

	static bool m_quit;
	static int m_framerate;
	static APE::Timing::millis m_last_frame_time;
};
