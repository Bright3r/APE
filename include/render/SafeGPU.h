#pragma once

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <memory>

namespace APE {
namespace Render {

namespace SafeGPU {

// Unique Resources wrapped with deleter for memory safety
template <typename SDL_T>
using UniqueGPUResource = std::unique_ptr<SDL_T, std::function<void(SDL_T*)>>;

template <typename SDL_T>
inline UniqueGPUResource<SDL_T> makeUnique(
	SDL_T* resource,
	std::function<void(SDL_T*)> deleter)
{
	return UniqueGPUResource<SDL_T>(resource, deleter);
}

// Shared Resources wrapped with deleter for memory safety
template <typename SDL_T>
using SharedGPUResource = std::shared_ptr<SDL_T>;

template <typename SDL_T>
inline SharedGPUResource<SDL_T> makeShared(
	SDL_T* resource,
	std::function<void(SDL_T*)> deleter)
{
	return SharedGPUResource<SDL_T>(resource, deleter);
}

using SharedGPUBuffer = SharedGPUResource<SDL_GPUBuffer>;

};	// end of namespace SafeGPU

};	// end of namespace Render
};	// end of namespace APE
