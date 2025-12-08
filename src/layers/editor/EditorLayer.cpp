#include "layers/editor/EditorLayer.h"
#include "core/components/Render.h"
#include "layers/editor/UI.h"
#include "core/components/Physics.h"
#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Camera.h"
#include "core/render/Model.h"
#include "core/scene/ModelLoader.h"
#include "phys/PlayerController.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/BackFaceMode.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Geometry/AABox.h>

#include <glm/glm.hpp>
#include <imgui.h>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

namespace APE::Editor {

void EditorLayer::setup() noexcept
{
	Engine::setWindowTitle("APE Engine");;

	static constexpr std::string_view CAR_PATH = "res/models/che/scene.gltf";
	static constexpr std::string_view CUBE_PATH = "res/models/cube.obj";
	static constexpr std::string_view SPHERE_PATH = "res/models/sphere.obj";
	static constexpr std::string_view CONE_PATH = "res/models/cone.obj";
	static constexpr std::string_view CYLINDER_PATH = "res/models/cylinder.obj";

	// auto car_model_handle = ModelLoader::load(CAR_PATH);
	// auto car = Engine::world().addModel(car_model_handle);
	
	std::vector<AssetHandle<Render::Model>> models;
	models.push_back(ModelLoader::load(CUBE_PATH));
	// models.push_back(ModelLoader::load(SPHERE_PATH));
	// models.push_back(ModelLoader::load(CONE_PATH));
	// models.push_back(ModelLoader::load(CYLINDER_PATH));

	auto& world = Engine::world();
	auto& phys_system = world.phys_system;
	auto& body_if = phys_system.phys_system.GetBodyInterface();

	// Add boxes
	constexpr int NUM_SHAPES = 10;
	int sqrt = std::sqrt(NUM_SHAPES);
	if (!models.empty()) {
		for (int i = 0; i < NUM_SHAPES; ++i) {
			auto& model_handle = models[i % models.size()];

			int row = i % sqrt;
			int col = i / sqrt;
			TransformComponent transform {};
			transform.position.x = (row - (sqrt / 2.f)) * 5;
			transform.position.z = (col - (sqrt / 2.f)) * 5;

			// Add entity model
			auto ent = world.addModel(model_handle, transform);

			// Register entity with physics system
			JPH::BodyCreationSettings box_settings(
				new JPH::BoxShape(JPH::Vec3(0.5f, 0.5f, 0.5f)), 
				JPH::RVec3(transform.position.x, transform.position.y, transform.position.z),
				JPH::Quat(transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w),
				JPH::EMotionType::Dynamic,
				Phys::Layers::MOVING
			);
			JPH::BodyID box_id = body_if.CreateAndAddBody(
				box_settings,
				JPH::EActivation::Activate
			);

			// Maintain handle to physics body in ECS
			world.registerPhysicsBody(ent, box_id);
		}
	}

	// Add floor
	auto box_handle = ModelLoader::load(CUBE_PATH);

	TransformComponent box_transform {};
	box_transform.position.y = -10.f;
	box_transform.scale = glm::vec3(20.f, 0.1f, 20.f);

	auto ent = world.addModel(box_handle, box_transform);
	
	JPH::BodyCreationSettings floor_settings(
		new JPH::BoxShape(JPH::Vec3(box_transform.scale.x / 2.f, box_transform.scale.y / 2.f, box_transform.scale.z / 2.f)), 
		JPH::RVec3(box_transform.position.x, box_transform.position.y, box_transform.position.z),
		JPH::Quat(box_transform.rotation.x, box_transform.rotation.y, box_transform.rotation.z, box_transform.rotation.w),
		JPH::EMotionType::Static,
		Phys::Layers::NON_MOVING
	);
	JPH::BodyID floor_id = body_if.CreateAndAddBody(
		floor_settings,
		JPH::EActivation::Activate
	);

	world.registerPhysicsBody(ent, floor_id);


	// Add Player
	// 
	// Transform
	TransformComponent player_transform {};
	player_transform.position.y = 0.f;
	player_transform.scale = glm::vec3(1.f, 2.f, 1.f);

	// Model
	auto player_ent = world.addModel(box_handle, player_transform);
	world.setTag(player_ent, "Player");
	
	// Controller
	JPH::CharacterVirtualSettings settings;
	settings.mShape = new JPH::BoxShape(JPH::Vec3(player_transform.scale.x / 2.f, player_transform.scale.y / 2.f, player_transform.scale.z / 2.f));
	settings.mInnerBodyLayer = Phys::Layers::MOVING;

	Phys::PlayerComponent player_comp { 
		std::make_unique<Phys::PlayerController>(settings, player_transform, phys_system)
	};
	world.registry.emplaceComponent<Phys::PlayerComponent>(player_ent, std::move(player_comp));

	// FPS Camera
	auto camera_ent = world.createEntity(player_ent, "Player Camera");
	auto fps_cam = std::make_shared<Render::Camera>(player_transform.position);
	Render::CameraComponent camera_comp(fps_cam);
	world.registry.emplaceComponent<Render::CameraComponent>(camera_ent, camera_comp);
	world.registry.emplaceComponent<TransformComponent>(camera_ent, glm::vec3(-0.4f, 0.4f, 0.f));


	// Optimize collision checks
	phys_system.optimizeBroadPhase();


	// Create fly cam
	fly_cam = std::make_shared<Render::Camera>(
		glm::vec3(26.5, 13.5, -2),
		-22.f,
		-180.f,
		45.f,
		0.3f
	);
	Engine::setCamera(fly_cam);
	Engine::setTabIn(true);

	Engine::setFramerate(60);

	selected_ent = world.root;
}

void EditorLayer::update() noexcept
{
	std::string fps = std::to_string(1000.0 / Engine::getLastFrameTimeMS().count());
	std::string window_title = "FPS: " + fps;
	Engine::setWindowTitle(window_title);
	
	// Quit
	auto& input = Engine::input();
	if (input.isKeyDown(SDLK_Q)) {
		Engine::setQuit(true);
	}

	// // Save
	// if (input.isKeyDown(SDLK_P) && input.isFirstFramePressed(SDLK_P)) {
	// 	Engine::saveScene("demos/test.json", Engine::world());
	// }
	// // Load
	// if (input.isKeyDown(SDLK_L) && input.isFirstFramePressed(SDLK_L)) {
	// 	Engine::loadScene("demos/test.json", Engine::world());
	// }

	// Camera Movement
	float speed = 10.f;
	float dt = Engine::getLastFrameTimeSec().count();
	if (input.isKeyDown(SDLK_SPACE)) {
		Engine::getCamera().lock()->moveUp(speed, dt);
	}
	if (input.isKeyDown(SDLK_LCTRL)) {
		Engine::getCamera().lock()->moveDown(speed, dt);
	}
	if (input.isKeyDown(SDLK_A)) {
		Engine::getCamera().lock()->moveLeft(speed, dt);
	}
	if (input.isKeyDown(SDLK_D)) {
		Engine::getCamera().lock()->moveRight(speed, dt);
	}
	if (input.isKeyDown(SDLK_W)) {
		Engine::getCamera().lock()->moveForward(speed, dt);
	}
	if (input.isKeyDown(SDLK_S)) {
		Engine::getCamera().lock()->moveBackward(speed, dt);
	}
	// Camera Tab In
	if (input.isKeyDown(SDLK_C) && input.isFirstFramePressed(SDLK_C)) {
		bool is_locked = Engine::getCamera().lock()->isLocked();
		Engine::getCamera().lock()->setLocked(!is_locked);
		Engine::setTabIn(is_locked);
	}

	if (input.isKeyDown(SDLK_P) && input.isFirstFramePressed(SDLK_P))
	{
		b_play_simulation = !b_play_simulation;
	}

	// Mouse button events
	for (auto& m_event : Engine::input().mouseButtonEvents()) {
		handleMouseButtonEvent(m_event);
	}

	// Mouse motion events
	for (auto& m_event : Engine::input().mouseMotionEvents()) {
		Engine::getCamera().lock()->rotate(m_event.xrel, m_event.yrel);
	}


	// Switch Camera
	// 
	auto pview = Engine::world().registry.view<Phys::PlayerComponent>().each();
	auto& [player_ent, player] = pview.at(0);
	
	// Get player camera
	ECS::EntityHandle cam_ent {};
	auto player_children = Engine::world().getChildren(player_ent);
	for (auto& child : player_children)
	{
		if (Engine::world().registry.hasComponent<Render::CameraComponent>(child))
		{
			cam_ent = child;
			break;
		}
	}

	auto cam_comp = Engine::world().registry.getComponent<Render::CameraComponent>(cam_ent);
	auto player_cam = cam_comp.camera;

	


	if (input.isKeyDown(SDLK_T) && input.isFirstFramePressed(SDLK_T)) {
		std::shared_ptr<Render::Camera> cam = fly_cam;
		if (Engine::getCamera().lock() == fly_cam)
		{
			cam = player_cam;
		}
		Engine::setCamera(cam);
	}



	// TEMPORARY - UPDATE PHYSICS
	if (!b_play_simulation) return;

	glm::vec3 player_dir(0.f);
	if (Engine::getCamera().lock() == player_cam)
	{
		if (input.isKeyDown(SDLK_W)) {
			player_dir += player_cam->getForwardVector();
		}
		if (input.isKeyDown(SDLK_S)) {
			player_dir -= player_cam->getForwardVector();
		}
		if (input.isKeyDown(SDLK_A)) {
			player_dir -= player_cam->getRightVector();
		}
		if (input.isKeyDown(SDLK_D)) {
			player_dir += player_cam->getRightVector();
		}

		if (input.isKeyDown(SDLK_SPACE)) {
			if (player.controller->body->IsSupported())
			{
				player.controller->jump(5.f);
			}
		}
	}

	auto player_speed = 5.f;
	glm::vec3 vel(0.f);
	if (glm::length(player_dir) != 0.f) vel = glm::normalize(player_dir) * player_speed;
	player.controller->setHorizontalVelocity(vel);


	auto& phys_system = Engine::world().phys_system;
	phys_system.update(dt);


	// Sync transforms with physics state
	auto& body_if = phys_system.phys_system.GetBodyInterface();
	auto view = Engine::world().registry.view<TransformComponent, Phys::PhysicsComponent>();
	for (auto& [ent, transform, pbody] : view.each())
	{
		auto pos = body_if.GetPosition(pbody.body_id);
		transform.position.x = pos.GetX();
		transform.position.y = pos.GetY();
		transform.position.z = pos.GetZ();

		auto rot = body_if.GetRotation(pbody.body_id);
		transform.rotation.x = rot.GetX();
		transform.rotation.y = rot.GetY();
		transform.rotation.z = rot.GetZ();
		transform.rotation.w = rot.GetW();
	}

	player.controller->update(dt, phys_system);

	auto player_pos = player.controller->body->GetPosition();
	auto player_rot = player.controller->body->GetRotation();

	auto& player_transform = Engine::world().registry.getComponent<TransformComponent>(player_ent);
	player_transform.position.x = player_pos.GetX();
	player_transform.position.y = player_pos.GetY();
	player_transform.position.z = player_pos.GetZ();

	player_transform.rotation.x = player_rot.GetX();
	player_transform.rotation.y = player_rot.GetY();
	player_transform.rotation.z = player_rot.GetZ();
	player_transform.rotation.w = player_rot.GetW();


	// Sync player camera with player position
	auto cam_transform = Engine::world().getWorldTransform(cam_ent);
	APE_TRACE("Camera Pos: ({}, {}, {})", 
		cam_transform.position.x,
		cam_transform.position.y,
		cam_transform.position.z
	);
	player_cam->setPosition(cam_transform.position);
}

void EditorLayer::draw() noexcept
{
	if (b_show_hitboxes)
	{
		auto view = Engine::world().registry.view<Phys::PhysicsComponent>();
		for (auto& [ent, pbody] : view.each())
		{
			drawAABB(pbody.body_id);
		}
	}
}

void EditorLayer::drawGUI() noexcept
{
	auto& world = Engine::world();

	drawDebugPanel(
		world,
		b_lock_selection,
		b_show_hitboxes,
		b_play_simulation,
		mouse_force
	);
	drawSceneHierarchyPanel(world, selected_ent);
	drawManipulatorPanel(world, selected_ent, gizmo_op);
	drawGizmo(world, selected_ent, gizmo_op);
}

void EditorLayer::handleMouseButtonEvent(SDL_MouseButtonEvent m_button) noexcept
{
	if (!m_button.down) return;

	// Calculate raycast
	auto screen_coords = glm::vec2(m_button.x, m_button.y);
	auto pos = Engine::getCamera().lock()->getPosition();
	auto dir = glm::normalize(screenToWorld(screen_coords) - pos);
	
	auto& phys_system = Engine::world().phys_system;
	auto collector = phys_system.castRay(pos, dir);
	if (collector.HadHit())
	{
		APE_TRACE("RAY HIT");
		auto& hit = collector.mHits[0];
		
		auto body_id = hit.mBodyID;
		if (!b_lock_selection) selected_ent = Engine::world().getPhysicsBodyEntity(body_id);
	}
}

void EditorLayer::drawAABB(JPH::BodyID body_id) noexcept
{
	auto& lock_if = Engine::world().phys_system.phys_system.GetBodyLockInterface();
	{
		JPH::BodyLockRead lock(lock_if, body_id);
		if (lock.Succeeded())
		{
			const JPH::Body& body = lock.GetBody();
			auto aabb = body.GetWorldSpaceBounds();

			auto pcenter = aabb.GetCenter();
			auto pextents = aabb.GetExtent();

			auto center = glm::vec3(pcenter.GetX(), pcenter.GetY(), pcenter.GetZ());
			auto extents = glm::vec3(pextents.GetX(), pextents.GetY(), pextents.GetZ());

			// Top Front Left
			glm::vec3 tfl = center + glm::vec3(-extents.x, extents.y, -extents.z);
			// Top Front Right
			glm::vec3 tfr = center + glm::vec3(extents.x, extents.y, -extents.z);
			// Top Back Left
			glm::vec3 tbl = center + glm::vec3(-extents.x, extents.y, extents.z);
			// Top Back Right
			glm::vec3 tbr = center + glm::vec3(extents.x, extents.y, extents.z);
			// Bottom Front Left
			glm::vec3 bfl = center + glm::vec3(-extents.x, -extents.y, -extents.z);
			// Bottom Front Right
			glm::vec3 bfr = center + glm::vec3(extents.x, -extents.y, -extents.z);
			// Bottom Back Left
			glm::vec3 bbl = center + glm::vec3(-extents.x, -extents.y, extents.z);
			// Bottom Back Right
			glm::vec3 bbr = center + glm::vec3(extents.x, -extents.y, extents.z);

			std::array<Uint8, 4> green = { 0, 255, 0, 255 };
			Engine::renderer()->drawLine(tfl, tfr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(tfl, tbl, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(tfl, bfl, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bbl, bfl, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bbl, tbl, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bbl, bbr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bfr, bbr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bfr, tfr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(bfr, bfl, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(tbr, tfr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(tbr, bbr, green, Engine::getCamera().lock().get());
			Engine::renderer()->drawLine(tbr, tbl, green, Engine::getCamera().lock().get());
		}
	}
}

glm::vec3 EditorLayer::screenToWorld(glm::vec2 screen_coords) noexcept
{
	// Screen coords to ndc
	glm::vec3 ndc = {
		(2.f * screen_coords.x) / Engine::context()->window_width - 1.f,
		1.f - (2.f * screen_coords.y) / Engine::context()->window_height,
		1.f
	};

	// Ndc to view space
	glm::mat4 inv_proj = glm::inverse(
		Engine::getCamera().lock()->getProjectionMatrix(Engine::context()->getAspectRatio())
	);

	glm::vec4 clip(ndc, 1.f);
	glm::vec4 eye = inv_proj * clip;
	glm::vec4 view(eye / eye.w);

	// View to world
	glm::mat4 inv_view = glm::inverse(Engine::getCamera().lock()->getViewMatrix());
	glm::vec4 world = inv_view * view;

	return glm::vec3(world);
}

};	// end of namespace

