#include "render/Renderer.h"

int main() {
	Renderer renderer(1200, 800);

	// Gameloop
	bool quit = false;
	SDL_Event event;
	while (!quit) {
		// Poll Events
		while (SDL_PollEvent(&event) != 0) {
			switch (event.type) {
				case SDL_EVENT_QUIT:
					quit = true;
					break;
			}
		}

		// Draw
		renderer.draw();
	}

	return 0; 
}
