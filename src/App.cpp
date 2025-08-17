#include "App.h"
#include "core/AppRunner.h"
#include "render/Model.h"
#include "render/Shapes/Cube.h"
#include "render/Shapes/Sphere.h"
#include "render/Shapes/Cone.h"
#include "render/Shapes/Cylinder.h"
#include "util/Logger.h"

#include <cmath>
#include <glm/fwd.hpp>
#include <imgui.h>

void App::setup() 
{
	AppRunner::setWindowTitle("APE Engine");;

	// scene.emplace_back(
	// 	std::make_unique<APE::Render::Model>("res/models/che/scene.gltf")
	// );

	for (size_t i = 0; i < 50; ++i) {
		scene.emplace_back(std::make_unique<APE::Render::Cube>());
		scene.emplace_back(std::make_unique<APE::Render::Sphere>());
		scene.emplace_back(std::make_unique<APE::Render::Cone>());
		scene.emplace_back(std::make_unique<APE::Render::Cylinder>());
	}

	// model = std::make_unique<APE::Render::Model>(
	// 	"res/models/ship/source/full_scene.fbx"
	// );
	
	AppRunner::setFramerate(240);
}

void App::update() 
{
	std::string window_title = "FPS: " + 
		std::to_string(1000.0 / AppRunner::getLastFrameTimeMS().count());
	AppRunner::setWindowTitle(window_title);
	
	APE::Render::Camera* cam = AppRunner::getMainCamera();
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

	// // Model Movement
	// if (AppRunner::keyDown(SDLK_M)) {
	// 	model->getTransform().position.x += 1;
	// }
	// if (AppRunner::keyDown(SDLK_N)) {
	// 	model->getTransform().position.x -= 1;
	// }
	// if (AppRunner::keyDown(SDLK_X)) {
	// 	model->getTransform().scale.x += 1;
	// }
	// if (AppRunner::keyDown(SDLK_Y)) {
	// 	model->getTransform().scale.y += 1;
	// }
	// if (AppRunner::keyDown(SDLK_Z)) {
	// 	model->getTransform().scale.z += 1;
	// }
	// if (AppRunner::keyDown(SDLK_R)) {
	// 	model->getTransform().rotation *= 
	// 		glm::angleAxis(
	// 			0.1f, 
	// 			glm::normalize(glm::vec3(1, 1, 1))
	// 		);
	// }
}

void App::draw() 
{
	int sz = scene.size();
	int sqrt = std::sqrt(sz);
	for (int i = 0; i < sz; ++i) {
		std::unique_ptr<APE::Render::Model>& model = scene[i];
		model->getTransform().position.x = ((i % sqrt) - (sqrt / 2)) * 5;
		model->getTransform().position.z = ((i / sqrt) - (sqrt / 2)) * 5;
		// model->getTransform().position.x = i * 5;
		// model->getTransform().position.z = i * 5;
		AppRunner::draw(model.get());
	}
}

void App::drawGUI() 
{
	ImGui::ShowDemoWindow(&b_show_demo);
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
	APE::Render::Camera* cam = AppRunner::getMainCamera();
	cam->rotate(mEvent.xrel, mEvent.yrel);
}

