#include "render/Context.h"
#include "util/Logger.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>

namespace APE {
namespace Render {

Context::Context(std::string_view title,
		 int window_width,
		 int window_height,
		 int window_flags)
	: title(title)
	, window_width(window_width)
	, window_height(window_height)
	, window_flags(window_flags)
{
	bool succ_init = SDL_Init(SDL_INIT_VIDEO);
	APE_CHECK(succ_init,
		"SDL_Init Failed - {}",
		SDL_GetError()
	);

	device = SDL_CreateGPUDevice(
		SDL_GPU_SHADERFORMAT_SPIRV|
		SDL_GPU_SHADERFORMAT_DXIL |
		SDL_GPU_SHADERFORMAT_MSL,
		true,
		NULL
	);
	APE_CHECK((device != nullptr),
		"SDL_CreateGPUDevice Failed - {}",
		SDL_GetError()
	);

	window = SDL_CreateWindow(
		this->title.c_str(),
		window_width,
		window_height, 
		window_flags
	);
	APE_CHECK((window != nullptr),
		"SDL_CreateWindow Failed - {}",
		SDL_GetError()
	);

	bool succ_claim_window = SDL_ClaimWindowForGPUDevice(device, window);
	APE_CHECK(succ_claim_window,
		"SDL_ClaimWindowForGPUDevice Failed - {}",
		SDL_GetError()
	);
}

Context::~Context()
{
	SDL_ReleaseWindowFromGPUDevice(device, window);
	SDL_DestroyWindow(window);
	SDL_DestroyGPUDevice(device);
}

};	// end of namespace Render
};	// end of namespace APE
