#include "render/Context.h"
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <iostream>

namespace APE {
namespace Render {

Context::Context(const std::string& title,
		 int window_width,
		 int window_height,
		 int window_flags)
	: title(title),
	window_width(window_width),
	window_height(window_height),
	window_flags(window_flags)
{
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "SDL_Init Failed: " << SDL_GetError() << "\n";
		return;
	}

	device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV
		| SDL_GPU_SHADERFORMAT_DXIL
		| SDL_GPU_SHADERFORMAT_MSL,
		true,
		NULL
	);
	if (!device) {
		std::cerr << "SDL_CreateGPUDevice Failed: " << SDL_GetError() << "\n";
		return;
	}

	window = SDL_CreateWindow(title.c_str(), window_width, window_height, window_flags);
	if (!window) {
		std::cerr << "SDL_CreateWindow Failed: " << SDL_GetError() << "\n";
		return;
	}

	if (!SDL_ClaimWindowForGPUDevice(device, window)) {
		std::cerr << "SDL_ClaimWindowForGPUDevice Failed: " << 
			SDL_GetError() << "\n";
		return;
	}
}

Context::~Context()
{
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyWindow(window);
	SDL_DestroyGPUDevice(device);
}

};	// end of namespace Render
};	// end of namespace APE
