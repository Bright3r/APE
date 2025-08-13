#include "App.h"
#include "core/AppRunner.h"
#include "render/Cube.h"
#include "render/Sphere.h"
#include "render/Model.h"
#include "util/Logger.h"

#include <glm/fwd.hpp>

void App::setup() {
	model = std::make_unique<APE::Render::Sphere>();

	// model = std::make_unique<APE::Render::Cube>();

	// model = std::make_unique<APE::Render::Model>("res/models/che/scene.gltf");

	// model = std::make_unique<APE::Render::Model>(
	// 	"res/models/ship/source/full_scene.fbx"
	// );
}

void App::update() {
	// std::cout << "Last Frame Time: " 
	// 	<< AppRunner::lastFrameTimeMS().count() 
	// 	<< std::endl;
	
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	// cam->print();
	float speed = 10.f;
	float dt = AppRunner::getLastFrameTimeSec().count();
	
	if (AppRunner::keyDown(SDLK_Q)) {
		AppRunner::setQuit(true);
	}
	if (AppRunner::keyDown(SDLK_W)) {
		cam->moveForward(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_S)) {
		cam->moveForward(-speed, dt);
	}
	if (AppRunner::keyDown(SDLK_A)) {
		cam->moveRight(-speed, dt);
	}
	if (AppRunner::keyDown(SDLK_D)) {
		cam->moveRight(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_SPACE)) {
		cam->moveUp(speed, dt);
	}
	if (AppRunner::keyDown(SDLK_LCTRL)) {
		cam->moveUp(-speed, dt);
	}
	if (AppRunner::keyDown(SDLK_M)) {
		model->getTransform().position.x += 1;
	}
	if (AppRunner::keyDown(SDLK_N)) {
		model->getTransform().position.x -= 1;
	}
	if (AppRunner::keyDown(SDLK_X)) {
		model->getTransform().scale.x += 1;
	}
	if (AppRunner::keyDown(SDLK_Y)) {
		model->getTransform().scale.y += 1;
	}
	if (AppRunner::keyDown(SDLK_Z)) {
		model->getTransform().scale.z += 1;
	}
	if (AppRunner::keyDown(SDLK_R)) {
		model->getTransform().rotation *= 
			glm::angleAxis(
				0.1f, 
		  		glm::normalize(glm::vec3(1, 1, 1))
			);
	}
}

void App::draw() {
	model->getTransform().position.x *= -1;
	AppRunner::draw(model.get());

	model->getTransform().position.x *= -1;
	AppRunner::draw(model.get());
}

void App::onKeyDown(SDL_KeyboardEvent key) {

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

