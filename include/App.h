#pragma once

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

class App {
public:
	// Template Methods
	void setup();
	void update();
	void draw(SDL_GPURenderPass *render_pass);
	void onKeyDown(SDL_KeyboardEvent key);
	void onKeyUp(SDL_KeyboardEvent key);
	void onMouseDown(SDL_MouseButtonEvent mButton);
	void onMouseUp(SDL_MouseButtonEvent mButton);
	void onMouseMove(float x, float y);

private:
	// App Data
	
};

