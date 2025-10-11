#pragma once

#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/scene/AssetLoader.h"
#include "core/ecs/Registry.h"
#include "core/scene/Scene.h"
#include "physics/Integrator.h"
#include "physics/PhysicsWorld.h"
#include "util/Logger.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include <filesystem>
#include <fstream>
#include <unordered_map>


namespace cereal {

using EntityMap = std::unordered_map<APE::ECS::EntityID, APE::ECS::EntityHandle>;

static inline APE::ECS::Registry* s_scene_registry = nullptr;
static inline APE::ECS::Registry* s_phys_registry = nullptr;
static inline std::unordered_map<APE::ECS::Registry*, EntityMap> s_old_to_new {};

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

template <class Archive>
void serialize(Archive& ar, glm::mat3& m)
{
	ar(
		cereal::make_nvp("col0", m[0]), 
		cereal::make_nvp("col1", m[1]),
		cereal::make_nvp("col2", m[2])
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

	h.parent = s_old_to_new.at(s_scene_registry).at(h.parent.id);
	for (size_t i = 0; i < h.children.size(); ++i) {
		h.children[i] = s_old_to_new.at(s_scene_registry).at(h.children[i].id);
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
* Physics Components
*/
template <class Archive>
void save(Archive& ar, const APE::Physics::RigidBodyComponent& rbd)
{
	ar(
		cereal::make_nvp("physics_ent", rbd.physics_ent)
	);
}

template <class Archive>
void load(Archive& ar, APE::Physics::RigidBodyComponent& rbd)
{
	ar(
		cereal::make_nvp("physics_ent", rbd.physics_ent)
	);

	rbd.physics_ent = s_old_to_new.at(s_phys_registry).at(rbd.physics_ent.id);
}

template <class Archive>
void serialize(Archive& ar, APE::Physics::RigidBody& rb)
{
	ar(
		cereal::make_nvp("pos", rb.pos),
		cereal::make_nvp("orientation", rb.orientation),
		cereal::make_nvp("vel_linear", rb.vel_linear),
		cereal::make_nvp("vel_angular", rb.vel_angular),
		cereal::make_nvp("inv_mass", rb.inv_mass),
		cereal::make_nvp("restitution", rb.restitution),
		cereal::make_nvp("moment", rb.moment),
		cereal::make_nvp("forces", rb.forces),
		cereal::make_nvp("torques", rb.torques)
	);
}

template <class Archive>
void serialize(Archive& ar, APE::Physics::Collisions::AABB& aabb)
{
	ar(
		cereal::make_nvp("type", aabb.type),
		cereal::make_nvp("pos", aabb.pos),
		cereal::make_nvp("min", aabb.min),
		cereal::make_nvp("max", aabb.max)
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
	void serialize(Archive& ar) 
	{
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

	ar(cereal::make_nvp(Component::Name, entries));
	APE_TRACE("Serialized {}", Component::Name);
}

template <class Archive, typename Component>
void deserializePool(Archive& ar, APE::ECS::Registry& r) noexcept
{
	std::vector<ECSPair<Component>> entries;
	ar(cereal::make_nvp(Component::Name, entries));

	for (auto& [ent, comp] : entries) {
		APE::ECS::EntityHandle new_ent = s_old_to_new.at(&r).at(ent.id);
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

	serializePool<Archive, APE::Physics::RigidBody>(ar, r);
	serializePool<Archive, APE::Physics::Collisions::AABB>(ar, r);
	serializePool<Archive, APE::Physics::RigidBodyComponent>(ar, r);
}

template <class Archive>
void load(Archive& ar, APE::ECS::Registry& r)
{
	std::vector<APE::ECS::EntityHandle> old_ents;
	ar(cereal::make_nvp("entities", old_ents));

	for (auto old_ent : old_ents) {
		s_old_to_new[&r][old_ent.id] = r.createEntity();
	}

	deserializePool<Archive, APE::TransformComponent>(ar, r);
	deserializePool<Archive, APE::HierarchyComponent>(ar, r);
	deserializePool<Archive, APE::Render::MeshComponent>(ar, r);
	deserializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	deserializePool<Archive, APE::Render::LightComponent>(ar, r);

	deserializePool<Archive, APE::Physics::RigidBody>(ar, r);
	deserializePool<Archive, APE::Physics::Collisions::AABB>(ar, r);
	deserializePool<Archive, APE::Physics::RigidBodyComponent>(ar, r);
}



/*
* Physics World
*/
template <class Archive>
void save(Archive& ar, const APE::Physics::PhysicsWorld& phys_world)
{
	// TODO - serialize integrator type
	ar(
		cereal::make_nvp("registry", phys_world.world)
	);
}

template <class Archive>
void load(Archive& ar, APE::Physics::PhysicsWorld& phys_world)
{	
	phys_world.integrator = std::make_unique<APE::Physics::EulerIntegrator>();

	APE::ECS::EntityHandle tombstone = phys_world.world.tombstone();
	s_old_to_new[&phys_world.world][tombstone.id] = tombstone;
	s_phys_registry = &phys_world.world;

	ar(
		cereal::make_nvp("registry", phys_world.world)
	);
}



/*
* Scene
*/
template <class Archive>
void save(Archive& ar, const APE::Scene& scene)
{
	ar(
		cereal::make_nvp("physics_world", scene.phys_world),
		cereal::make_nvp("registry", scene.registry), 
		cereal::make_nvp("root", scene.root)
	);
}

template <class Archive>
void load(Archive& ar, APE::Scene& scene)
{
	s_old_to_new.clear();

	APE::ECS::EntityHandle tombstone = scene.registry.tombstone();
	s_old_to_new[&scene.registry][tombstone.id] = tombstone;
	s_scene_registry = &scene.registry;

	ar(
		cereal::make_nvp("physics_world", scene.phys_world),
		cereal::make_nvp("registry", scene.registry),
		cereal::make_nvp("root", scene.root)
	);

	scene.root = s_old_to_new.at(&scene.registry).at(scene.root.id);
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

