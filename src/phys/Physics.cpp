#include "phys/Physics.h"

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
#include <memory>
#include <sys/types.h>

namespace APE::Phys
{

PhysicsSystem::PhysicsSystem(
	const uint cMaxBodies,
	const uint cNumBodyMutexes,
	const uint cMaxBodyPairs,
	const uint cMaxContactConstraints
) noexcept
{
	// Memory Allocator
	JPH::RegisterDefaultAllocator();

	// Traces and Callbacks
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;);

	// Factory for creating class instances (required)
	JPH::Factory::sInstance = new JPH::Factory();

	// Register physics types with factory
	JPH::RegisterTypes();

	// Temp allocator (default 10 MB)
	temp_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

	// Job system
	job_system = std::make_unique<JPH::JobSystemThreadPool>(
		JPH::cMaxPhysicsJobs,
		JPH::cMaxPhysicsBarriers,
		std::thread::hardware_concurrency() - 1
	);

	// Init physics system
	phys_system.Init(
		cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		broad_phase_layer_interface,
		object_vs_broadphase_layer_filter,
		object_vs_object_layer_filter
	);
}

PhysicsSystem::~PhysicsSystem() noexcept
{
	JPH::BodyInterface& bi = phys_system.GetBodyInterface();
	JPH::BodyIDVector body_ids;
	phys_system.GetBodies(body_ids);
	bi.RemoveBodies(body_ids.data(), body_ids.size());
	bi.DestroyBodies(body_ids.data(), body_ids.size());

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::update(float delta) noexcept
{
	phys_system.Update(delta, 1, temp_allocator.get(), job_system.get());
}

void PhysicsSystem::optimizeBroadPhase() noexcept
{
	phys_system.OptimizeBroadPhase();
}

JPH::AllHitCollisionCollector<JPH::CastRayCollector> PhysicsSystem::castRay(
	glm::vec3 pos,
	glm::vec3 dir
) noexcept
{
	// Create JPH Raycast Query
	auto rpos = JPH::Vec3(pos.x, pos.y, pos.z);
	auto rdir = JPH::Vec3(dir.x, dir.y, dir.z);
	JPH::RRayCast ray { rpos, MAX_RAY_DIST * rdir };

	JPH::RayCastSettings ray_settings;
	ray_settings.SetBackFaceMode(JPH::EBackFaceMode::CollideWithBackFaces);

	JPH::AllHitCollisionCollector<JPH::CastRayCollector> collector;

	// Cast ray
	phys_system.GetNarrowPhaseQuery().CastRay(ray, ray_settings, collector);
	collector.Sort();

	return collector;
}

};	// end of namespace APE::Phys

