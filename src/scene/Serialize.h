#pragma once

#include "components/Object.h"
#include "scene/AssetManager.h"
#include "ecs/Registry.h"
#include "scene/Scene.h"

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

#include <filesystem>
#include <fstream>

namespace cereal {

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
* Entity Handle
*/
template <class Archive>
void serialize(Archive& ar, APE::ECS::EntityHandle& ent)
{
	ar(cereal::make_nvp("id", ent.id));
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
void serialize(Archive& ar, APE::HierarchyComponent& h)
{
	ar(
		cereal::make_nvp("parent", h.parent), 
		cereal::make_nvp("children", h.children),
		cereal::make_nvp("tag", h.tag)
	);
}


/*
* Render Components
*/
// template <class Archive>
// void serialize(Archive& ar, APE::Render::MeshComponent& m)
// {
// 	ar(
// 		cereal::make_nvp("model_handle", m.model_handle), 
// 		cereal::make_nvp("mesh_index", m.mesh_index)
// 	);
// }


/*
* ECS Component Pool
*/
template <class Archive, typename Component>
void trySerializePool(Archive& ar, APE::ECS::Registry& r) noexcept
{
	if (r.hasComponent<Component>()) {
		for (auto [ent, comp] : r.getPool<Component>()) {
			ar(
				cereal::make_nvp("ent", ent),
				cereal::make_nvp("component", comp)
			);
		}
	}
}


/*
* ECS Registry
*/
template <class Archive>
void serialize(Archive& ar, APE::ECS::Registry& r)
{
	trySerializePool<Archive, APE::TransformComponent>(ar, r);
	trySerializePool<Archive, APE::HierarchyComponent>(ar, r);
	// trySerializePool<Archive, APE::Render::MeshComponent>(ar, r);
	// trySerializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	// trySerializePool<Archive, APE::Render::LightComponent>(ar, r);
}


/*
* Scene
*/
template <class Archive>
void serialize(Archive& ar, APE::Scene& scene)
{
	ar(
		cereal::make_nvp("registry", scene.registry), 
		cereal::make_nvp("root", scene.root)
	);
}


/*
* Asset Manager
*/
template <class Archive>
void serialize(Archive& ar, APE::AssetManager& am)
{

}


};	// end of namespace




namespace APE {

struct Serialize {
	static void saveScene(
		std::filesystem::path save_path,
		Scene& world,
		AssetManager& asset_manager)
	{
		std::ofstream os(save_path);
		cereal::JSONOutputArchive archive(os);
		archive(asset_manager, world);
	}
};

};	// end of namespace

