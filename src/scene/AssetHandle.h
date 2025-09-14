#pragma once

#include <filesystem>
#include <string>

namespace APE {

enum class AssetClass {
	None = 0,
	Model,
	Texture,
};

struct AssetKey {
	std::filesystem::path path;
	std::string sub_index;

	AssetKey(
		std::filesystem::path path = "",
		std::string sub_index = "") noexcept
		: path(path)
		, sub_index(sub_index)
	{ }

	bool operator==(const AssetKey& other) const noexcept
	{
		return path == other.path && sub_index == other.sub_index;
	}

	bool operator!=(const AssetKey& other) const noexcept
	{
		return !(*this == other);
	}

	std::string to_string() const {
		return path.string() + "::" + sub_index;
	}
};

template <typename Asset>
struct AssetHandle {
	AssetKey key;
	AssetClass asset_class;
	std::shared_ptr<Asset> data;

	AssetHandle(
		AssetKey asset_key = {},
		AssetClass asset_class = AssetClass::None,
		std::shared_ptr<Asset> data = nullptr) noexcept
		: key(asset_key)
		, asset_class(asset_class)
		, data(data)
	{ }
};

};	// end of namespace

