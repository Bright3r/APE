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

void App::setup() 
{
	AppRunner::setWindowTitle("APE Engine");;

	// scene.emplace_back(
	// 	std::make_unique<APE::Render::Model>("res/models/che/scene.gltf")
	// );

	constexpr int NUM_SHAPE_SETS = 1;
	for (size_t i = 0; i < NUM_SHAPE_SETS; ++i) {
		scene.emplace_back(std::make_unique<APE::Render::Cube>());
		scene.emplace_back(std::make_unique<APE::Render::Sphere>());
		scene.emplace_back(std::make_unique<APE::Render::Cone>());
		scene.emplace_back(std::make_unique<APE::Render::Cylinder>());
	}

	// model = std::make_unique<APE::Render::Model>(
	// 	"res/models/ship/source/full_scene.fbx"
	// );
	
	AppRunner::setFramerate(144);

	cam = std::make_shared<APE::Render::Camera>(
		glm::vec3(-2.5f, 8.f, 8.f),
		-45.f,
		-90.f,
		45.f,
		0.3f
	);
	AppRunner::setCamera(cam);
	AppRunner::setTabIn(true);
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
	ImGui::Text("Camera");

	glm::vec3 pos { cam->getPosition() };
	ImGui::SliderFloat3("pos", &pos[0], -100.f, 100.f, "%.2f");
	cam->setPosition(pos);

	float pitch { cam->getPitch() };
	ImGui::SliderFloat("pitch", &pitch, -90.f, 90.f, "%.2f");
	cam->setPitch(pitch);

	float yaw { cam->getYaw() };
	ImGui::SliderFloat("yaw", &yaw, -360.f, 360.f, "%.2f");
	cam->setYaw(yaw);

	float fov { cam->getFOV() };
	ImGui::SliderFloat("fov", &fov, 10.f, 120.f, "%.1f");
	cam->setFOV(fov);

	float sensitivity { cam->getSensitivity() };
	ImGui::SliderFloat("sensitivity", &sensitivity, 0.01f, 1.f, "%.2f");
	cam->setSensitivity(sensitivity);
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

