#include "App.h"
#include "core/AppRunner.h"
#include "render/Cube.h"
#include "render/Model.h"
#include "util/Logger.h"

#include <glm/fwd.hpp>

void App::setup() {
	model = std::make_unique<APE::Render::Cube>();

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
	float speed = 5.f;
	float dt = AppRunner::getLastFrameTimeSec().count();
	
	if (b_quit) {
		AppRunner::setQuit(true);
	}
	if (b_move_forward) {
		cam->moveForward(speed, dt);
	}
	if (b_move_backward) {
		cam->moveForward(-speed, dt);
	}
	if (b_move_left) {
		cam->moveRight(-speed, dt);
	}
	if (b_move_right) {
		cam->moveRight(speed, dt);
	}
	if (b_move_up) {
		cam->moveUp(speed, dt);
	}
	if (b_move_down) {
		cam->moveUp(-speed, dt);
	}
	if (b_displace_right) {
		model->getTransform().position.x += 1;
		APE_TRACE("Mesh.pos.x = {}", model->getTransform().position.x);
	}
	if (b_displace_left) {
		model->getTransform().position.x -= 1;
		APE_TRACE("Mesh.pos.x = {}", model->getTransform().position.x);
	}
}

void App::draw() {
	model->getTransform().position.x *= -1;
	AppRunner::draw(model.get());

	model->getTransform().position.x *= -1;
	AppRunner::draw(model.get());
}

void App::onKeyDown(SDL_KeyboardEvent key) {
	switch (key.key) {
		case SDLK_Q:
			b_quit = true;
			break;
		case SDLK_W:
			b_move_forward = true;
			break;
		case SDLK_S:
			b_move_backward = true;
			break;
		case SDLK_A:
			b_move_left = true;
			break;
		case SDLK_D:
			b_move_right = true;
			break;
		case SDLK_SPACE:
			b_move_up = true;
			break;
		case SDLK_LCTRL:
			b_move_down = true;
			break;
		case SDLK_M:
			b_displace_right = true;
			break;
		case SDLK_N:
			b_displace_left = true;
			break;
	}
}

void App::onKeyUp(SDL_KeyboardEvent key) {
	switch (key.key) {
		case SDLK_Q:
			b_quit = false;
			break;
		case SDLK_W:
			b_move_forward = false;
			break;
		case SDLK_S:
			b_move_backward = false;
			break;
		case SDLK_A:
			b_move_left = false;
			break;
		case SDLK_D:
			b_move_right = false;
			break;
		case SDLK_SPACE:
			b_move_up = false;
			break;
		case SDLK_LCTRL:
			b_move_down = false;
			break;
		case SDLK_M:
			b_displace_right = false;
			break;
		case SDLK_N:
			b_displace_left = false;
			break;
	}
}

void App::onMouseDown(SDL_MouseButtonEvent mButton) {

}

void App::onMouseUp(SDL_MouseButtonEvent mButton) {

}

void App::onMouseMove(SDL_MouseMotionEvent mEvent) {
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

