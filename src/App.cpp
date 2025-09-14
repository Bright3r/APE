#include "App.h"
#include "components/Object.h"
#include "core/AppRunner.h"
#include "scene/AssetHandle.h"
#include "render/Camera.h"
#include "render/Model.h"
#include "scene/ModelLoader.h"

#include <glm/fwd.hpp>
#include <imgui.h>

#include <cmath>
#include <vector>

void App::setup() 
{
	AppRunner::setWindowTitle("APE Engine");;

	auto& world = AppRunner::getWorld();

	static constexpr std::string_view CAR_PATH = "res/models/che/scene.gltf";
	world.addModel(APE::ModelLoader::load(CAR_PATH));


	static constexpr std::string_view CUBE_PATH = "res/models/cube.obj";
	static constexpr std::string_view SPHERE_PATH = "res/models/sphere.obj";
	static constexpr std::string_view CONE_PATH = "res/models/cone.obj";
	static constexpr std::string_view CYLINDER_PATH = "res/models/cylinder.obj";

	std::vector<APE::AssetHandle<APE::Render::Model>> models;
	models.push_back(APE::ModelLoader::load(CUBE_PATH));
	models.push_back(APE::ModelLoader::load(SPHERE_PATH));
	models.push_back(APE::ModelLoader::load(CONE_PATH));
	models.push_back(APE::ModelLoader::load(CYLINDER_PATH));

	constexpr int NUM_SHAPES = 200;
	int sqrt = std::sqrt(NUM_SHAPES);
	for (int i = 0; i < NUM_SHAPES; ++i) {
		auto& model_handle = models[i % models.size()];

		APE::TransformComponent transform {};
		transform.position.x = ((i % sqrt) - (sqrt / 2.f)) * 5;
		transform.position.z = ((i / sqrt) - (sqrt / 2.f)) * 5;

		world.addModel(model_handle, transform);
	}


	cam = std::make_shared<APE::Render::Camera>(
		glm::vec3(-2.5f, 8.f, 8.f),
		-45.f,
		-90.f,
		45.f,
		0.3f
	);
	AppRunner::setCamera(cam);
	AppRunner::setTabIn(true);

	AppRunner::setFramerate(60);
}

void App::update() 
{
	std::string window_title = "FPS: " + 
		std::to_string(1000.0 / AppRunner::getLastFrameTimeMS().count());
	AppRunner::setWindowTitle(window_title);
	
	// cam->print();
	float speed = 10.f;
	float dt = AppRunner::getLastFrameTimeSec().count();
	
	// Quit
	if (AppRunner::keyDown(SDLK_Q)) {
		AppRunner::setQuit(true);
	}

	// Save
	if (AppRunner::keyDown(SDLK_P)) {
		AppRunner::saveScene("demos/test.json");
	}
	// Load
	if (AppRunner::keyDown(SDLK_L)) {
		AppRunner::loadScene("demos/test.json");
	}

	// Camera Tab In
	if (AppRunner::keyDown(SDLK_C)) {
		bool is_locked = cam->isLocked();
		cam->setLocked(!is_locked);
		AppRunner::setTabIn(is_locked);
	}

	// Camera Movement
	if (AppRunner::keyDown(SDLK_SPACE)) {
		cam->moveUp(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_LCTRL)) {
		cam->moveDown(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_A)) {
		cam->moveLeft(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_D)) {
		cam->moveRight(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_W)) {
		cam->moveForward(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_S)) {
		cam->moveBackward(speed, dt);
	}
}

void App::draw() 
{

}

void App::drawGUI() 
{

}

void App::onKeyDown(SDL_KeyboardEvent key) 
{

}

void App::onKeyUp(SDL_KeyboardEvent key) 
{

}

void App::onMouseDown(SDL_MouseButtonEvent mButton) 
{

}

void App::onMouseUp(SDL_MouseButtonEvent mButton) 
{

}

void App::onMouseMove(SDL_MouseMotionEvent mEvent) 
{
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

