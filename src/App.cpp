#include "App.h"
#include "AppRunner.h"

void App::setup() {

}

void App::update() {
	std::cout << "Last Frame Time: " 
		<< AppRunner::lastFrameTimeMS().count() 
		<< std::endl;
}

void App::draw() {

}

void App::onKeyDown(SDL_KeyboardEvent key) {

}

void App::onKeyUp(SDL_KeyboardEvent key) {

}

void App::onMouseDown(SDL_MouseButtonEvent mButton) {

}

void App::onMouseUp(SDL_MouseButtonEvent mButton) {

}

void App::onMouseMove(float x, float y) {

}

