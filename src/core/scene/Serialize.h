#pragma once

#include "core/components/Object.h"
#include "core/scene/AssetLoader.h"
#include "core/ecs/Registry.h"
#include "core/scene/Scene.h"
#include "util/Logger.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include <filesystem>
#include <fstream>
#include <unordered_map>

namespace cereal {

static inline
std::unordered_map<APE::ECS::EntityID, APE::ECS::EntityHandle> s_old_to_new;

/*
* GLM
*/
template <class Archive>
void serialize(Archive& ar, glm::vec2& v)
{
	ar(
		cereal::make_nvp("x", v.x), 
		cereal::make_nvp("y", v.y)
	);
}

template <class Archive>
void serialize(Archive& ar, glm::vec3& v)
{
	ar(
		cereal::make_nvp("x", v.x), 
		cereal::make_nvp("y", v.y),
		cereal::make_nvp("z", v.z)
	);
}

template <class Archive>
void serialize(Archive& ar, glm::vec4& v)
{
	ar(
		cereal::make_nvp("x", v.x), 
		cereal::make_nvp("y", v.y),
		cereal::make_nvp("z", v.z),
		cereal::make_nvp("w", v.w)
	);
}

template <class Archive>
void serialize(Archive& ar, glm::quat& q)
{
	ar(
		cereal::make_nvp("x", q.x), 
		cereal::make_nvp("y", q.y),
		cereal::make_nvp("z", q.z),
		cereal::make_nvp("w", q.w)
	);
}


/*
* Filepath
*/
template <class Archive>
void save(Archive& ar, const std::filesystem::path& path)
{
	ar(cereal::make_nvp("path", path.string()));
}

template <class Archive>
void load(Archive& ar, std::filesystem::path& path)
{
	std::string s;
	ar(s);
	path = std::filesystem::path(s);
}


/*
* Entity Handle
*/
template <class Archive>
void serialize(Archive& ar, APE::ECS::EntityHandle& ent)
{
	ar(cereal::make_nvp("id", ent.id));
}


/*
* Asset Handle
*/
template <class Archive>
void serialize(Archive& ar, APE::AssetKey& key)
{
	ar(
		cereal::make_nvp("path", key.path),
		cereal::make_nvp("sub_index", key.sub_index)
	);
}

template <class Archive, typename Asset>
void save(Archive& ar, const APE::AssetHandle<Asset>& asset)
{
	ar(
		cereal::make_nvp("asset_key", asset.key),
		cereal::make_nvp("asset_class", asset.asset_class)
	);
}

template <class Archive, typename Asset>
void load(Archive& ar, APE::AssetHandle<Asset>& asset)
{
	ar(
		cereal::make_nvp("asset_key", asset.key),
		cereal::make_nvp("asset_class", asset.asset_class)
	);

	asset = APE::AssetLoader::load<Asset>(asset.key, asset.asset_class);
}


/*
* Object.h Components
*/
template <class Archive>
void serialize(Archive& ar, APE::TransformComponent& t)
{
	ar(
		cereal::make_nvp("position", t.position),
		cereal::make_nvp("scale", t.scale),
		cereal::make_nvp("rotation", t.rotation)
	);
}

template <class Archive>
void save(Archive& ar, const APE::HierarchyComponent& h)
{
	ar(
		cereal::make_nvp("parent", h.parent), 
		cereal::make_nvp("children", h.children),
		cereal::make_nvp("tag", h.tag)
	);
}

template <class Archive>
void load(Archive& ar, APE::HierarchyComponent& h)
{
	ar(
		cereal::make_nvp("parent", h.parent), 
		cereal::make_nvp("children", h.children),
		cereal::make_nvp("tag", h.tag)
	);

	h.parent = s_old_to_new.at(h.parent.id);
	for (size_t i = 0; i < h.children.size(); ++i) {
		h.children[i] = s_old_to_new.at(h.children[i].id);
	}
}


/*
* Render Components
*/
template <class Archive>
void serialize(Archive& ar, APE::Render::MeshComponent& m)
{
	ar(
		cereal::make_nvp("model_handle", m.model_handle), 
		cereal::make_nvp("mesh_index", m.mesh_index)
	);
}

template <class Archive>
void serialize(Archive& ar, APE::Render::MaterialComponent& m)
{
	ar(cereal::make_nvp("texture_handle", m.texture_handle));
}

template <class Archive>
void serialize(Archive& ar, APE::Render::LightComponent& l)
{
	ar(
		cereal::make_nvp("type", l.type), 
		cereal::make_nvp("intensity", l.intensity),
		cereal::make_nvp("color", l.color),
		cereal::make_nvp("cutoff_angle", l.cutoff_angle),
		cereal::make_nvp("shape", l.shape),
		cereal::make_nvp("extent", l.extent)
	);
}

/*
* ECS Component Pool
*/
template <typename Component>
struct ECSPair {
	APE::ECS::EntityHandle ent;
	Component comp;

