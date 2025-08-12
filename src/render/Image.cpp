#include "render/Image.h"
#include "util/Logger.h"
#include <cstring>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace APE {
namespace Render {

Image::Image()
{
	loadCheckerboard();
}

Image::Image(std::filesystem::path path)
{
	loadImage(path);
}

Image::Image(int width, int height, const std::byte* data)
{
	// Assume data is R8G8BA8 format
	int num_channels = DEFAULT_IMG_CHANNELS;
	if (height > 0) {
		m_width = width;
		m_height = height;
		m_num_channels = num_channels;

		// Copy stb image data to a new vector
		m_pixels.resize(getSizeBytes());
		std::memcpy(m_pixels.data(), data, getSizeBytes());

		return;
	}

	// if height is 0, image is compressed
	unsigned char* decompressed_data = 
		stbi_load_from_memory(
			reinterpret_cast<const unsigned char*>(data),
			width,	// num bytes of compressed image
			&width,
			&height,
			&num_channels,
			DEFAULT_IMG_CHANNELS	// force R8G8B8A8
		);

	if (decompressed_data == nullptr) {
		// Fallback to default texture
		APE_ERROR("Failed to load embedded texture.");
		loadCheckerboard();
	}

	m_width = width;
	m_height = height;
	m_num_channels = DEFAULT_IMG_CHANNELS;

	// Copy stb image data to a new vector
	m_pixels.resize(getSizeBytes());
	std::memcpy(m_pixels.data(), decompressed_data, getSizeBytes());

	// Cleanup image data
	stbi_image_free(decompressed_data);
}

void Image::loadImage(std::filesystem::path path)
{
	std::string abs_path = std::filesystem::absolute(path);
	int width, height, num_channels;
	std::byte* data = reinterpret_cast<std::byte*>(stbi_load(
		abs_path.c_str(),
		&width,
		&height,
		&num_channels,
		DEFAULT_IMG_CHANNELS	// force R8G8B8A8
	));

	// Fallback to default texture if stbi_load fails
	if (data == nullptr) {
		APE_ERROR("Failed to load image: {}", abs_path.c_str());

		loadCheckerboard();
		return;
	}

	m_width = width;
	m_height = height;
	m_num_channels = DEFAULT_IMG_CHANNELS;

	// Copy stb image data to a new vector
	m_pixels.resize(getSizeBytes());
	std::memcpy(m_pixels.data(), data, getSizeBytes());

	// Cleanup image data
	stbi_image_free(data);
}


void Image::loadCheckerboard()
{
	m_width = 2;
	m_height = 2;
	m_num_channels = DEFAULT_IMG_CHANNELS;

	static constexpr std::byte on  { 0xff };
	static constexpr std::byte off { 0x00 };
	m_pixels = { 
		off, off, off,  on,
		 on, off,  on,  on,
		 on, off,  on,  on,
		off, off, off,  on,
	};
}

Uint32 Image::getSizeBytes() const
{
	return m_width * m_height * m_num_channels;
}

Uint32 Image::getWidth() const
{
	return m_width;
}

Uint32 Image::getHeight() const
{
	return m_height;
}

Uint32 Image::getNumChannels() const
{
	return m_num_channels;
}

std::byte* Image::getPixels()
{
	return m_pixels.data();
}

void Image::trace() const
{
	std::string pixel_str;
	for (size_t i = 0; i < m_pixels.size(); ++i) {
		pixel_str += std::to_string(static_cast<unsigned char>(m_pixels[i]));
		pixel_str += " ";
	}

	APE_TRACE(
		"width = {}, height = {}, num_channels = {} \n Pixels = {}",
		getWidth(),
		getHeight(),
		getNumChannels(),
		pixel_str
	);
}

};	// end of namespace Render
};	// end of namespace APE

