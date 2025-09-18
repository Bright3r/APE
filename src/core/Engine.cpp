#include "core/Engine.h"
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
#include <chrono>

namespace APE {

void Engine::init(
	std::string_view window_title,
	int window_width,
	int window_height) noexcept 
{
	// Initialize w/ default app settings
	s_quit = false;
	s_framerate = 60.f;
	s_last_frame_time = std::chrono::milliseconds(0);

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

void Engine::pushLayer(std::unique_ptr<Application> layer) noexcept
{
	s_layers.push_back(std::move(layer));
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
			s_input.keyDown(event.key.key);
			break;
		case SDL_EVENT_KEY_UP:
			s_input.keyUp(event.key.key);
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
			s_input.mouseButton(event.button);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			s_input.mouseMotion(event.motion);
			break;
		}
	}
}

void Engine::stepGameloop() noexcept
{
	// Poll User Input
	pollEvents();

	// Update Application Layers
	for (auto& app : s_layers) {
		app->update();
	}
	s_input.flush();

	// Draw to Screen
	s_renderer->beginDrawing();

	// 3D Scene
	for (auto& app : s_layers) {
		app->draw();
	}

	// GUI
	for (auto& app : s_layers) {
		app->drawGUI();
	}

	s_renderer->endDrawing();
}

void Engine::run() noexcept 
{
	// Initial Setup
	for (auto& app : s_layers) {
		app->setup();
	}

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

Render::Renderer* Engine::renderer() noexcept
{
	return s_renderer.get();
}

Render::Context* Engine::context() noexcept
{
	return s_context.get();
}

Scene& Engine::world() noexcept
{
	return s_world;
}

Input::State& Engine::input() noexcept
{
	return s_input;
}

std::weak_ptr<Render::Camera> Engine::getCamera() noexcept 
{
	return s_camera;
}

void Engine::setCamera(std::shared_ptr<Render::Camera> cam) noexcept
{
	s_camera = cam;
}

void Engine::setTabIn(bool is_tabbed_in) noexcept
{
	SDL_SetWindowRelativeMouseMode(s_context->window, is_tabbed_in);
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

