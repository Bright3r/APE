#pragma once

#include "scene/ImageLoader.h"
#include "scene/ModelLoader.h"

namespace APE {

struct AssetLoader {
	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset>
	load(AssetKey key, AssetClass asset_class) noexcept
	{
		switch (asset_class) {
		case APE::AssetClass::Model:
			return ModelLoader::load(key);
			break;
		case APE::AssetClass::Texture:
			return ImageLoader::load(key);
			break;
		default:
			APE_ABORT(
				"AssetLoader::load() Failed: Cannot load classless asset."
			);
		};
	}
};

};
