#pragma once

#include "render/SafeGPU.h"
#include <SDL3/SDL_stdinc.h>
#include <cstddef>
#include <filesystem>
#include <string_view>
#include <vector>

namespace APE::Render {

class Image {
public:
	static inline std::string_view DEFAULT_IMG_PATH = "res/textures/checkerboard.png";
private:
	static constexpr int DEFAULT_IMG_CHANNELS = 4;

	int m_width;
	int m_height;
	int m_num_channels;
	std::vector<std::byte> m_pixels;
	std::filesystem::path m_path;
	SafeGPU::UniqueGPUTexture m_texture_buffer;

public:
	Image() noexcept;

	Image(std::filesystem::path path) noexcept;

	Image(std::filesystem::path path, 
		int width,
		int height,
		const std::byte* data) noexcept;

	[[nodiscard]] std::filesystem::path getPath() const noexcept;

	[[nodiscard]] static std::filesystem::path getDefaultPath() noexcept;

	void loadImage(std::filesystem::path path) noexcept;

	void loadCheckerboard() noexcept;

	[[nodiscard]] SafeGPU::UniqueGPUTexture& textureBuffer() noexcept;

	[[nodiscard]] Uint32 getSizeBytes() const noexcept;

	[[nodiscard]] Uint32 getWidth() const noexcept;

	[[nodiscard]] Uint32 getHeight() const noexcept;

	[[nodiscard]] Uint32 getNumChannels() const noexcept;

	[[nodiscard]] std::byte* getPixels() noexcept;

	void trace() const noexcept;
};

};	// end of namespace

