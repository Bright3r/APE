#include "render/Renderer.h"
#include <SDL3/SDL_video.h>

Renderer::Renderer(int width, int height) 
	: width(width), height(height) 
{
	int success = init();
	if (success != 0) {
		std::cerr << "Failed to initialize Renderer." << std::endl;
	}
}

Renderer::~Renderer() {
	SDL_GL_DestroyContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

int Renderer::init() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	window = SDL_CreateWindow("APE Physics Engine", width, height, 
			   SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		std::cerr << "SDL_CreateWindowAndRenderer Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	context = SDL_GL_CreateContext(window);
	if (!context) {
		std::cerr << "SDL_GL_CreateContext Failed " << SDL_GetError() << std::endl;
		return SDL_APP_FAILURE;
	}

	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
	    std::cerr << "Failed to initialize GLAD" << std::endl;
	    return -1;
	}

	return 0;
}

void Renderer::draw() const {
	clear();

	SDL_GL_SwapWindow(window);
}

void Renderer::clear() const {
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}
