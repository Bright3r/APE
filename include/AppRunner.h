#pragma once

#include "App.h"
#include "render/Context.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "util/timing.h"

#include <memory>

#include <SDL3/SDL_events.h>
#include <string_view>

namespace AppRunner {
	// Application State
	//
	static std::unique_ptr<App> m_app;
	// Rendering
	static std::shared_ptr<APE::Render::Context> m_context;
	static std::shared_ptr<APE::Render::Shader> m_shader;
	static std::unique_ptr<APE::Render::Renderer> m_renderer;
	// Timing
	static bool m_quit;
	static int m_framerate;
	static APE::Timing::millis m_last_frame_time;


	// Main Functions
	//
	void init(std::string_view window_title, int window_width, int window_height);
	void pollEvents();
	void stepGameloop();
	void run();

	// Grpahics Functions
	std::unique_ptr<APE::Render::Shader> createShader(
		const APE::Render::ShaderDescription& shader_desc
	);
	void useShader(std::shared_ptr<APE::Render::Shader> shader);


	// Utility Functions
	//
	bool quit();
	void setQuit(bool quit);
	int framerate();
	void setFramerate(int fps);
	APE::Timing::seconds lastFrameTimeSec();
	APE::Timing::millis lastFrameTimeMS();
};
