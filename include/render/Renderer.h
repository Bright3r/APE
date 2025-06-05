#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include <glad/glad.h>
#include <iostream>

class Renderer {
public:
	Renderer(int width, int height);
	~Renderer();

	int init();
	void draw() const;
	void clear() const;

private:
	SDL_Window *window;
	SDL_GLContext context;

	int width, height;
};
