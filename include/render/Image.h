#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace APE {
namespace Render {

class Image {
private:
	int m_width;
	int m_height;
	int m_num_channels;
	std::vector<std::byte> m_pixels;

public:
	Image();

	Image(std::filesystem::path path);

	Image(int width, int height, const std::byte* data);

	void loadImage(std::filesystem::path path);

	void loadCheckerboard();

	Uint32 getSizeBytes() const;

	Uint32 getWidth() const;

	Uint32 getHeight() const;

	Uint32 getNumChannels() const;

	std::vector<std::byte>& getPixels();

	void trace() const;
};

};	// end of namespace Render
};	// end of namespace APE

