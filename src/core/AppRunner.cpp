#include "core/AppRunner.h"
#include "components/Object.h"
#include "render/Shader.h"
#include "util/Logger.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <cstring>
#include <imgui_impl_sdl3.h>

#include <chrono>
#include <utility>
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

	// Create renderer
	s_renderer = std::make_unique<APE::Render::Renderer>(s_context);

	// Select root entity
	s_selected_ent = s_world.root;
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

	// Draw to Screen
	s_renderer->beginDrawing();

	// 3D Scene
	draw();
	s_app->draw();

	// GUI
	s_app->drawGUI();
	drawSceneHierarchyPanel();
	drawManipulatorPanel();

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

APE::ECS::EntityHandle AppRunner::getSelectedEntity() noexcept
{
	return s_selected_ent;
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

	for (auto [ent, mesh, material, transform, hierarchy] : view) {
		glm::mat4 model_mat = getModelMatrix(ent);
		s_renderer->draw(mesh, material, s_camera, model_mat);
	}
}

void AppRunner::drawSceneHierarchyPanel() noexcept
{
	ImGui::Begin("Scene Hierarchy Panel");
	// ImGui::Text("Scene Hierarchy Panel");
	APE_TRACE("Selected Entity = {}", s_selected_ent.id);

	// dfs
	std::vector<std::pair<APE::ECS::EntityHandle, std::string>> stack;
	stack.push_back({ s_world.root, "" });
	while (!stack.empty()) {
		auto [ent, nest_padding] = stack.back();
		stack.pop_back();

		auto& hierarchy = 
			s_world.registry.getComponent<APE::HierarchyComponent>(ent);

		// Create button to select entity
		auto imgui_id = std::format("###{}", ent.id);
		auto padded_tag = std::format(
			"{}{}{}",
			nest_padding, 
			hierarchy.tag.c_str(),
			imgui_id
		);
		if (ImGui::Button(padded_tag.c_str())) {
			s_selected_ent = ent;
		}

		std::string child_padding = nest_padding + "     ";
		for (auto child : hierarchy.children) {
			if (s_world.registry.hasComponent<APE::HierarchyComponent>(child)) {
				stack.push_back({ child, child_padding });
			}
		}
	}

	ImGui::End();
}

void AppRunner::drawManipulatorPanel() noexcept
{
	ImGui::Begin("Manipulator Panel");

	auto ent = s_selected_ent;
	if (s_world.registry.hasComponent<APE::HierarchyComponent>(ent)) {
		auto& hierarchy = 
			s_world.registry.getComponent<APE::HierarchyComponent>(ent);

		char buf[128];
		strncpy(buf, hierarchy.tag.c_str(), sizeof(buf));
		if (ImGui::InputText("Entity Tag", buf, sizeof(buf))) {
			hierarchy.tag = buf;
		}
	}

	if (s_world.registry.hasComponent<APE::TransformComponent>(ent)) {
		auto& tran = 
			s_world.registry.getComponent<APE::TransformComponent>(ent);

		ImGui::SliderFloat3("Position", &tran.position[0], -100.f, 100.f, "%.2f");
		ImGui::SliderFloat3("Scale", &tran.scale[0], 0.01f, 10.f, "%.2f");
	}

	ImGui::End();
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

