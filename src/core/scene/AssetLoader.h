#pragma once

#include "core/render/Model.h"
#include "core/scene/ImageLoader.h"
#include "core/scene/ModelLoader.h"

#include <type_traits>

namespace APE {

struct AssetLoader {
	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset>
	load(AssetKey key, AssetClass asset_class) noexcept
	{
		if constexpr (std::is_same_v<Asset, Render::Model>) {
			return ModelLoader::load(key);
		}
		else if constexpr (std::is_same_v<Asset, Render::Image>) {
			return ImageLoader::load(key);
		}
		else {
			APE_ABORT(
				"AssetLoader::load() Failed: Cannot load classless asset."
			);
		}
	}
};

};
