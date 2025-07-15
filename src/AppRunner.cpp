#include "AppRunner.h"
#include <chrono>

namespace AppRunner {

void init(int window_width, int window_height) 
{
	framerate = 60.f;
	last_frame_time = std::chrono::milliseconds(0);

	app = std::make_unique<App>();
	renderer = std::make_unique<Renderer>(window_width, window_height);
}

bool pollEvents(SDL_Event& event)
{
	bool quit = false;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				quit = true;
				break;
			case SDL_EVENT_KEY_DOWN:
				app->onKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
				app->onKeyUp(event.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				app->onMouseDown(event.button);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				app->onMouseUp(event.button);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				app->onMouseMove(event.motion.x, event.motion.y);
				break;
		}
	}
	return quit;
}

bool stepGameloop(SDL_Event& event)
{
	bool quit = pollEvents(event);

	// Update App Data
	app->update();

	// Draw To Screen
	renderer->clear();
	app->draw();
	renderer->display();

	return quit;
}

void run() 
{
	// Initial Setup
	app->setup();

	// Game Loop
	bool quit = false;
	SDL_Event event;
	while (!quit) {
		auto start = std::chrono::high_resolution_clock::now();

		// Time the execution of gameloop iteration
		APE::Timing::millis loop_time_ms; 
		quit = APE::Timing::timeFunctionCall([&]() {
			return stepGameloop(event);
		}, loop_time_ms);

		// Wait until the next frame time
		APE::Timing::seconds target_frame_time { 1.0 / framerate };
		APE::Timing::waitFor(target_frame_time - loop_time_ms);

		// Track total time of frame
		auto end = std::chrono::high_resolution_clock::now();
		last_frame_time = end - start;
	}
}

void setFrameRate(int fps) 
{
	framerate = fps;
}

int frameRate() 
{
	return framerate;
}

APE::Timing::seconds lastFrameTimeSec() 
{
	return std::chrono::duration_cast<APE::Timing::seconds>(last_frame_time);
}

APE::Timing::millis lastFrameTimeMS() 
{
	return std::chrono::duration_cast<APE::Timing::millis>(last_frame_time);
}

};
