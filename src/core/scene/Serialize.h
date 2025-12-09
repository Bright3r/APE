#pragma once

#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/scene/AssetLoader.h"
#include "core/ecs/Registry.h"
#include "core/scene/Scene.h"
#include "util/Logger.h"

#include "phys/Physics.h"
#include "phys/PlayerController.h"
#include <Jolt/ObjectStream/ObjectStreamTextOut.h>
#include <Jolt/ObjectStream/ObjectStreamTextIn.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/EActivation.h>

#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>


namespace cereal {

using EntityMap = std::unordered_map<APE::ECS::EntityID, APE::ECS::EntityHandle>;
static inline std::unordered_map<APE::ECS::Registry*, EntityMap> s_old_to_new {};

static inline APE::Scene* s_scene {};
static inline const APE::Scene* s_scene_const {};

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

	h.parent = s_old_to_new.at(&s_scene->registry).at(h.parent.id);
	for (size_t i = 0; i < h.children.size(); ++i) {
		h.children[i] = s_old_to_new.at(&s_scene->registry).at(h.children[i].id);
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

template <class Archive>
void serialize(Archive& ar, APE::Render::Camera& cam)
{
	ar(
		cereal::make_nvp("position", cam.m_position),
		cereal::make_nvp("pitch", cam.m_pitch),
		cereal::make_nvp("yaw", cam.m_yaw),
		cereal::make_nvp("fov", cam.m_fov),
		cereal::make_nvp("sensitivity", cam.m_sensitivity),
		cereal::make_nvp("near_plane", cam.m_near_plane),
		cereal::make_nvp("is_locked", cam.m_is_locked)
	);
}

template <class Archive>
void serialize(Archive& ar, APE::Render::CameraComponent& cam)
{
	ar(cereal::make_nvp("camera", cam.camera));
}


/*
* Physics Components
*/
template <class Archive>
void save(Archive& ar, const JPH::Vec3& vec)
{
	ar(cereal::make_nvp("x", vec.GetX()));
	ar(cereal::make_nvp("y", vec.GetY()));
	ar(cereal::make_nvp("z", vec.GetZ()));
}

template <class Archive>
void load(Archive& ar, JPH::Vec3& vec)
{
	float x, y, z;
	ar(cereal::make_nvp("x", x));
	ar(cereal::make_nvp("y", y));
	ar(cereal::make_nvp("z", z));

	vec.Set(x, y, z);
}


template <class Archive>
void save(Archive& ar, const JPH::Quat& quat)
{
	ar(cereal::make_nvp("x", quat.GetX()));
	ar(cereal::make_nvp("y", quat.GetY()));
	ar(cereal::make_nvp("z", quat.GetZ()));
	ar(cereal::make_nvp("w", quat.GetW()));
}

template <class Archive>
void load(Archive& ar, JPH::Quat& quat)
{
	float x, y, z, w;
	ar(cereal::make_nvp("x", x));
	ar(cereal::make_nvp("y", y));
	ar(cereal::make_nvp("z", z));
	ar(cereal::make_nvp("w", w));

	quat.Set(x, y, z, w);
}


std::string getMotionTypeString(const JPH::EMotionType& motion_type) noexcept
{
	std::string motion_str {};
	switch (motion_type)
	{
	case JPH::EMotionType::Dynamic:
		motion_str = "Dynamic";
		break;
	case JPH::EMotionType::Kinematic:
		motion_str = "Kinematic";
		break;
	case JPH::EMotionType::Static:
		motion_str = "Static";
		break;
	}

	return motion_str;
}

JPH::EMotionType getMotionType(const std::string& motion_str) noexcept
{
	JPH::EMotionType motion_type;
	if (motion_str == "Dynamic")
	{
		motion_type = JPH::EMotionType::Dynamic;
	}
	else if (motion_str == "Kinematic")
	{
		motion_type = JPH::EMotionType::Kinematic;
	}
	else if (motion_str == "Static")
	{
		motion_type = JPH::EMotionType::Static;
	}

	return motion_type;
}


std::string getObjectLayerString(const JPH::ObjectLayer& layer) noexcept
{
	std::string layer_str {};
	switch (layer)
	{
	case APE::Phys::Layers::MOVING:
		layer_str = "Moving";
		break;
	case APE::Phys::Layers::NON_MOVING:
		layer_str = "Non_Moving";
		break;
	}

	return layer_str;
}

JPH::ObjectLayer getObjectLayer(const std::string& layer_str) noexcept
{
	JPH::ObjectLayer layer;
	if (layer_str == "Moving")
	{
		layer = APE::Phys::Layers::MOVING;
	}
	else if (layer_str == "Non_Moving")
	{
		layer = APE::Phys::Layers::NON_MOVING;
	}

	return layer;
}


std::string getBoxShapeString(const JPH::BoxShape* box) noexcept
{
	std::stringstream data;

	auto extents = box->GetHalfExtent();
	data << extents.GetX() << ",";
	data << extents.GetY() << ",";
	data << extents.GetZ();

	return data.str();
}

JPH::BoxShape* getBoxShape(const std::string& info) noexcept
{
	std::stringstream data(info);

	float x, y, z;
	char trash;
	data >> x >> trash;
	data >> y >> trash;
	data >> z;

	APE_TRACE("Box Shape: ({},{},{})", x, y, z);
	return new JPH::BoxShape(JPH::Vec3(x, y, z));
}


std::string getSphereShapeString(const JPH::SphereShape* sphere) noexcept
{
	std::stringstream data;

	auto radius = sphere->GetRadius();
	data << radius;

	APE_TRACE("Sphere Shape: radius={}", radius);
	return data.str();
}

JPH::SphereShape* getSphereShape(const std::string& info) noexcept
{
	std::stringstream data(info);

	float radius;
	data >> radius;

	return new JPH::SphereShape(radius);
}


std::string getShapeTypeString(const JPH::Shape* shape) noexcept
{
	std::string shape_type = "Undefined";

	switch (shape->GetSubType())
	{
	case JPH::EShapeSubType::Box:
		shape_type = "Box";
		break;
	case JPH::EShapeSubType::Sphere:
		shape_type = "Sphere";
		break;
	default:
		shape_type = "Empty";
		APE_ABORT("Cannot save unsupported physics shape.");
	}

	return shape_type;
}

JPH::EShapeSubType getShapeType(const std::string& shape_type_str) noexcept
{
	if (shape_type_str == "Box") return JPH::EShapeSubType::Box;
	if (shape_type_str == "Sphere") return JPH::EShapeSubType::Sphere;

	return JPH::EShapeSubType::Empty;
}

std::string getShapeString(const JPH::Shape* shape) noexcept
{
	std::string shape_info;

	switch (shape->GetSubType())
	{
	case JPH::EShapeSubType::Box:
		shape_info = getBoxShapeString(reinterpret_cast<const JPH::BoxShape*>(shape));
		break;
	case JPH::EShapeSubType::Sphere:
		shape_info = getSphereShapeString(reinterpret_cast<const JPH::SphereShape*>(shape));
		break;
	default:
		APE_ABORT("Cannot save unsupported physics shape.");
	}

	return shape_info;
}

JPH::Shape* getShape(JPH::EShapeSubType shape_type, const std::string& info) noexcept
{
	switch (shape_type)
	{
	case JPH::EShapeSubType::Box:
		return getBoxShape(info);
	case JPH::EShapeSubType::Sphere:
		return getSphereShape(info);
	default:
		APE_ABORT("Cannot load unsupported physics shape.");
	}
}



template <class Archive>
void save(Archive& ar, const APE::Phys::PhysicsComponent& phys_comp)
{
	auto body_id = phys_comp.body_id;
	auto& body_if = s_scene_const->phys_system.phys_system.GetBodyInterface();
	auto shape = body_if.GetShape(body_id);

	auto shape_type_str = getShapeTypeString(shape);
	ar(cereal::make_nvp("shape_type", shape_type_str));

	auto shape_info = getShapeString(shape);
	ar(cereal::make_nvp("shape_info", shape_info));

	ar(cereal::make_nvp("position", body_if.GetPosition(body_id)));

	ar(cereal::make_nvp("rotation", body_if.GetRotation(body_id)));

	auto motion_type = body_if.GetMotionType(body_id);
	auto motion_type_str = getMotionTypeString(motion_type);
	ar(cereal::make_nvp("motion_type", motion_type_str));

	auto layer = body_if.GetObjectLayer(body_id);
	auto layer_str = getObjectLayerString(layer);
	ar(cereal::make_nvp("layer", layer_str));
}

template <class Archive>
void load(Archive& ar, APE::Phys::PhysicsComponent& phys_comp)
{
	std::string shape_type_str;
	ar(cereal::make_nvp("shape_type", shape_type_str));
	auto shape_type = getShapeType(shape_type_str);

	std::string shape_info_str;
	ar(cereal::make_nvp("shape_info", shape_info_str));
	auto shape = getShape(shape_type, shape_info_str);

	JPH::Vec3 position;
	ar(cereal::make_nvp("position", position));

	JPH::Quat rotation;
	ar(cereal::make_nvp("rotation", rotation));

	std::string motion_type_str;
	ar(cereal::make_nvp("motion_type", motion_type_str));
	auto motion_type = getMotionType(motion_type_str);

	std::string layer_str;
	ar(cereal::make_nvp("layer", layer_str));
	auto layer = getObjectLayer(layer_str);

	// Add body to physics system
	JPH::BodyCreationSettings settings(
		shape,
		position,
		rotation,
		motion_type,
		layer
	);
	auto body_id = s_scene->createPhysicsBody(settings);
	phys_comp.body_id = body_id;
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

	serializePool<Archive, APE::HierarchyComponent>(ar, r);
	serializePool<Archive, APE::TransformComponent>(ar, r);
	serializePool<Archive, APE::Render::MeshComponent>(ar, r);
	serializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	serializePool<Archive, APE::Render::LightComponent>(ar, r);

	serializePool<Archive, APE::Render::CameraComponent>(ar, r);
	serializePool<Archive, APE::Phys::PhysicsComponent>(ar, r);
	// serializePool<Archive, APE::Phys::PlayerComponent>(ar, r);
}

template <class Archive>
void load(Archive& ar, APE::ECS::Registry& r)
{
	// Remove default root
	r.destroyEntity(s_scene->root);

	// Update mapping of old entities to new entities
	std::vector<APE::ECS::EntityHandle> old_ents;
	ar(cereal::make_nvp("entities", old_ents));
	for (auto old_ent : old_ents) {
		s_old_to_new[&r][old_ent.id] = r.createEntity();
	}

	deserializePool<Archive, APE::HierarchyComponent>(ar, r);
	deserializePool<Archive, APE::TransformComponent>(ar, r);
	deserializePool<Archive, APE::Render::MeshComponent>(ar, r);
	deserializePool<Archive, APE::Render::MaterialComponent>(ar, r);
	deserializePool<Archive, APE::Render::LightComponent>(ar, r);

	deserializePool<Archive, APE::Render::CameraComponent>(ar, r);
	deserializePool<Archive, APE::Phys::PhysicsComponent>(ar, r);
	// deserializePool<Archive, APE::Phys::PlayerComponent>(ar, r);
}


/*
* Scene
*/
template <class Archive>
void save(Archive& ar, const APE::Scene& scene)
{
	s_scene_const = &scene;

	ar(
		cereal::make_nvp("registry", scene.registry), 
		cereal::make_nvp("root", scene.root)
	);
}

template <class Archive>
void load(Archive& ar, APE::Scene& scene)
{
	s_scene = &scene;
	s_scene_const = &scene;

	// map old entities to new ones
	s_old_to_new.clear();
	APE::ECS::EntityHandle tombstone = scene.registry.tombstone();
	s_old_to_new[&scene.registry][tombstone.id] = tombstone;

	ar(
		cereal::make_nvp("registry", scene.registry), 
		cereal::make_nvp("root", scene.root)
	);

	scene.root = s_old_to_new.at(&scene.registry).at(scene.root.id);

	// Update mapping of physics bodies to entities
	auto view = scene.registry.view<APE::Phys::PhysicsComponent>();
	for (auto& [ent, phys_comp] : view.each())
	{
		auto body_id = phys_comp.body_id;
		scene.pbody_to_ent[body_id] = ent;
	}
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
		return std::move(world);
	}
};

};	// end of namespace

