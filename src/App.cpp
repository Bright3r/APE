#include "App.h"
#include "AppRunner.h"

#include <iostream>

void App::setup() {

}

void App::update() {
	std::cout << "Last Frame Time: " 
		<< AppRunner::lastFrameTimeMS().count() 
		<< std::endl;
}

void App::draw(SDL_GPURenderPass *render_pass) {
	SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
}

void App::onKeyDown(SDL_KeyboardEvent key) {
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	float speed = 5.f;
	float dt = AppRunner::lastFrameTimeMS().count();

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
	}
}

void App::onKeyUp(SDL_KeyboardEvent key) {

}

void App::onMouseDown(SDL_MouseButtonEvent mButton) {

}

void App::onMouseUp(SDL_MouseButtonEvent mButton) {

}

void App::onMouseMove(float x, float y) {

}

