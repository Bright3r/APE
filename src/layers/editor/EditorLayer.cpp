#include "layers/editor/EditorLayer.h"
#include "core/components/Physics.h"
#include "layers/editor/UI.h"
#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Camera.h"
#include "core/render/Model.h"
#include "core/scene/ModelLoader.h"

#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/EActivation.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include <cmath>
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

	auto& phys_system = Engine::physics_system();
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
			auto ent = Engine::world().addModel(model_handle, transform);

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
			Engine::world().registry.emplaceComponent<Phys::PhysicsComponent>(ent, box_id);
		}
	}

	// Add floor
	auto box_handle = ModelLoader::load(CUBE_PATH);

	TransformComponent box_transform {};
	box_transform.position.y = -10.f;
	box_transform.scale = glm::vec3(20.f, 1.f, 20.f);

	auto ent = Engine::world().addModel(box_handle, box_transform);
	
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

	Engine::world().registry.emplaceComponent<Phys::PhysicsComponent>(ent, floor_id);


	// Optimize collision checks
	phys_system.optimizeBroadPhase();


	// Create fly cam
	cam = std::make_shared<Render::Camera>(
		glm::vec3(26.5, 13.5, -2),
		-22.f,
		-180.f,
		45.f,
		0.3f
	);
	Engine::setCamera(cam);
	Engine::setTabIn(true);

	Engine::setFramerate(60);
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

	// Save
	if (input.isKeyDown(SDLK_P) && input.isFirstFramePressed(SDLK_P)) {
		Engine::saveScene("demos/test.json", Engine::world());
	}
	// Load
	if (input.isKeyDown(SDLK_L) && input.isFirstFramePressed(SDLK_L)) {
		Engine::loadScene("demos/test.json", Engine::world());
	}

	// Camera Movement
	float speed = 10.f;
	float dt = Engine::getLastFrameTimeSec().count();
	if (input.isKeyDown(SDLK_SPACE)) {
		cam->moveUp(speed, dt);
	}
	if (input.isKeyDown(SDLK_LCTRL)) {
		cam->moveDown(speed, dt);
	}
	if (input.isKeyDown(SDLK_A)) {
		cam->moveLeft(speed, dt);
	}
	if (input.isKeyDown(SDLK_D)) {
		cam->moveRight(speed, dt);
	}
	if (input.isKeyDown(SDLK_W)) {
		cam->moveForward(speed, dt);
	}
	if (input.isKeyDown(SDLK_S)) {
		cam->moveBackward(speed, dt);
	}
	// Camera Tab In
	if (input.isKeyDown(SDLK_C) && input.isFirstFramePressed(SDLK_C)) {
		bool is_locked = cam->isLocked();
		cam->setLocked(!is_locked);
		Engine::setTabIn(is_locked);
	}


	// Mouse button events
	for (auto& m_event : Engine::input().mouseButtonEvents()) {
		handleMouseButtonEvent(m_event);
	}

	// Mouse motion events
	for (auto& m_event : Engine::input().mouseMotionEvents()) {
		cam->rotate(m_event.xrel, m_event.yrel);
	}


	// TEMPORARY - UPDATE PHYSICS
	auto& phys_system = Engine::physics_system();
	phys_system.update(dt);

	// Sync transforms with physics state
	auto& body_if = phys_system.phys_system.GetBodyInterface();
	auto view = Engine::world().registry.view<TransformComponent, Phys::PhysicsComponent>();
	for (auto& [ent, transform, pbody] : view.each())
	{
		// auto pos = body_if.GetCenterOfMassPosition(pbody.body_id);
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
}

void EditorLayer::draw() noexcept
{

}

void EditorLayer::drawGUI() noexcept
{
	auto& world = Engine::world();

	drawDebugPanel(
		world,
		b_lock_selection,
		b_show_hitboxes,
		mouse_force
	);
	drawSceneHierarchyPanel(world, selected_ent);
	drawManipulatorPanel(world, selected_ent, gizmo_op);
	drawGizmo(world, selected_ent, gizmo_op);
}

void EditorLayer::handleMouseButtonEvent(SDL_MouseButtonEvent m_button) noexcept
{

}

