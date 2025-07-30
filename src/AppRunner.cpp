#include "AppRunner.h"
#include "render/Shader.h"
#include <SDL3/SDL_video.h>
#include <chrono>
#include <functional>

namespace AppRunner {

void init(std::string_view window_title, int window_width, int window_height) 
{
	// Initialize w/ default app settings
	m_quit = false;
	m_framerate = 60.f;
	m_last_frame_time = std::chrono::milliseconds(0);

	// Create user-defined app
	m_app = std::make_unique<App>();

	// Initialize renderer w/ default shader
	m_context = std::make_shared<APE::Render::Context>(
		window_title, window_width, window_height, SDL_WINDOW_RESIZABLE);

	m_renderer = std::make_unique<APE::Render::Renderer>(
		m_context, 
		m_main_camera.get()
	);

	// Initialize main cam
	m_main_camera = std::make_unique<APE::Render::Camera>();
}

void pollEvents()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				setQuit(true);
				break;
			case SDL_EVENT_KEY_DOWN:
				m_app->onKeyDown(event.key);
				break;
			case SDL_EVENT_KEY_UP:
				m_app->onKeyUp(event.key);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				m_app->onMouseDown(event.button);
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				m_app->onMouseUp(event.button);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				m_app->onMouseMove(event.motion.x, event.motion.y);
				break;
		}
	}
}

void stepGameloop()
{
	// Poll User Input
	pollEvents();

	// Update App Data
	m_app->update();

	// Draw To Screen
	m_renderer->draw([&](SDL_GPURenderPass *render_pass) {
		m_app->draw(render_pass);
	});
}

void run() 
{
	// Initial Setup
	m_app->setup();

	// Game Loop
	while (!m_quit) {
		auto start = std::chrono::high_resolution_clock::now();

		// Time the execution of gameloop iteration
		APE::Timing::millis loop_time_ms; 
		APE::Timing::timeFunctionCall([&]() {
			stepGameloop();
		}, loop_time_ms);

		// Wait until the next frame time
		APE::Timing::seconds target_frame_time { 1.0 / m_framerate };
		APE::Timing::waitFor(target_frame_time - loop_time_ms);

		// Track total time of frame
		auto end = std::chrono::high_resolution_clock::now();
		m_last_frame_time = end - start;
	}
}

std::unique_ptr<APE::Render::Shader> createShader(
	const APE::Render::ShaderDescription& shader_desc)
{
	return m_renderer->createShader(shader_desc);
}

void useShader(std::shared_ptr<APE::Render::Shader> shader)
{
	m_shader = shader;
	m_renderer->useShader(shader.get());
}

APE::Render::Camera* getMainCamera() {
	return m_main_camera.get();
}

bool quit() 
{
	return m_quit;
}

void setQuit(bool quit) 
{
	m_quit = quit;
}

int framerate() 
{
	return m_framerate;
}

void setFramerate(int fps) 
{
	m_framerate = fps;
}

APE::Timing::seconds lastFrameTimeSec() 
{
	return std::chrono::duration_cast<APE::Timing::seconds>(m_last_frame_time);
}

APE::Timing::millis lastFrameTimeMS() 
{
	return std::chrono::duration_cast<APE::Timing::millis>(m_last_frame_time);
}

};	// end of namespace AppRunner

