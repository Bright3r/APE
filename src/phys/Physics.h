#pragma once

#include "util/Logger.h"

#include <Jolt/Core/Core.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Jolt.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/SubShapeIDPair.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>

#include <glm/glm.hpp>
#include <array>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <sys/types.h>

namespace APE::Phys
{

static void TraceImpl(const char *in_fmt, ...)
{
	va_list list;
	va_start(list, in_fmt);
	char buf[1024];
	vsnprintf(buf, sizeof(buf), in_fmt, list);
	va_end(list);

	APE_TRACE(buf);
}

#ifdef JPH_ENABLE_ASSERTS
static bool AssertFailedImpl(
	const char *in_expr,
	const char *in_msg,
	const char *in_file,
	uint in_line
)
{
	if (in_expr && in_msg && in_file) APE_WARN("{}: {}: ( {} ) {}", in_file, in_line, in_expr, in_msg);
	else APE_WARN("JPH ASSERTION FAILURE: NO MESSAGE");

	return true;
}
#endif

namespace Layers 
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

struct ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
	virtual bool ShouldCollide(
		JPH::ObjectLayer in_obj1,
		JPH::ObjectLayer in_obj2
	) const override
	{
		switch (in_obj1)
		{
		case Layers::NON_MOVING:
			return in_obj2 == Layers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			return false;
		}
	}
};

namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING { 0 };
	static constexpr JPH::BroadPhaseLayer MOVING { 1 };
	static constexpr size_t NUM_LAYERS { 2 };
};

class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
	std::array<JPH::BroadPhaseLayer, BroadPhaseLayers::NUM_LAYERS> m_obj_to_broad;

public:
	BroadPhaseLayerInterfaceImpl()
	{
		m_obj_to_broad[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		m_obj_to_broad[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer in_layer) const override
	{
		return m_obj_to_broad[in_layer];
	}

	#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer in_layer) const override
	{
		switch (static_cast<JPH::BroadPhaseLayer::Type>(in_layer))
		{
		case (static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::NON_MOVING))
			return "NON_MOVING";
		case (static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayers::MOVING))
			return "MOVING";
		default:
			return "INVALID";
		}
	}
	#endif
};

struct ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
	virtual bool ShouldCollide(
		JPH::ObjectLayer in_layer1,
		JPH::BroadPhaseLayer in_layer2
	) const override
	{
		switch (in_layer1)
		{
		case Layers::NON_MOVING:
			return in_layer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			return false;
		}
	}
};

struct MyContactListener : public JPH::ContactListener
{
	virtual JPH::ValidateResult OnContactValidate(
		const JPH::Body& in_body1,
		const JPH::Body& in_body2,
		JPH::RVec3Arg in_base_offset,
		const JPH::CollideShapeResult& in_collision_res
	) override
	{
		APE_TRACE("Contact validate callback");
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(
		const JPH::Body& in_body1,
		const JPH::Body& in_body2,
		const JPH::ContactManifold& in_manifold1,
		JPH::ContactSettings& io_settings
	) override
	{
		APE_TRACE("A contact was added");
	}

	virtual void OnContactPersisted(
		const JPH::Body& in_body1,
		const JPH::Body& in_body2,
		const JPH::ContactManifold& in_manifold1,
		JPH::ContactSettings& io_settings
	) override
	{
		APE_TRACE("A contact was persisted");
	}

	virtual void OnContactRemoved(const JPH::SubShapeIDPair& in_sub_shape_pair) override
	{
		APE_TRACE("A contact was removed");
	}
};

struct MyBodyActivationListener : public JPH::BodyActivationListener
{
	virtual void OnBodyActivated(
		const JPH::BodyID& in_body_id,
		JPH::uint64 in_body_user_data
	) override
	{
		APE_TRACE("A body was activated");
	}

	virtual void OnBodyDeactivated(
		const JPH::BodyID& in_body_id,
		JPH::uint64 in_body_user_data
	) override
	{
		APE_TRACE("A body went to sleep");
	}
};

struct PhysicsSystem
{
	JPH::PhysicsSystem phys_system;
	std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
	std::unique_ptr<JPH::JobSystemThreadPool> job_system;

	// Physics system parameters
	BroadPhaseLayerInterfaceImpl broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	static constexpr float MAX_RAY_DIST = 10000.f;

	PhysicsSystem(
		const uint cMaxBodies = 65536,
		const uint cNumBodyMutexes = 0,
		const uint cMaxBodyPairs = 65536,
		const uint cMaxContactConstraints = 10240
	) noexcept;

	// Delete copy ctor
	PhysicsSystem(const PhysicsSystem&) = delete;
	PhysicsSystem& operator=(const PhysicsSystem&) = delete;

	// Delete move ctor
	PhysicsSystem(PhysicsSystem&& other) = delete;
	PhysicsSystem& operator=(PhysicsSystem&& other) = delete;

	~PhysicsSystem() noexcept;

	void update(float delta) noexcept;

	void optimizeBroadPhase() noexcept;

	JPH::AllHitCollisionCollector<JPH::CastRayCollector> castRay(
		glm::vec3 pos,
		glm::vec3 dir
	) noexcept;
};

};	// end of namespace APE::Phys

