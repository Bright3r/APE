#pragma once

#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/render/Model.h"
#include "phys/Physics.h"

#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <utility>

namespace APE 
{

struct Scene 
{
	ECS::Registry registry;
	ECS::EntityHandle root;
	std::unique_ptr<Phys::PhysicsSystem> phys_system;
	std::unordered_map<JPH::BodyID, ECS::EntityHandle> pbody_to_ent;

	Scene() noexcept;
	Scene(Scene&& other) noexcept;

	Scene& operator=(Scene&& other) noexcept;

	ECS::EntityHandle createEntity(
		ECS::EntityHandle parent,
		std::string tag = ""
	) noexcept;

	void setParent(ECS::EntityHandle child, ECS::EntityHandle parent) noexcept;

	void setTag(ECS::EntityHandle ent, const std::string& tag) noexcept;

	ECS::EntityHandle getParent(ECS::EntityHandle ent) noexcept;

	std::vector<ECS::EntityHandle> getChildren(ECS::EntityHandle ent) noexcept;

	TransformComponent getWorldTransform(ECS::EntityHandle ent) noexcept;

	glm::mat4 getModelMatrix(ECS::EntityHandle ent) noexcept;

	ECS::EntityHandle addModel(
		AssetHandle<Render::Model> model_handle,
		const TransformComponent& transform = {}
	) noexcept;

	JPH::BodyID createPhysicsBody(JPH::BodyCreationSettings body_settings) noexcept;

	JPH::BodyID createAndRegisterPhysicsBody(
		ECS::EntityHandle ent,
		JPH::BodyCreationSettings body_settings
	) noexcept;

	void removePhysicsBody(ECS::EntityHandle ent) noexcept;

	ECS::EntityHandle getPhysicsBodyEntity(JPH::BodyID body_id) const noexcept;

	ECS::EntityHandle addPlayer(
		AssetHandle<Render::Model> model_handle,
		const TransformComponent& transform,
		JPH::Shape *shape
	) noexcept;

	std::pair<ECS::EntityHandle, Phys::PlayerComponent*> getPlayer() noexcept;

	std::pair<ECS::EntityHandle, Render::CameraComponent*> getCamera(
		ECS::EntityHandle parent
	) noexcept;

	void stepPhysics(float dt) noexcept;

	void stepPlayer(float dt) noexcept;

	void syncWithPhysicsState() noexcept;

	void syncPlayerCamera() noexcept;
};

};	// end of namespace