// void EditorLayer::drawAABB(
// 	const Physics::Collisions::AABB& aabb,
// 	const TransformComponent& transform) noexcept
// {
// 	auto extents = aabb.extents();
// 	auto center = aabb.center();
//
// 	// Top Front Left
// 	glm::vec3 tfl = center + glm::vec3(-extents.x, extents.y, -extents.z);
// 	tfl = glm::vec3(transform.getModelMatrix() * glm::vec4(tfl, 1.f));
// 	// Top Front Right
// 	glm::vec3 tfr = center + glm::vec3(extents.x, extents.y, -extents.z);
// 	tfr = glm::vec3(transform.getModelMatrix() * glm::vec4(tfr, 1.f));
// 	// Top Back Left
// 	glm::vec3 tbl = center + glm::vec3(-extents.x, extents.y, extents.z);
// 	tbl = glm::vec3(transform.getModelMatrix() * glm::vec4(tbl, 1.f));
// 	// Top Back Right
// 	glm::vec3 tbr = center + glm::vec3(extents.x, extents.y, extents.z);
// 	tbr = glm::vec3(transform.getModelMatrix() * glm::vec4(tbr, 1.f));
// 	// Bottom Front Left
// 	glm::vec3 bfl = center + glm::vec3(-extents.x, -extents.y, -extents.z);
// 	bfl = glm::vec3(transform.getModelMatrix() * glm::vec4(bfl, 1.f));
// 	// Bottom Front Right
// 	glm::vec3 bfr = center + glm::vec3(extents.x, -extents.y, -extents.z);
// 	bfr = glm::vec3(transform.getModelMatrix() * glm::vec4(bfr, 1.f));
// 	// Bottom Back Left
// 	glm::vec3 bbl = center + glm::vec3(-extents.x, -extents.y, extents.z);
// 	bbl = glm::vec3(transform.getModelMatrix() * glm::vec4(bbl, 1.f));
// 	// Bottom Back Right
// 	glm::vec3 bbr = center + glm::vec3(extents.x, -extents.y, extents.z);
// 	bbr = glm::vec3(transform.getModelMatrix() * glm::vec4(bbr, 1.f));
//
// 	std::array<Uint8, 4> green = { 0, 0, 255, 255 };
// 	Engine::renderer()->drawLine(tfl, tfr, green, cam.get());
// 	Engine::renderer()->drawLine(tfl, tbl, green, cam.get());
// 	Engine::renderer()->drawLine(tfl, bfl, green, cam.get());
// 	Engine::renderer()->drawLine(bbl, bfl, green, cam.get());
// 	Engine::renderer()->drawLine(bbl, tbl, green, cam.get());
// 	Engine::renderer()->drawLine(bbl, bbr, green, cam.get());
// 	Engine::renderer()->drawLine(bfr, bbr, green, cam.get());
// 	Engine::renderer()->drawLine(bfr, tfr, green, cam.get());
// 	Engine::renderer()->drawLine(bfr, bfl, green, cam.get());
// 	Engine::renderer()->drawLine(tbr, tfr, green, cam.get());
// 	Engine::renderer()->drawLine(tbr, bbr, green, cam.get());
// 	Engine::renderer()->drawLine(tbr, tbl, green, cam.get());
// }

glm::vec3 EditorLayer::screenToWorld(glm::vec2 screen_coords) noexcept
{
	// Screen coords to ndc
	glm::vec3 ndc = {
		(2.f * screen_coords.x) / Engine::context()->window_width - 1.f,
		1.f - (2.f * screen_coords.y) / Engine::context()->window_height,
		1.f
	};

	// Ndc to view space
	glm::mat4 inv_proj = 
		glm::inverse(cam->getProjectionMatrix(Engine::context()->getAspectRatio()));
	glm::vec4 clip(ndc, 1.f);
	glm::vec4 eye = inv_proj * clip;
	glm::vec4 view(eye / eye.w);

	// View to world
	glm::mat4 inv_view = glm::inverse(cam->getViewMatrix());
	glm::vec4 world = inv_view * view;

	return glm::vec3(world);
}

};	// end of namespace