	template <class Archive>
	void serialize(Archive& ar) {
		ar(
			cereal::make_nvp("entity", ent),
			cereal::make_nvp("component", comp)
		);
	}
};

template <class Archive, typename Component>
void serializePool(Archive& ar, const APE::ECS::Registry& r) noexcept
{
	std::vector<ECSPair<Component>> entries;
	if (r.hasComponent<Component>()) {
		for (auto [ent, comp] : r.getPool<Component>()) {
			entries.emplace_back(ent, comp);
		}
	}

	APE_TRACE("Serialized {}", Component::Name);
	ar(cereal::make_nvp(Component::Name, entries));
}

template <class Archive, typename Component>
void deserializePool(Archive& ar, APE::ECS::Registry& r) noexcept
{
	std::vector<ECSPair<Component>> entries;
	ar(cereal::make_nvp(Component::Name, entries));

	for (auto& [ent, comp] : entries) {
		APE::ECS::EntityHandle new_ent = s_old_to_new.at(ent.id);
		r.emplaceComponent<Component>(new_ent, comp);
	}

	APE_TRACE("Deserialized {}", Component::Name);
}


/*
* ECS Registry
*/
template <class Archive>
void save(Archive& ar, const APE::ECS::Registry& r)
{
	ar(cereal::make_nvp("entities", r.entities()));

	serializePool<Archive, APE::TransformComponent>(ar, r);
	serializePool<Archive, APE::HierarchyComponent>(ar, r);
	serializePool<Archive, APE::Render::MeshComponent>(ar, r);
	serializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	serializePool<Archive, APE::Render::LightComponent>(ar, r);
}

template <class Archive>
void load(Archive& ar, APE::ECS::Registry& r)
{
	std::vector<APE::ECS::EntityHandle> old_ents;
	ar(cereal::make_nvp("entities", old_ents));

	for (auto old_ent : old_ents) {
		s_old_to_new[old_ent.id] = r.createEntity();
	}

	deserializePool<Archive, APE::TransformComponent>(ar, r);
	deserializePool<Archive, APE::HierarchyComponent>(ar, r);
	deserializePool<Archive, APE::Render::MeshComponent>(ar, r);
	deserializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	deserializePool<Archive, APE::Render::LightComponent>(ar, r);
}


/*
* Scene
*/
template <class Archive>
void save(Archive& ar, const APE::Scene& scene)
{
	ar(
		cereal::make_nvp("registry", scene.registry), 
		cereal::make_nvp("root", scene.root)
	);
}

template <class Archive>
void load(Archive& ar, APE::Scene& scene)
{
	s_old_to_new.clear();
	APE::ECS::EntityHandle tombstone = scene.registry.tombstone();
	s_old_to_new[tombstone.id] = tombstone;

	ar(
		cereal::make_nvp("registry", scene.registry),
		cereal::make_nvp("root", scene.root)
	);

	scene.root = s_old_to_new.at(scene.root.id);
}

};	// end of namespace


namespace APE {

struct Serialize {
	static void saveScene(
		std::filesystem::path save_path,
		::APE::Scene& world) noexcept
	{
		std::ofstream os(save_path);
		cereal::JSONOutputArchive archive(os);
		archive(world);
	}

	static Scene loadScene(std::filesystem::path load_path)
	{
		std::ifstream is(load_path);
		cereal::JSONInputArchive archive(is);
		
		Scene world;
		archive(world);

		APE_TRACE("New scene entity count: {}",
			world.registry.numEntities()
		);
		return world;
	}
};

};	// end of namespace

