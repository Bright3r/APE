#pragma once

#include "util/Logger.h"

#include <filesystem>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace APE {

enum class AssetClass {
	None = 0,
	Model,
	Texture,
};

template <typename Asset>
struct AssetHandle {
	AssetClass asset_class;
	std::filesystem::path asset_path;
	std::shared_ptr<Asset> data;
};

class AssetManager {
private:
	struct InternalAsset {
		AssetClass asset_class;
		std::type_index type_id;
		std::shared_ptr<void> data;
	};

	static inline 
	std::unordered_map<std::filesystem::path, InternalAsset> s_assets;

public:
	[[nodiscard]] static bool 
	contains(std::filesystem::path asset_path) noexcept
	{
		return s_assets.contains(asset_path);
	}

	template <typename Asset>
	static AssetHandle<Asset> 
	upload(std::filesystem::path asset_path,
		AssetClass asset_class,
		std::unique_ptr<Asset> data) noexcept
	{
		APE_CHECK((!contains(asset_path)),
			"AssetManager::upload() Failed: Cannot reupload asset {}.",
			asset_path.c_str()
		);

		s_assets.emplace(
			asset_path,
			InternalAsset { 
				.asset_class = asset_class,
				.type_id = typeid(Asset),
				.data = std::shared_ptr<void>(std::move(data)),
			}
		);
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
		APE_CHECK((asset.type_id == typeid(Asset)),
			"AssetManager::makeHandle() Failed: Type mismatch for {}.",
			asset_path.c_str()
		);

		return {
			.asset_class = asset.asset_class,
			.asset_path = asset_path,
			.data = std::static_pointer_cast<Asset>(asset.data),
		};
	}
};

};	// end of namespace

