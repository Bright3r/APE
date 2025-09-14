#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/components/Render.h"
#include "core/scene/Serialize.h"
#include "util/Logger.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <imgui_impl_sdl3.h>
#include <ImGuizmo.h>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <cstring>
#include <chrono>
#include <utility>

namespace APE {

void Engine::init(
	std::unique_ptr<Application> app,
	std::string_view window_title,
	int window_width,
	int window_height) noexcept 
{
	// Initialize w/ default app settings
	s_quit = false;
	s_framerate = 60.f;
	s_last_frame_time = std::chrono::milliseconds(0);

	// Create app state
	s_app = std::move(app);

	// Initialize renderer w/ default shader
	s_context = std::make_shared<Render::Context>(
		window_title,
		window_width,
		window_height,
		SDL_WINDOW_RESIZABLE
	);

	// Initialize main cam
	s_camera = std::make_shared<Render::Camera>();

	// Create renderer
	s_renderer = std::make_unique<Render::Renderer>(s_context);
}

void Engine::pollEvents() noexcept
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		ImGui_ImplSDL3_ProcessEvent(&event);
		switch (event.type) {
			case SDL_EVENT_WINDOW_RESIZED:
				resizeWindow(event);
				break;
			case SDL_EVENT_QUIT:
				setQuit(true);
				break;
			case SDL_EVENT_KEY_DOWN:
				s_key_state[event.key.key] = true;
				s_app->onKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
				s_key_state[event.key.key] = false;
				s_app->onKeyUp(event.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				s_app->onMouseDown(event.button);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				s_app->onMouseUp(event.button);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				s_app->onMouseMove(event.motion);
				break;
		}
	}
}

void Engine::stepGameloop() noexcept
{
	// Poll User Input
	pollEvents();

	// Update Application Data
	s_app->update();

	// Draw to Screen
	s_renderer->beginDrawing();

	// 3D Scene
	draw(s_app->world);

	// GUI
	s_app->drawGUI();

	s_renderer->endDrawing();
}

void Engine::run() noexcept 
{
	// Initial Setup
	s_app->setup();

	// Game Loop
	while (!s_quit) {
		auto start = std::chrono::high_resolution_clock::now();

		// Time the execution of gameloop iteration
		Timing::millis loop_time_ms; 
		Timing::timeFunctionCall([&]() {
			stepGameloop();
		}, loop_time_ms);

		// Wait until the next frame time
		Timing::seconds target_frame_time { 1.0 / s_framerate };
		Timing::waitFor(target_frame_time - loop_time_ms);

		// Track total time of frame
		auto end = std::chrono::high_resolution_clock::now();
		s_last_frame_time = end - start;
	}

	std::terminate();
}

bool Engine::keyDown(SDL_Keycode key) noexcept
{
	if (s_key_state.contains(key)) {
		return s_key_state.at(key);
	}
	return false;
}

void Engine::setTabIn(bool is_tabbed_in) noexcept
{
	SDL_SetWindowRelativeMouseMode(s_context->window, is_tabbed_in);
}

void Engine::saveScene(std::filesystem::path save_path, Scene& world) noexcept
{
	Serialize::saveScene(save_path, world);
}

bool Engine::loadScene(std::filesystem::path load_path, Scene& world) noexcept
{
	if (load_path.extension() == ".json") {
		world = Serialize::loadScene(load_path);
		return true;
	}
	return false;
}

Render::Renderer* Engine::getRenderer() noexcept
{
	return s_renderer.get();
}

std::weak_ptr<Render::Camera> Engine::getCamera() noexcept 
{
	return s_camera;
}

void Engine::setCamera(std::shared_ptr<Render::Camera> cam) noexcept
{
	s_camera = cam;
}

void Engine::draw(Scene& world) noexcept
{
	auto view = world.registry.view<
		Render::MeshComponent,
		Render::MaterialComponent,
		TransformComponent,
		HierarchyComponent>();

	size_t num_objs { 0 };
	for (auto [ent, mesh, material, transform, hierarchy] : view) {
		glm::mat4 model_mat = world.getModelMatrix(ent);
		s_renderer->draw(mesh, material, s_camera, model_mat);
		
		++num_objs;
	}
}

void Engine::resizeWindow(const SDL_Event& event) noexcept
{
	s_context->window_width = event.window.data1;
	s_context->window_height = event.window.data2;
	s_renderer->reset();
}

void Engine::setWindowTitle(const std::string& window_title) noexcept
{
	s_context->title = window_title;
	SDL_SetWindowTitle(s_context->window, window_title.c_str());
}

bool Engine::getQuit() noexcept 
{
	return s_quit;
}

void Engine::setQuit(bool quit) noexcept 
{
	s_quit = quit;
}

int Engine::getFramerate() noexcept 
{
	return s_framerate;
}

void Engine::setFramerate(int fps) noexcept 
{
	APE_CHECK((fps > 0),
	   "Engine::setFramerate(int fps) Failed: fps <= 0"
	);
	s_framerate = fps;
}

Timing::seconds Engine::getLastFrameTimeSec() noexcept 
{
	return std::chrono::duration_cast<Timing::seconds>(s_last_frame_time);
}

Timing::millis Engine::getLastFrameTimeMS() noexcept 
{
	return std::chrono::duration_cast<Timing::millis>(s_last_frame_time);
}

};	// end of namespace

