#pragma once

#include "phys/Physics.h"

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

#include <core/components/Object.h>
#include <memory>

namespace APE::Phys
{

struct PlayerController
{
	// could be unique_ptr but shared_ptr instead for trivial copy
	std::shared_ptr<JPH::CharacterVirtual> body;

	PlayerController() noexcept
		: body(nullptr)
	{

	}

	PlayerController(
		JPH::CharacterVirtualSettings& in_settings,
		const TransformComponent& transform,
		PhysicsSystem& phys_system
	) noexcept
	{
		body = std::make_unique<JPH::CharacterVirtual>(
			&in_settings, 
			JPH::Vec3(transform.position.x, transform.position.y, transform.position.z),
			JPH::Quat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w),
			&phys_system.phys_system
		);
	}

	void update(float dt, PhysicsSystem& phys_system) noexcept
	{
		auto& ps = phys_system.phys_system;

		// Apply Gravity
		auto vel = body->GetLinearVelocity();
		if (!body->IsSupported())
		{
			vel += ps.GetGravity() * dt;
			body->SetLinearVelocity(vel);
		}
		else
		{
			vel.SetY(0.f);
		}

		// Update
		body->Update(
			dt,
			ps.GetGravity(),
			{},
			{},
			JPH::BodyFilter(),
			JPH::ShapeFilter(),
			*phys_system.temp_allocator
		);
	}

	void move(
		const glm::vec3& dir,
		float speed
	) noexcept
	{
		glm::vec3 vel(0.f);
		if (glm::length(dir) != 0.f) 
		{
			vel = glm::normalize(dir) * speed;
		}

		setHorizontalVelocity(vel);
	}

	void setPosition(const glm::vec3& pos) noexcept
	{
		JPH::Vec3 jpos { pos.x, pos.y, pos.z };
		body->SetPosition(jpos);
	}

	void setRotation(const glm::quat& rot) noexcept
	{
		JPH::Quat jrot { rot.x, rot.y, rot.z, rot.w };
		body->SetRotation(jrot);
	}

	void addVelocity(const glm::vec3& vel) noexcept
	{
		auto jvel = body->GetLinearVelocity();
		jvel += JPH::Vec3(vel.x, vel.y, vel.z);
		body->SetLinearVelocity(jvel);
	}

	void setVelocity(const glm::vec3& vel) noexcept
	{
		JPH::Vec3 jvel(vel.x, vel.y, vel.z);
		body->SetLinearVelocity(jvel);
	}

	void setHorizontalVelocity(const glm::vec3& vel) noexcept
	{
		auto jvel = body->GetLinearVelocity();
		jvel.SetX(vel.x);
		jvel.SetZ(vel.z);
		body->SetLinearVelocity(jvel);
	}

	void jump(float strength) noexcept
	{
		auto jvel = body->GetLinearVelocity();
		jvel.SetY(strength);
		body->SetLinearVelocity(jvel);
	}
};

};	// end of namespace

