#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

#include <string>
#include <string_view>

namespace APE {
namespace Render {

struct Context {
	SDL_Window *window;
	SDL_GPUDevice *device;
	std::string title;
	int window_width;
	int window_height;
	int window_flags;

	Context(std::string_view title, int window_width, 
		int window_height, int window_flags);
	~Context();
	Context(const Context& other) = delete;
	Context operator=(const Context& other) = delete;
	Context(const Context&& other) = delete;
	Context operator=(const Context&& other) = delete;
};


};	// end of namespace Render
};	// end of namespace APE
