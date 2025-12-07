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
	std::unique_ptr<JPH::CharacterVirtual> player_body;

	PlayerController(
		JPH::CharacterVirtualSettings& in_settings,
		TransformComponent& transform,
		PhysicsSystem& phys_system) noexcept
	{
		player_body = std::make_unique<JPH::CharacterVirtual>(
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
		auto vel = player_body->GetLinearVelocity();
		if (!player_body->IsSupported())
		{
			vel += ps.GetGravity() * dt;
			player_body->SetLinearVelocity(vel);
		}
		else
		{
			vel.SetY(0.f);
		}

		// Update
		player_body->Update(
			dt,
			ps.GetGravity(),
			{},
			{},
			JPH::BodyFilter(),
			JPH::ShapeFilter(),
			*phys_system.temp_allocator
		);
	}

	void addVelocity(glm::vec3& vel) noexcept
	{
		auto jvel = player_body->GetLinearVelocity();
		jvel += JPH::Vec3(vel.x, vel.y, vel.z);
		player_body->SetLinearVelocity(jvel);
	}

	void setVelocity(glm::vec3& vel) noexcept
	{
		JPH::Vec3 jvel(vel.x, vel.y, vel.z);
		player_body->SetLinearVelocity(jvel);
	}

	void setHorizontalVelocity(glm::vec3& vel) noexcept
	{
		auto jvel = player_body->GetLinearVelocity();
		jvel.SetX(vel.x);
		jvel.SetZ(vel.z);
		player_body->SetLinearVelocity(jvel);
	}

	void jump(float strength) noexcept
	{
		auto jvel = player_body->GetLinearVelocity();
		jvel.SetY(strength);
		player_body->SetLinearVelocity(jvel);
	}
};

};	// end of namespace

