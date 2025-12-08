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
	std::unique_ptr<JPH::CharacterVirtual> body;

	PlayerController(
		JPH::CharacterVirtualSettings& in_settings,
		TransformComponent& transform,
		PhysicsSystem& phys_system) noexcept
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

	void addVelocity(glm::vec3& vel) noexcept
	{
		auto jvel = body->GetLinearVelocity();
		jvel += JPH::Vec3(vel.x, vel.y, vel.z);
		body->SetLinearVelocity(jvel);
	}

	void setVelocity(glm::vec3& vel) noexcept
	{
		JPH::Vec3 jvel(vel.x, vel.y, vel.z);
		body->SetLinearVelocity(jvel);
	}

	void setHorizontalVelocity(glm::vec3& vel) noexcept
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

