#include "AppRunner.h"
#include <chrono>

namespace AppRunner {

void init(int window_width, int window_height) 
{
	m_quit = false;
	m_framerate = 60.f;
	m_last_frame_time = std::chrono::milliseconds(0);

	m_app = std::make_unique<App>();
	m_renderer = std::make_unique<Renderer>(window_width, window_height);
}

void pollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				setQuit(true);
				break;
			case SDL_EVENT_KEY_DOWN:
				m_app->onKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
				m_app->onKeyUp(event.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				m_app->onMouseDown(event.button);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				m_app->onMouseUp(event.button);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				m_app->onMouseMove(event.motion.x, event.motion.y);
				break;
		}
	}
}

void stepGameloop()
{
	// Poll User Input
	pollEvents();

	// Update App Data
	m_app->update();

	// Draw To Screen
	m_renderer->clear();
	m_app->draw();
	m_renderer->display();
}

void run() 
{
	// Initial Setup
	m_app->setup();

	// Game Loop
	while (!m_quit) {
		auto start = std::chrono::high_resolution_clock::now();

		// Time the execution of gameloop iteration
		APE::Timing::millis loop_time_ms; 
		APE::Timing::timeFunctionCall([&]() {
			stepGameloop();
		}, loop_time_ms);

		// Wait until the next frame time
		APE::Timing::seconds target_frame_time { 1.0 / m_framerate };
		APE::Timing::waitFor(target_frame_time - loop_time_ms);

		// Track total time of frame
		auto end = std::chrono::high_resolution_clock::now();
		m_last_frame_time = end - start;
	}
}

bool quit() 
{
	return m_quit;
}

void setQuit(bool quit) 
{
	m_quit = quit;
}

int framerate() 
{
	return m_framerate;
}

void setFramerate(int fps) 
{
	m_framerate = fps;
}


APE::Timing::seconds lastFrameTimeSec() 
{
	return std::chrono::duration_cast<APE::Timing::seconds>(m_last_frame_time);
}

APE::Timing::millis lastFrameTimeMS() 
{
	return std::chrono::duration_cast<APE::Timing::millis>(m_last_frame_time);
}

};
