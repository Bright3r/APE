#include "AppRunner.h"
#include "render/Shader.h"
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_video.h>
#include <chrono>
#include <exception>


// Application State
//
std::unique_ptr<App> AppRunner::s_app {};

// Rendering
//
std::shared_ptr<APE::Render::Context> AppRunner::s_context {};
std::shared_ptr<APE::Render::Shader> AppRunner::s_shader {};
std::unique_ptr<APE::Render::Renderer> AppRunner::s_renderer {};
std::unique_ptr<APE::Render::Camera> AppRunner::s_main_camera {};

// Timing
//
bool AppRunner::s_quit {};
int AppRunner::s_framerate {};
APE::Timing::millis AppRunner::s_last_frame_time {};



void AppRunner::init(
	std::string_view window_title,
	int window_width,
	int window_height) 
{
	// Initialize w/ default app settings
	s_quit = false;
	s_framerate = 60.f;
	s_last_frame_time = std::chrono::milliseconds(0);

	// Create user-defined app
	s_app = std::make_unique<App>();

	// Initialize renderer w/ default shader
	s_context = std::make_shared<APE::Render::Context>(
		window_title, window_width, window_height, SDL_WINDOW_RESIZABLE
	);

	// Initialize main cam
	s_main_camera = std::make_unique<APE::Render::Camera>();

	// Create Renderer
	s_renderer = std::make_unique<APE::Render::Renderer>(
		s_context, 
		s_main_camera.get()
	);
}

void AppRunner::pollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				setQuit(true);
				break;
			case SDL_EVENT_KEY_DOWN:
				s_app->onKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
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

void AppRunner::stepGameloop()
{
	// Poll User Input
	pollEvents();

	// Update App Data
	s_app->update();

	// Draw To Screen
	s_renderer->beginDrawing();
	s_app->draw();
	s_renderer->endDrawing();
}

void AppRunner::run() 
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

std::unique_ptr<APE::Render::Shader> AppRunner::createShader(
	const APE::Render::ShaderDescription& shader_desc) 
{
	return s_renderer->createShader(shader_desc);
}

void AppRunner::useShader(std::shared_ptr<APE::Render::Shader> shader)
{
	s_shader = shader;
	s_renderer->useShader(shader.get());
}

APE::Render::Camera* AppRunner::getMainCamera() 
{
	return s_main_camera.get();
}

void AppRunner::draw(APE::Render::Model& model)
{
	s_renderer->draw(model);
}

bool AppRunner::getQuit() 
{
	return s_quit;
}

void AppRunner::setQuit(bool quit) 
{
	s_quit = quit;
}

int AppRunner::getFramerate() 
{
	return s_framerate;
}

void AppRunner::setFramerate(int fps) 
{
	s_framerate = fps;
}

APE::Timing::seconds AppRunner::getLastFrameTimeSec() 
{
	return std::chrono::duration_cast<APE::Timing::seconds>(s_last_frame_time);
}

APE::Timing::millis AppRunner::getLastFrameTimeMS() 
{
	return std::chrono::duration_cast<APE::Timing::millis>(s_last_frame_time);
}

