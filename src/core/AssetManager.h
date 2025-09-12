#pragma once

#include "util/Logger.h"

#include <filesystem>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace APE {

template <typename Asset>
struct AssetHandle {
	std::shared_ptr<Asset> data;
	std::filesystem::path asset_path;
};

class AssetManager {
private:
	struct InternalAsset {
		std::shared_ptr<void> data;
		std::type_index type;
	};

	static inline 
	std::unordered_map<std::filesystem::path, InternalAsset> s_assets;

public:
	template <typename Asset>
	static AssetHandle<Asset> 
	upload(std::shared_ptr<Asset> data, std::filesystem::path asset_path) noexcept
	{
		APE_CHECK((!s_assets.contains(asset_path)),
			"AssetManager::upload() Failed: Cannot reupload asset {}.",
			asset_path.c_str()
		);

		s_assets[asset_path] = {
			.data = data,
			.type = typeid(Asset),
		};
		return makeHandle<Asset>(asset_path);
	}

	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset> 
	get(std::filesystem::path asset_path) noexcept
	{
		return makeHandle<Asset>(asset_path);
	}

private:
	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset>
	makeHandle(std::filesystem::path asset_path) noexcept
	{
		auto it = s_assets.find(asset_path);
		APE_CHECK((it != s_assets.end()),
			"AssetManager::get() Failed: Asset {} not yet loaded.",
			asset_path.c_str()
		);

		auto& asset = it->second;
		APE_CHECK((asset.type == typeid(Asset)),
			"AssetManager::makeHandle() Failed: Type mismatch for {}.",
			asset_path.c_str()
		);

		return {
			.data = std::static_pointer_cast<Asset>(asset.data),
			.asset_path = asset_path,
		};
	}
};

};	// end of namespace

