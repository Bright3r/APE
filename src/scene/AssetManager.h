#pragma once

#include "util/Logger.h"
#include "scene/AssetHandle.h"

#include <functional>
#include <memory>
#include <typeindex>

namespace APE {

class AssetManager {
private:
	struct InternalAsset {
		AssetClass asset_class;
		std::type_index type_id;
		std::shared_ptr<void> data;
	};

	struct AssetKeyHash {
		size_t operator()(const AssetKey& k) const noexcept
		{
			return std::hash<std::string>()(k.to_string());
		}
	};

	static inline 
	std::unordered_map<AssetKey, InternalAsset, AssetKeyHash> s_assets;

public:
	AssetManager() noexcept = default;

	[[nodiscard]] static bool 
	contains(const AssetKey& key) noexcept
	{
		if (!s_assets.contains(key)) return false;
		return s_assets.at(key).data != nullptr;
	}

	template <typename Asset>
	static AssetHandle<Asset> 
	upload(const AssetKey& key,
		AssetClass asset_class,
		std::unique_ptr<Asset> data) noexcept
	{
		APE_CHECK((!contains(key)),
			"AssetManager::upload() Failed: Cannot reupload asset {}.",
			key.to_string()
		);

		s_assets.emplace(
			key,
			InternalAsset { 
				.asset_class = asset_class,
				.type_id = typeid(Asset),
				.data = std::shared_ptr<void>(std::move(data)),
			}
		);
		return makeHandle<Asset>(key);
	}

	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset> 
	get(const AssetKey& key) noexcept
	{
		return makeHandle<Asset>(key);
	}

private:
	template <typename Asset>
	[[nodiscard]] static AssetHandle<Asset>
	makeHandle(const AssetKey& key) noexcept
	{
		auto it = s_assets.find(key);
		APE_CHECK((it != s_assets.end()),
			"AssetManager::get() Failed: Asset {} not yet loaded.",
			key.to_string()
		);

		auto& asset = it->second;
		APE_CHECK((asset.type_id == typeid(Asset)),
			"AssetManager::makeHandle() Failed: Type mismatch for {}.",
			key.to_string()
		);

		return AssetHandle<Asset>(
			key, 
			asset.asset_class, 
			std::static_pointer_cast<Asset>(asset.data)
		);
	}
};

};	// end of namespace

