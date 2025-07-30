#pragma once

#include "App.h"
#include "render/Camera.h"
#include "render/Context.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "util/timing.h"

#include <memory>

#include <SDL3/SDL_events.h>
#include <string_view>

class AppRunner {
private:
	// Application State
	//
	static std::unique_ptr<App> m_app;

	// Rendering
	//
	static std::shared_ptr<APE::Render::Context> m_context;
	static std::shared_ptr<APE::Render::Shader> m_shader;
	static std::unique_ptr<APE::Render::Renderer> m_renderer;
	static std::unique_ptr<APE::Render::Camera> m_main_camera;

	// Timing
	//
	static bool m_quit;
	static int m_framerate;
	static APE::Timing::millis m_last_frame_time;

public:
	// Main Functions
	//
	static void 
	init(std::string_view window_title, int window_width, int window_height);

	static void pollEvents();

	static void stepGameloop();

	static void run();


	// Graphics Functions
	static std::unique_ptr<APE::Render::Shader> 
	createShader(const APE::Render::ShaderDescription& shader_desc);

	static void useShader(std::shared_ptr<APE::Render::Shader> shader);

	static APE::Render::Camera* getMainCamera();


	// Utility Functions
	//
	static bool getQuit();

	static void setQuit(bool quit);

	static int getFramerate();

	static void setFramerate(int fps);

	static APE::Timing::seconds getLastFrameTimeSec();

	static APE::Timing::millis getLastFrameTimeMS();
};

