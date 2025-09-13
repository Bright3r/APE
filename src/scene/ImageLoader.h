#pragma once

#include "render/Image.h"
#include "scene/AssetHandle.h"
#include "scene/AssetManager.h"

#include <filesystem>
#include <memory>

namespace APE {

struct ImageLoader {
	[[nodiscard]] static AssetHandle<Render::Image>
	load(std::filesystem::path path) noexcept
	{
		AssetKey key { path };
		if (!AssetManager::contains(key)) {
			AssetManager::upload<Render::Image>(
				key,
				AssetClass::Texture,
				std::move(std::make_unique<Render::Image>(path))
			);
		}
		return AssetManager::get<Render::Image>(key);
	}

	[[nodiscard]] static AssetHandle<Render::Image>
	defaultImage() noexcept
	{
		return load(Render::Image::DEFAULT_IMG_PATH);
	}
};

};	// end of namespace
