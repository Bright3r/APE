#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <iostream>

int width = 1200;
int height = 800;

SDL_Window *window = NULL;

int main() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	window = SDL_CreateWindow("APE Physics Engine", width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		std::cerr << "SDL_CreateWindowAndRenderer Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		std::cerr << "SDL_GL_CreateContext Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
	    std::cerr << "Failed to initialize GLAD" << std::endl;
	    return -1;
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

		// Draw
		glViewport(0, 0, width, height);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0; 
}
