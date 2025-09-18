#include "editor/Editor.h"

#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Camera.h"
#include "core/render/Model.h"
#include "core/scene/ModelLoader.h"
#include "editor/UI.h"
#include "physics/BVH.h"
#include "physics/Colliders.h"

#include <glm/glm.hpp>
#include <imgui.h>

#include <cmath>
#include <limits>
#include <vector>

namespace APE::Editor {

void run() noexcept
{
	auto app = std::make_unique<EditorApplication>();
	Engine::init(std::move(app), "APE Physics Engine", 1200, 800);
	Engine::run();
}

void EditorApplication::setup() noexcept
{
	Engine::setWindowTitle("APE Engine");;

	static constexpr std::string_view CAR_PATH = "res/models/che/scene.gltf";
	static constexpr std::string_view CUBE_PATH = "res/models/cube.obj";
	static constexpr std::string_view SPHERE_PATH = "res/models/sphere.obj";
	static constexpr std::string_view CONE_PATH = "res/models/cone.obj";
	static constexpr std::string_view CYLINDER_PATH = "res/models/cylinder.obj";

	auto car_model_handle = ModelLoader::load(CAR_PATH);
	auto car = world.addModel(car_model_handle);
	rbd = &world.addRigidBody(car, car_model_handle);
	
	auto cube_model_handle = ModelLoader::load(CUBE_PATH);
	auto cube = world.addModel(cube_model_handle);
	rbd = &world.addRigidBody(cube, cube_model_handle);

	std::vector<AssetHandle<Render::Model>> models;
	// models.push_back(ModelLoader::load(CUBE_PATH));
	// models.push_back(ModelLoader::load(SPHERE_PATH));
	// models.push_back(ModelLoader::load(CONE_PATH));
	// models.push_back(ModelLoader::load(CYLINDER_PATH));

	constexpr int NUM_SHAPES = 200;
	int sqrt = std::sqrt(NUM_SHAPES);
	if (!models.empty()) {
		for (int i = 0; i < NUM_SHAPES; ++i) {
			auto& model_handle = models[i % models.size()];

			int row = i % sqrt;
			int col = i / sqrt;
			TransformComponent transform {};
			transform.position.x = (row - (sqrt / 2.f)) * 5;
			transform.position.z = (col - (sqrt / 2.f)) * 5;

			world.addModel(model_handle, transform);
		}
	}


	cam = std::make_shared<Render::Camera>(
		glm::vec3(-2.5f, 8.f, 8.f),
		-45.f,
		-90.f,
		45.f,
		0.3f
	);
	Engine::setCamera(cam);
	Engine::setTabIn(true);

	Engine::setFramerate(60);
}

void EditorApplication::update() noexcept
{
	std::string window_title = "FPS: " + 
		std::to_string(1000.0 / Engine::getLastFrameTimeMS().count());
	Engine::setWindowTitle(window_title);
	
	// Quit
	if (Engine::keyDown(SDLK_Q)) {
		Engine::setQuit(true);
	}

	// Save
	if (Engine::keyDown(SDLK_P)) {
		Engine::saveScene("demos/test.json", world);
	}
	// Load
	if (Engine::keyDown(SDLK_L)) {
		Engine::loadScene("demos/test.json", world);
	}

	// Camera Tab In
	if (Engine::keyDown(SDLK_C)) {
		bool is_locked = cam->isLocked();
		cam->setLocked(!is_locked);
		Engine::setTabIn(is_locked);
	}

	// Camera Movement
	float speed = 10.f;
	float dt = Engine::getLastFrameTimeSec().count();
	if (Engine::keyDown(SDLK_SPACE)) {
		cam->moveUp(speed, dt);
	}
	if (Engine::keyDown(SDLK_LCTRL)) {
		cam->moveDown(speed, dt);
	}
	if (Engine::keyDown(SDLK_A)) {
		cam->moveLeft(speed, dt);
	}
	if (Engine::keyDown(SDLK_D)) {
		cam->moveRight(speed, dt);
	}
	if (Engine::keyDown(SDLK_W)) {
		cam->moveForward(speed, dt);
	}
	if (Engine::keyDown(SDLK_S)) {
		cam->moveBackward(speed, dt);
	}
}

void EditorApplication::draw() noexcept
{
	auto view = world.registry.view<Physics::RigidBodyComponent, TransformComponent>();
	for (auto [ent, rbd, transform] : view.each()) {
		drawBVH(rbd.phys_state.bvh, transform);
	}
}

void EditorApplication::drawGUI() noexcept
{
	drawDebugPanel(world);
	drawSceneHierarchyPanel(world, selected_ent);
	drawManipulatorPanel(world, selected_ent, gizmo_op);
	drawGizmo(world, selected_ent, gizmo_op);
}

void EditorApplication::onKeyDown(SDL_KeyboardEvent key) noexcept
{

}

void EditorApplication::onKeyUp(SDL_KeyboardEvent key) noexcept
{

}

void EditorApplication::onMouseDown(SDL_MouseButtonEvent mButton) noexcept
{
	// Cast ray from camera
	glm::vec2 screen_coords(mButton.x, mButton.y);
	glm::vec3 world_coords = screenToWorld(screen_coords);
	Physics::Collider::Ray ray = {
		.pos = cam->getPosition(),
		.dir = glm::normalize(world_coords - cam->getPosition())
	};

	// Check for collision with scene models
	float t_best = std::numeric_limits<float>::max();
	auto view = world.registry.view<Physics::RigidBodyComponent, TransformComponent>();
	for (auto [ent, rbd, transform] : view.each()) {
		// Transform ray into rbd's model space
		glm::mat4 inv_model_mat = glm::inverse(transform.getModelMatrix());
		Physics::Collider::Ray ray_local = {
			.pos = glm::vec3(inv_model_mat * glm::vec4(ray.pos, 1.f)),
			.dir = glm::normalize(glm::vec3(inv_model_mat * glm::vec4(ray.dir, 0.f)))
		};

		float t;
		Physics::Collider::Triangle tri;
		if (rbd.phys_state.bvh.checkCollision(ray_local, t, tri)) {
			APE_TRACE("HIT");
			if (t < t_best) {
				selected_ent = ent;
			}
		}
	}
}

void EditorApplication::onMouseUp(SDL_MouseButtonEvent mButton) noexcept
{

}

void EditorApplication::onMouseMove(SDL_MouseMotionEvent mEvent) noexcept
{
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

void EditorApplication::drawAABB(
	Physics::Collider::AABB& aabb,
	TransformComponent& transform) noexcept
{
	auto extents = aabb.extents();
	auto center = aabb.center();

	// Top Front Left
	glm::vec3 tfl = center + glm::vec3(-extents.x, extents.y, -extents.z);
	tfl = glm::vec3(transform.getModelMatrix() * glm::vec4(tfl, 1.f));
	// Top Front Right
	glm::vec3 tfr = center + glm::vec3(extents.x, extents.y, -extents.z);
	tfr = glm::vec3(transform.getModelMatrix() * glm::vec4(tfr, 1.f));
	// Top Back Left
	glm::vec3 tbl = center + glm::vec3(-extents.x, extents.y, extents.z);
	tbl = glm::vec3(transform.getModelMatrix() * glm::vec4(tbl, 1.f));
	// Top Back Right
	glm::vec3 tbr = center + glm::vec3(extents.x, extents.y, extents.z);
	tbr = glm::vec3(transform.getModelMatrix() * glm::vec4(tbr, 1.f));
	// Bottom Front Left
	glm::vec3 bfl = center + glm::vec3(-extents.x, -extents.y, -extents.z);
	bfl = glm::vec3(transform.getModelMatrix() * glm::vec4(bfl, 1.f));
	// Bottom Front Right
	glm::vec3 bfr = center + glm::vec3(extents.x, -extents.y, -extents.z);
	bfr = glm::vec3(transform.getModelMatrix() * glm::vec4(bfr, 1.f));
	// Bottom Back Left
	glm::vec3 bbl = center + glm::vec3(-extents.x, -extents.y, extents.z);
	bbl = glm::vec3(transform.getModelMatrix() * glm::vec4(bbl, 1.f));
	// Bottom Back Right
	glm::vec3 bbr = center + glm::vec3(extents.x, -extents.y, extents.z);
	bbr = glm::vec3(transform.getModelMatrix() * glm::vec4(bbr, 1.f));

	std::array<Uint8, 4> green = { 0, 0, 255, 255 };
	Engine::renderer()->drawLine(tfl, tfr, green, cam.get());
	Engine::renderer()->drawLine(tfl, tbl, green, cam.get());
	Engine::renderer()->drawLine(tfl, bfl, green, cam.get());
	Engine::renderer()->drawLine(bbl, bfl, green, cam.get());
	Engine::renderer()->drawLine(bbl, tbl, green, cam.get());
	Engine::renderer()->drawLine(bbl, bbr, green, cam.get());
	Engine::renderer()->drawLine(bfr, bbr, green, cam.get());
	Engine::renderer()->drawLine(bfr, tfr, green, cam.get());
	Engine::renderer()->drawLine(bfr, bfl, green, cam.get());
	Engine::renderer()->drawLine(tbr, tfr, green, cam.get());
	Engine::renderer()->drawLine(tbr, bbr, green, cam.get());
	Engine::renderer()->drawLine(tbr, tbl, green, cam.get());
}

void EditorApplication::drawNode(
	Physics::Collider::BVHNode& node,
	TransformComponent& transform) noexcept
{
	drawAABB(node.aabb, transform);
	for (auto& child : node.children) {
		drawNode(child, transform);
	}
}

void EditorApplication::drawBVH(
	Physics::Collider::BVH& bvh,
	TransformComponent& transform) noexcept
{
	drawNode(bvh.root, transform);
}

glm::vec3 EditorApplication::screenToWorld(glm::vec2 screen_coords) noexcept
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

