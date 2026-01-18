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

namespace APE::Editor 
{

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
	
	// auto air_fryer_handle = ModelLoader::load("res/models/ultimate-interior/Air Fryer.glb");
	// auto air_fryer = Engine::world().addModel(air_fryer_handle);

	auto sponza_handle = ModelLoader::load("res/models/main_sponza/NewSponza_Main_glTF_003.gltf");
	auto sponza = Engine::world().addModel(sponza_handle);
	
	std::vector<AssetHandle<Render::Model>> models;
	models.push_back(ModelLoader::load(CUBE_PATH));
	// models.push_back(ModelLoader::load(SPHERE_PATH));
	// models.push_back(ModelLoader::load(CONE_PATH));
	// models.push_back(ModelLoader::load(CYLINDER_PATH));

	auto& world = Engine::world();
	auto& phys_system = world.phys_system;
	auto& body_if = phys_system->phys_system.GetBodyInterface();

	// Add boxes
	constexpr int NUM_SHAPES = 10;
	int sqrt = std::sqrt(NUM_SHAPES);
	if (!models.empty()) 
	{
		for (int i = 0; i < NUM_SHAPES; ++i) 
		{
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
			world.createAndRegisterPhysicsBody(ent, box_settings);
		}
	}


	// Add light
	auto box_handle = ModelLoader::load(CUBE_PATH);

	TransformComponent light_transform {};
	light_transform.position.y = 10;

	Render::LightComponent light {};

	auto light_ent = world.addModel(box_handle, light_transform);
	world.setTag(light_ent, "Light 0");
	world.registry.emplaceComponent<Render::LightComponent>(light_ent, light);

	auto light_ent2 = world.addModel(box_handle, light_transform);
	world.setTag(light_ent2, "Light 1");
	world.registry.emplaceComponent<Render::LightComponent>(light_ent2, light);



	// Add floor
	TransformComponent box_transform {};
	box_transform.position.y = -10.f;
	box_transform.scale = glm::vec3(20.f, 0.1f, 20.f);

	auto floor_ent = world.addModel(box_handle, box_transform);
	
	JPH::BodyCreationSettings floor_settings(
		new JPH::BoxShape(JPH::Vec3(box_transform.scale.x / 2.f, box_transform.scale.y / 2.f, box_transform.scale.z / 2.f)), 
		JPH::RVec3(box_transform.position.x, box_transform.position.y, box_transform.position.z),
		JPH::Quat(box_transform.rotation.x, box_transform.rotation.y, box_transform.rotation.z, box_transform.rotation.w),
		JPH::EMotionType::Static,
		Phys::Layers::NON_MOVING
	);
	world.createAndRegisterPhysicsBody(floor_ent, floor_settings);


	// Add Player
	// 
	TransformComponent player_transform {};
	player_transform.position.y = 0.f;
	player_transform.scale = glm::vec3(1.f, 2.f, 1.f);

	auto player_shape = new JPH::BoxShape(
		JPH::Vec3(
			player_transform.scale.x / 2.f,
			player_transform.scale.y / 2.f,
			player_transform.scale.z / 2.f
		)
	);
	auto player_ent = world.addPlayer(
		box_handle,
		player_transform, 
		player_shape
	);

	// Optimize collision checks
	phys_system->optimizeBroadPhase();


	// FPS Camera
	auto camera_ent = world.createEntity(player_ent, "Player Camera");
	auto fps_cam = std::make_shared<Render::Camera>(player_transform.position);
	fps_cam->setFOV(103.f);
	Render::CameraComponent camera_comp(fps_cam);
	world.registry.emplaceComponent<Render::CameraComponent>(camera_ent, camera_comp);
	world.registry.emplaceComponent<TransformComponent>(
		camera_ent,
		glm::vec3(-0.4f, 0.4f, 0.f)
	);

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
	Engine::setFramerate(1000);

	selected_ent = world.root;
}

