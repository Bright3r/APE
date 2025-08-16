#pragma once

#include "App.h"
#include "render/Camera.h"
#include "render/Context.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "render/Model.h"
#include "util/timing.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_events.h>

#include <memory>
#include <string_view>

class AppRunner {
private:
	// Application State
	//
	static inline std::unique_ptr<App> s_app;
	static inline std::unordered_map<SDL_Keycode, bool> s_key_state;

	// Rendering
	//
	static inline std::shared_ptr<APE::Render::Context> s_context;
	static inline std::shared_ptr<APE::Render::Shader> s_shader;
	static inline std::unique_ptr<APE::Render::Renderer> s_renderer;
	static inline std::unique_ptr<APE::Render::Camera> s_main_camera;

	// Timing
	//
	static inline bool s_quit;
	static inline int s_framerate;
	static inline APE::Timing::millis s_last_frame_time;

public:
	// Main Functions
	//
	static void init(
		std::string_view window_title,
		int window_width,
		int window_height);

	static void pollEvents();

	static void stepGameloop();

	static void run();

	static bool keyDown(SDL_Keycode key);

	static void setTabIn(bool is_tabbed_in);


	// Graphics Functions
	static std::unique_ptr<APE::Render::Shader> createShader(
		const APE::Render::ShaderDescription& vert_shader_desc, 
		const APE::Render::ShaderDescription& frag_shader_desc);

	static void useShader(std::shared_ptr<APE::Render::Shader> shader);

	static APE::Render::Camera* getMainCamera();

	static void draw(APE::Render::Model* mesh);

	static void resizeWindow(const SDL_Event& event);

	static void setWindowTitle(std::string_view window_title);


	// Utility Functions
	//
	static bool getQuit();

	static void setQuit(bool quit);

	static int getFramerate();

	static void setFramerate(int fps);

	static APE::Timing::seconds getLastFrameTimeSec();

	static APE::Timing::millis getLastFrameTimeMS();
};

