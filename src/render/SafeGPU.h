#pragma once

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <memory>

namespace APE::Render::SafeGPU {

// Unique Resources wrapped with deleter for memory safety
template <typename SDL_T>
using UniqueGPUResource = std::unique_ptr<SDL_T, std::function<void(SDL_T*)>>;

template <typename SDL_T>
[[nodiscard]] inline UniqueGPUResource<SDL_T> makeUnique(
	SDL_T* resource,
	std::function<void(SDL_T*)> deleter) noexcept
{
	return UniqueGPUResource<SDL_T>(resource, deleter);
}

// Shared Resources wrapped with deleter for memory safety
template <typename SDL_T>
using SharedGPUResource = std::shared_ptr<SDL_T>;

template <typename SDL_T>
[[nodiscard]] inline SharedGPUResource<SDL_T> makeShared(
	SDL_T* resource,
	std::function<void(SDL_T*)> deleter) noexcept
{
	return SharedGPUResource<SDL_T>(resource, deleter);
}

// Alias commonly-used templates
using UniqueGPUBuffer = UniqueGPUResource<SDL_GPUBuffer>;
using SharedGPUBuffer = SharedGPUResource<SDL_GPUBuffer>;
using UniqueGPUGraphicsPipeline = UniqueGPUResource<SDL_GPUGraphicsPipeline>;
using UniqueGPUTexture = UniqueGPUResource<SDL_GPUTexture>;
using UniqueGPUSampler = UniqueGPUResource<SDL_GPUSampler>;

};	// end of namespace