void EditorLayer::update() noexcept
{
	std::string fps = std::to_string(1.0 / Engine::getLastFrameTimeSec().count());
	std::string window_title = "FPS: " + fps;
	Engine::setWindowTitle(window_title);
	
	// Quit
	auto& input = Engine::input();
	if (input.isKeyDown(SDLK_Q)) 
	{
		Engine::setQuit(true);
	}

	// Save
	if (input.isKeyDown(SDLK_Y) && input.isFirstFramePressed(SDLK_Y)) 
	{
		Engine::saveScene("demos/test.json", Engine::world());
		APE_TRACE("SAVED SCENE");
	}
	// Load
	if (input.isKeyDown(SDLK_U) && input.isFirstFramePressed(SDLK_U)) 
	{
		Scene world;
		Engine::loadScene("demos/test.json", world);

		Engine::world() = std::move(world);
		APE_TRACE("LOADED SCENE");
	}

	// Camera Movement
	float speed = 10.f;
	float dt = Engine::getLastFrameTimeSec().count();
	auto cam = Engine::getCamera().lock();
	if (input.isKeyDown(SDLK_SPACE)) 
	{
		cam->moveUp(speed, dt);
	}
	if (input.isKeyDown(SDLK_LCTRL)) 
	{
		cam->moveDown(speed, dt);
	}
	if (input.isKeyDown(SDLK_A)) 
	{
		cam->moveLeft(speed, dt);
	}
	if (input.isKeyDown(SDLK_D)) 
	{
		cam->moveRight(speed, dt);
	}
	if (input.isKeyDown(SDLK_W)) 
	{
		cam->moveForward(speed, dt);
	}
	if (input.isKeyDown(SDLK_S)) 
	{
		cam->moveBackward(speed, dt);
	}

	// Camera Tab In
	if (input.isKeyDown(SDLK_C) && input.isFirstFramePressed(SDLK_C)) 
	{
		bool is_locked = cam->isLocked();
		cam->setLocked(!is_locked);
		Engine::setTabIn(is_locked);
	}

	// Mouse button events
	for (auto& m_event : Engine::input().mouseButtonEvents()) 
	{
		handleMouseButtonEvent(m_event);
	}

	// Mouse motion events
	for (auto& m_event : Engine::input().mouseMotionEvents()) 
	{
		cam->rotate(m_event.xrel, m_event.yrel);
	}


	// Switch Camera
	auto [player_ent, player] = Engine::world().getPlayer();
	auto [cam_ent, player_cam_comp] = Engine::world().getCamera(player_ent);
	auto player_cam = player_cam_comp->camera;
	if (input.isKeyDown(SDLK_T) && input.isFirstFramePressed(SDLK_T)) 
	{
		std::shared_ptr<Render::Camera> curr_cam = fly_cam;
		if (cam == fly_cam)
		{
			curr_cam = player_cam;
		}
		Engine::setCamera(curr_cam);
	}


	// Pause Simulation
	if (input.isKeyDown(SDLK_P) && input.isFirstFramePressed(SDLK_P))
	{
		b_play_simulation = !b_play_simulation;
	}



	// TEMPORARY - UPDATE PHYSICS
	if (!b_play_simulation) return;

	glm::vec3 player_dir(0.f);
	if (Engine::getCamera().lock() == player_cam)
	{
		auto forward = player_cam->getForwardVector();
		forward.y = 0;
		forward = glm::normalize(forward);

		auto right = player_cam->getRightVector();
		right.y = 0;
		right = glm::normalize(right);

		if (input.isKeyDown(SDLK_W)) 
		{
			player_dir += forward;
		}
		if (input.isKeyDown(SDLK_S)) 
		{
			player_dir -= forward;
		}
		if (input.isKeyDown(SDLK_A)) 
		{
			player_dir -= right;
		}
		if (input.isKeyDown(SDLK_D)) 
		{
			player_dir += right;
		}

		if (input.isKeyDown(SDLK_SPACE)) 
		{
			if (player->controller.body->IsSupported())
			{
				player->controller.jump(5.f);
			}
		}

		player->controller.move(player_dir, 5.f);
	}

	auto& world = Engine::world();
	world.stepPhysics(dt);
	world.stepPlayer(dt);

	// Sync transforms with physics state
	world.syncWithPhysicsState();
	world.syncPlayerCamera();
}

void EditorLayer::draw() noexcept
{
	auto renderer = Engine::renderer();
	auto cam = Engine::getCamera().lock();
	std::array<Uint8, 4> green = { 0, 255, 0, 255 };

	auto& world = Engine::world();
	for (auto& [ent, light] : world.registry.view<Render::LightComponent>().each())
	{
		auto transform = world.getWorldTransform(ent);
		auto p1 = transform.position;
		auto p2 = p1 + transform.getLookatDirection();
		renderer->drawLine(p1, p2, green, cam.get());
	}

	if (b_show_hitboxes)
	{
		auto& world = Engine::world();
		auto view = world.registry.view<Phys::PhysicsComponent>();
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
	
	auto cam = Engine::getCamera().lock();
	auto wp = cam->screenToWorld(screen_coords, *Engine::context());
	auto pos = cam->getPosition();
	auto dir = glm::normalize(wp - pos);
	
	auto& phys_system = Engine::world().phys_system;
	auto collector = phys_system->castRay(pos, dir);
	if (collector.HadHit())
	{
		APE_TRACE("RAY HIT");
		auto& hit = collector.mHits[0];
		
		auto body_id = hit.mBodyID;
		auto& world = Engine::world();
		if (!b_lock_selection) selected_ent = world.getPhysicsBodyEntity(body_id);
	}
}

void EditorLayer::drawAABB(JPH::BodyID body_id) noexcept
{
	auto& world = Engine::world();
	auto& lock_if = world.phys_system->phys_system.GetBodyLockInterface();
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
			auto cam = Engine::getCamera().lock().get();
			auto renderer = Engine::renderer();
			renderer->drawLine(tfl, tfr, green, cam);
			renderer->drawLine(tfl, tbl, green, cam);
			renderer->drawLine(tfl, bfl, green, cam);
			renderer->drawLine(bbl, bfl, green, cam);
			renderer->drawLine(bbl, tbl, green, cam);
			renderer->drawLine(bbl, bbr, green, cam);
			renderer->drawLine(bfr, bbr, green, cam);
			renderer->drawLine(bfr, tfr, green, cam);
			renderer->drawLine(bfr, bfl, green, cam);
			renderer->drawLine(tbr, tfr, green, cam);
			renderer->drawLine(tbr, bbr, green, cam);
			renderer->drawLine(tbr, tbl, green, cam);
		}
	}
}

};	// end of namespace

