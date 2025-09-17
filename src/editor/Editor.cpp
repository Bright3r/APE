#include "editor/Editor.h"

#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/scene/AssetHandle.h"
#include "core/render/Camera.h"
#include "core/render/Model.h"
#include "core/scene/ModelLoader.h"
#include "editor/UI.h"

#include <glm/glm.hpp>
#include <imgui.h>

#include <cmath>
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

	// world.addModel(ModelLoader::load(CAR_PATH));
	
	auto cube_model_handle = ModelLoader::load(CUBE_PATH);
	auto cube = world.addModel(cube_model_handle);
	world.addRigidBody(cube, cube_model_handle);

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
	Engine::renderer()->drawLine(
		glm::vec3(0),
		glm::vec3(0, 100, 0),
		{ 255, 0, 0, 255 },
		cam.get()
	);
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

}

void EditorApplication::onMouseUp(SDL_MouseButtonEvent mButton) noexcept
{

}

void EditorApplication::onMouseMove(SDL_MouseMotionEvent mEvent) noexcept
{
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

};	// end of namespace

