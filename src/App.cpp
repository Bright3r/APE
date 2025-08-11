#include "App.h"
#include "AppRunner.h"
#include "render/Model.h"
#include "util/Logger.h"

#include <glm/fwd.hpp>

void App::setup() {
	// model = APE::Render::Model("res/models/textured-cube.obj");
	model = APE::Render::Model("res/models/che/scene.gltf");
	// model = APE::Render::Model("res/models/ship/source/full_scene.fbx");
}

void App::update() {
	// std::cout << "Last Frame Time: " 
	// 	<< AppRunner::lastFrameTimeMS().count() 
	// 	<< std::endl;
	
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	// cam->print();
}

void App::draw() {
	model.getTransform().position.x *= -1;
	AppRunner::draw(model);

	model.getTransform().position.x *= -1;
	AppRunner::draw(model);
}

void App::onKeyDown(SDL_KeyboardEvent key) {
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	float speed = 5.f;
	float dt = AppRunner::getLastFrameTimeSec().count();

	switch (key.key) {
		case SDLK_Q:
			AppRunner::setQuit(true);
			break;
		case SDLK_W:
			cam->moveForward(speed, dt);
			break;
		case SDLK_S:
			cam->moveForward(-speed, dt);
			break;
		case SDLK_A:
			cam->moveRight(-speed, dt);
			break;
		case SDLK_D:
			cam->moveRight(speed, dt);
			break;
		case SDLK_SPACE:
			cam->moveUp(speed, dt);
			break;
		case SDLK_LCTRL:
			cam->moveUp(-speed, dt);
			break;
		case SDLK_M:
			model.getTransform().position.x += 1;
			APE_TRACE("Mesh.pos.x = {}", model.getTransform().position.x);
			break;
		case SDLK_N:
			model.getTransform().position.x -= 1;
			APE_TRACE("Mesh.pos.x = {}", model.getTransform().position.x);
			break;

	}
}

void App::onKeyUp(SDL_KeyboardEvent key) {

}

void App::onMouseDown(SDL_MouseButtonEvent mButton) {

}

void App::onMouseUp(SDL_MouseButtonEvent mButton) {

}

void App::onMouseMove(SDL_MouseMotionEvent mEvent) {
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

