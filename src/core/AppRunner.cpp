#include "core/AppRunner.h"
#include "components/Object.h"
#include "render/Shader.h"
#include "util/Logger.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <imgui_impl_sdl3.h>

#include <chrono>
#include <vector>

void AppRunner::init(
	std::string_view window_title,
	int window_width,
	int window_height) noexcept 
{
	// Initialize w/ default app settings
	s_quit = false;
	s_framerate = 60.f;
	s_last_frame_time = std::chrono::milliseconds(0);

	// Create app state
	s_app = std::make_unique<App>();

	// Initialize renderer w/ default shader
	s_context = std::make_shared<APE::Render::Context>(
		window_title,
		window_width,
		window_height,
		SDL_WINDOW_RESIZABLE
	);

	// Initialize main cam
	s_camera = std::make_shared<APE::Render::Camera>();

	// Create Renderer
	s_renderer = std::make_unique<APE::Render::Renderer>(s_context);
}

void AppRunner::pollEvents() noexcept
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

void AppRunner::stepGameloop() noexcept
{
	// Poll User Input
	pollEvents();

	// Update App Data
	s_app->update();

	// Draw To Screen
	s_renderer->beginDrawing();
	draw();
	s_app->draw();
	s_app->drawGUI();
	s_renderer->endDrawing();
}

void AppRunner::run() noexcept 
{
	// Initial Setup
	s_app->setup();

	// Game Loop
	while (!s_quit) {
		auto start = std::chrono::high_resolution_clock::now();

		// Time the execution of gameloop iteration
		APE::Timing::millis loop_time_ms; 
		APE::Timing::timeFunctionCall([&]() {
			stepGameloop();
		}, loop_time_ms);

		// Wait until the next frame time
		APE::Timing::seconds target_frame_time { 1.0 / s_framerate };
		APE::Timing::waitFor(target_frame_time - loop_time_ms);

		// Track total time of frame
		auto end = std::chrono::high_resolution_clock::now();
		s_last_frame_time = end - start;
	}

	std::terminate();
}

bool AppRunner::keyDown(SDL_Keycode key) noexcept
{
	if (s_key_state.find(key) != s_key_state.end()) {
		return s_key_state.at(key);
	}

	return false;
}

void AppRunner::setTabIn(bool is_tabbed_in) noexcept
{
	SDL_SetWindowRelativeMouseMode(s_context->window, is_tabbed_in);
}

APE::Scene& AppRunner::getWorld() noexcept
{
	return s_world;
}

std::unique_ptr<APE::Render::Shader> AppRunner::createShader(
	const APE::Render::ShaderDescription& vert_shader_desc, 
	const APE::Render::ShaderDescription& frag_shader_desc) noexcept 
{
	return s_renderer->createShader(vert_shader_desc, frag_shader_desc);
}

void AppRunner::useShader(std::shared_ptr<APE::Render::Shader> shader) noexcept
{
	s_shader = shader;
	s_renderer->useShader(shader.get());
}

APE::Render::Camera* AppRunner::getCamera() noexcept 
{
	return s_camera.get();
}

void AppRunner::setCamera(std::shared_ptr<APE::Render::Camera> cam) noexcept
{
	s_camera = cam;
}

glm::mat4 AppRunner::getModelMatrix(APE::ECS::EntityHandle ent) noexcept
{
	std::vector<glm::mat4> stack;
	while (s_world.registry.hasAllComponents<
		APE::TransformComponent, APE::HierarchyComponent>(ent)) 
	{
		auto [transform, hierarchy] = s_world.registry.getComponents<
			APE::TransformComponent, APE::HierarchyComponent>(ent);

		stack.emplace_back(transform.getModelMatrix());
		ent = hierarchy.parent;
	}

	glm::mat4 model_mat(1.f);
	while (!stack.empty()) {
		model_mat *= stack.back();
		stack.pop_back();
	}
	return model_mat;
}

void AppRunner::draw() noexcept
{
	auto view = s_world.registry.view<
		APE::Render::MeshComponent,
		APE::Render::MaterialComponent,
		APE::TransformComponent,
		APE::HierarchyComponent>();

	for (auto& ent : view) {
		auto [mesh, material, transform, hierarchy] = 
			s_world.registry.getComponents<
				APE::Render::MeshComponent,
				APE::Render::MaterialComponent,
				APE::TransformComponent,
				APE::HierarchyComponent>(ent);

		glm::mat4 model_mat = getModelMatrix(ent);
		s_renderer->draw(mesh, material, s_camera, model_mat);
	}
}

void AppRunner::resizeWindow(const SDL_Event& event) noexcept
{
	s_context->window_width = event.window.data1;
	s_context->window_height = event.window.data2;
	s_renderer->reset();
}

void AppRunner::setWindowTitle(std::string_view window_title) noexcept
{
	s_context->title = window_title;
	SDL_SetWindowTitle(s_context->window, std::string(window_title).c_str());
}

bool AppRunner::getQuit() noexcept 
{
	return s_quit;
}

void AppRunner::setQuit(bool quit) noexcept 
{
	s_quit = quit;
}

int AppRunner::getFramerate() noexcept 
{
	return s_framerate;
}

void AppRunner::setFramerate(int fps) noexcept 
{
	APE_CHECK((fps > 0),
	   "AppRunner::setFramerate(int fps) Failed: fps <= 0"
	);
	s_framerate = fps;
}

APE::Timing::seconds AppRunner::getLastFrameTimeSec() noexcept 
{
	return std::chrono::duration_cast<APE::Timing::seconds>(s_last_frame_time);
}

APE::Timing::millis AppRunner::getLastFrameTimeMS() noexcept 
{
	return std::chrono::duration_cast<APE::Timing::millis>(s_last_frame_time);
}

