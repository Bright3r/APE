#pragma once

#include "core/render/Image.h"
#include "core/scene/AssetHandle.h"

#include <filesystem>

namespace APE {

struct ImageLoader {
	[[nodiscard]] static AssetHandle<Render::Image>
	load(std::filesystem::path path) noexcept;

	[[nodiscard]] static AssetHandle<Render::Image>
	load(AssetKey asset_key) noexcept;

	[[nodiscard]] static AssetHandle<Render::Image>
	defaultImage() noexcept;
};

};	// end of namespace
