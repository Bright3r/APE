#pragma once

#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace APE::Render {

class Image {
private:
	static constexpr int DEFAULT_IMG_CHANNELS = 4;

	int m_width;
	int m_height;
	int m_num_channels;
	std::vector<std::byte> m_pixels;

public:
	Image() noexcept;

	Image(std::filesystem::path path) noexcept;

	Image(int width, int height, const std::byte* data) noexcept;

	void loadImage(std::filesystem::path path) noexcept;

	void loadCheckerboard() noexcept;

	[[nodiscard]] Uint32 getSizeBytes() const noexcept;

	[[nodiscard]] Uint32 getWidth() const noexcept;

	[[nodiscard]] Uint32 getHeight() const noexcept;

	[[nodiscard]] Uint32 getNumChannels() const noexcept;

	[[nodiscard]] std::byte* getPixels() noexcept;

	void trace() const noexcept;
};

};	// end of namespace

