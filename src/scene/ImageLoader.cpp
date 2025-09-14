#include "ImageLoader.h"
#include "AssetManager.h"

namespace APE {

AssetHandle<Render::Image> 
ImageLoader::load(std::filesystem::path path) noexcept
{
	AssetKey asset_key { path };
	return load(asset_key);
}

AssetHandle<Render::Image> 
ImageLoader::load(AssetKey asset_key) noexcept
{
	if (!AssetManager::contains(asset_key)) {
		AssetManager::upload<Render::Image>(
			asset_key,
			AssetClass::Texture,
			std::move(std::make_unique<Render::Image>(asset_key.path))
		);
	}
	return AssetManager::get<Render::Image>(asset_key);
}

AssetHandle<Render::Image> ImageLoader::defaultImage() noexcept
{
	return load(Render::Image::DEFAULT_IMG_PATH);
}

};	// end of namespace

