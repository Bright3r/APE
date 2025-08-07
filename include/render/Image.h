#pragma once

#include "util/Logger.h"
#include <SDL3/SDL_surface.h>
#include <filesystem>

#include <SDL3/SDL_gpu.h>

namespace APE {
namespace Render {

class Image {
private:
	SDL_Surface* m_data;

public:
	Image(std::filesystem::path path, int num_channels)
	{
		m_data = loadImage(path, num_channels);
	}

	~Image()
	{
		SDL_DestroySurface(m_data);
	}

	// Delete copy
	Image(const Image&) = delete;
	Image& operator=(const Image&) = delete;

	static SDL_Surface* loadImage(std::filesystem::path path, int channels) 
	{
		std::string abs_path = std::filesystem::absolute(path);
		SDL_Surface* surface = SDL_LoadBMP(abs_path.c_str());
		if (!surface) {
			APE_ERROR("Failed to load image: - {}", abs_path);
			return nullptr;
		}

		if (channels != 4) {
			APE_ERROR("Failed to load image: invalid channel count");
			SDL_DestroySurface(surface);
			return nullptr;
		}

		SDL_PixelFormat format = SDL_PIXELFORMAT_ABGR8888;
		if (surface->format != format) {
			SDL_Surface* conv = SDL_ConvertSurface(surface, format);
			SDL_DestroySurface(surface);
			surface = conv;
		}

		return surface;
	}

	Uint32 getSize() const
	{
		return getWidth() * getHeight() * 4;
	}

	Uint32 getWidth() const
	{
		return m_data->w;
	}

	Uint32 getHeight() const
	{
		return m_data->h;
	}

	void* getPixels() const
	{
		return m_data->pixels;
	}
};

};	// end of namespace Render
};	// end of namespace APE
