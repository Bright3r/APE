#include "AppRunner.h"
#include <chrono>

namespace AppRunner {

void init(int windowWidth, int windowHeight) {
	framerate = 60.f;
	lastFrameTimeSec = 0.f;

	app = std::make_unique<App>();
	renderer = std::make_unique<Renderer>(windowWidth, windowHeight);
}

void run() {
	// Initial Setup
	app->setup();

	// Game Loop
	bool quit = false;
	SDL_Event event;
	while (!quit) {
		auto startTime = std::chrono::high_resolution_clock::now();

		// Poll Events
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

		// Update App Data
		app->update();

		// Draw To Screen
		renderer->clear();
		app->draw();
		renderer->display();


		// Calculate Frame Computation Time
		auto endTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float>(endTime - startTime).count();

		// Wait Until Next Frame Window
		float targetFrameTime = 1.f / framerate;
		float delayTime = (targetFrameTime - deltaTime) * 1000.f;
		if (delayTime > 1.0f) {
			// Sleep until 1ms before next frame
			SDL_Delay(static_cast<Uint32>(delayTime - 1.0f));
		}
		// Spin-wait to delay the last 1ms accurately
		while (std::chrono::high_resolution_clock::now() - endTime < 
			std::chrono::duration<float>(targetFrameTime - deltaTime)) { }

		// Calculate the total frame time
		endTime = std::chrono::high_resolution_clock::now();
		lastFrameTimeSec = std::chrono::duration<float>(endTime - startTime).count();
	}
}

void setFrameRate(int fps) {
	framerate = fps;
}

int getFrameRate() {
	return framerate;
}

float getLastFrameTimeSec() {
	return lastFrameTimeSec;
}

float getLastFrameTimeMS() {
	return lastFrameTimeSec * 1000.f;
}

};
