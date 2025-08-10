#pragma once

#include "util/Logger.h"
#include <SDL3/SDL_surface.h>
#include <filesystem>

#include <SDL3/SDL_gpu.h>

namespace APE {
namespace Render {

struct Pixel {
	Uint8 r, g, b, a;
};

class Image {
private:
	SDL_Surface* m_data;

	inline static Pixel s_default_tex[] = {
		{ 255, 255, 255, 255 },
		{ 255, 0, 255, 255 },
		{ 255, 255, 0, 255 },
		{ 255, 255, 255, 255 },
	};

public:
	Image()
	{
		// m_data = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_RGBA8888);
		// std::memcpy(m_data->pixels, &s_default_tex, sizeof(s_default_tex));
		m_data = loadImage("res/textures/ravioli.bmp", 4);
		APE_TRACE("DEFAULT_TEX USED");
	}

	Image(std::filesystem::path path, int num_channels)
	{
		m_data = loadImage(path, num_channels);
	}

	Image(int width, int height, void* data)
	{
		m_data = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);
		std::memcpy(m_data->pixels, data, width * height * 4);
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

	void trace() const
	{
		unsigned char* pixel = static_cast<unsigned char*>(m_data->pixels);
		std::string str;
		for (int x = 0; x < getWidth(); ++x) {
			for (int y = 0; y < getHeight(); ++y) {
				int num = static_cast<int>(
					pixel[(y * getWidth() + x) * 4]
				);
				str += std::to_string(num) + " ";
			}
		}

		APE_TRACE(
			"width = {} \n height = {} \n Pixels = {}",
			getWidth(),
			getHeight(),
			str
		);
	}
};

};	// end of namespace Render
};	// end of namespace APE
