#pragma once

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <memory>

namespace APE {
namespace Render {

namespace SafeGPU {

// Wrap SDL_GPUBuffer with deleter for memory safety
//
// Unique Pointer
using UniqueGPUBuffer = std::unique_ptr<
	SDL_GPUBuffer, 
	std::function<void(SDL_GPUBuffer*)>>;
inline UniqueGPUBuffer makeUniqueGPUBuffer(
	SDL_GPUBuffer* gpu_buf,
	SDL_GPUDevice* device)
{
	auto deleter = [device](SDL_GPUBuffer* buf) {
		if (buf && device) {
			SDL_ReleaseGPUBuffer(device, buf);
		}
	};
	return std::unique_ptr<SDL_GPUBuffer, decltype(deleter)>(gpu_buf, deleter);
}

// Shared Pointer
using SharedGPUBuffer = std::shared_ptr<SDL_GPUBuffer>;
inline SharedGPUBuffer makeSharedGPUBuffer(
	SDL_GPUBuffer* gpu_buf,
	SDL_GPUDevice* device)
{
	auto deleter = [device](SDL_GPUBuffer* buf) {
		if (buf && device) {
			SDL_ReleaseGPUBuffer(device, buf);
		}
	};
	return std::shared_ptr<SDL_GPUBuffer>(gpu_buf, deleter);
}

};	// end of namespace SafeGPU

};	// end of namespace Render
};	// end of namespace APE
