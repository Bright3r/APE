#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>
#include <iostream>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	if (!SDL_CreateWindowAndRenderer("APE Physics Engine", 640, 480, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		std::cerr << "SDL_CreateWindowAndRenderer Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

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

		SDL_RenderClear(renderer);

		SDL_FRect rect = { 100, 100, 200, 200 };
		SDL_SetRenderDrawColor(renderer, 0, 255,255, 255);
		SDL_RenderFillRect(renderer, &rect);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0; 
}
