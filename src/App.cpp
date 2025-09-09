#include "App.h"
#include "core/AppRunner.h"
#include "render/Camera.h"
#include "render/Model.h"
#include "render/Shapes/Cube.h"
#include "render/Shapes/Sphere.h"
#include "render/Shapes/Cone.h"
#include "render/Shapes/Cylinder.h"

#include <glm/fwd.hpp>
#include <imgui.h>

#include <cmath>
#include <vector>

void App::setup() 
{
	AppRunner::setWindowTitle("APE Engine");;

	auto& world = AppRunner::getWorld();

	// auto car = std::make_unique<APE::Render::Model>("res/models/che/scene.gltf");
	// world.addModel(car.get());

	// auto ship = std::make_unique<APE::Render::Model>(
	// 	"res/models/ship/source/full_scene.fbx"
	// );
	// world.addModel(ship.get());

	auto cube = std::make_unique<APE::Render::Cube>();
	auto sphere = std::make_unique<APE::Render::Sphere>();
	auto cone = std::make_unique<APE::Render::Cone>();
	auto cylinder = std::make_unique<APE::Render::Cylinder>();

	constexpr int NUM_SHAPE_SETS = 50;
	std::vector<APE::Render::Model*> models;
	for (size_t i = 0; i < NUM_SHAPE_SETS; ++i) {
		models.push_back(cube.get());
		models.push_back(sphere.get());
		models.push_back(cone.get());
		models.push_back(cylinder.get());
	}

	int sqrt = std::sqrt(models.size());
	for (int i = 0; i < models.size(); ++i) {
		APE::Render::Model* model = models[i];
		model->getTransform().position.x = ((i % sqrt) - (sqrt / 2.f)) * 5;
		model->getTransform().position.z = ((i / sqrt) - (sqrt / 2.f)) * 5;

		world.addModel(models[i]);
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

	// Camera Tab In
	if (AppRunner::keyDown(SDLK_TAB)) {
		bool is_locked = cam->getLocked();
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

