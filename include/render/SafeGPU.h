#pragma once

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <memory>

namespace APE {
namespace Render {

namespace SafeGPU {

struct SDL_GPUBuffer_Deleter {
	SDL_GPUDevice *device;

	void operator()(SDL_GPUBuffer* buf) const
	{
		if (buf && device) {
			SDL_ReleaseGPUBuffer(device, buf);
		}
	}
};

// Wrap SDL_GPUBuffer with deleter for memory safety
using UniqueGPUBuffer = std::unique_ptr<SDL_GPUBuffer, SDL_GPUBuffer_Deleter>;
using SharedGPUBuffer = std::shared_ptr<SDL_GPUBuffer>;

inline SharedGPUBuffer make_shared_gpu_buffer(
	SDL_GPUBuffer* gpu_buf,
	std::function<void(SDL_GPUBuffer*)> deleter)
{
	return std::shared_ptr<SDL_GPUBuffer>(gpu_buf, deleter);
}

};	// end of namespace SafeGPU

};	// end of namespace Render
};	// end of namespace APE
