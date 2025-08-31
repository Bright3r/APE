#pragma once

#include "App.h"
#include "core/Scene.h"
#include "render/Camera.h"
#include "render/Context.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "render/Model.h"
#include "util/Timing.h"

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

	// Scene Hierarchy
	//
	static inline APE::Scene s_world;

	// Rendering
	//
	static inline std::shared_ptr<APE::Render::Context> s_context;
	static inline std::shared_ptr<APE::Render::Shader> s_shader;
	static inline std::unique_ptr<APE::Render::Renderer> s_renderer;
	static inline std::shared_ptr<APE::Render::Camera> s_camera;

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
		int window_height) noexcept;

	static void pollEvents() noexcept;

	static void stepGameloop() noexcept;

	static void run() noexcept;

	static bool keyDown(SDL_Keycode key) noexcept;

	static void setTabIn(bool is_tabbed_in) noexcept;

	static APE::Scene& getWorld() noexcept;


	// Graphics Functions
	[[nodiscard]] static std::unique_ptr<APE::Render::Shader> createShader(
		const APE::Render::ShaderDescription& vert_shader_desc, 
		const APE::Render::ShaderDescription& frag_shader_desc) noexcept;

	static void useShader(std::shared_ptr<APE::Render::Shader> shader) noexcept;

	[[nodiscard]] static APE::Render::Camera* getCamera() noexcept;

	static void setCamera(std::shared_ptr<APE::Render::Camera> cam) noexcept;

	static void draw(APE::Render::Model* model) noexcept;

	static void resizeWindow(const SDL_Event& event) noexcept;

	static void setWindowTitle(std::string_view window_title) noexcept;


	// Utility Functions
	//
	[[nodiscard]] static bool getQuit() noexcept;

	static void setQuit(bool quit) noexcept;

	[[nodiscard]] static int getFramerate() noexcept;

	static void setFramerate(int fps) noexcept;

	[[nodiscard]] static APE::Timing::seconds getLastFrameTimeSec() noexcept;

	[[nodiscard]] static APE::Timing::millis getLastFrameTimeMS() noexcept;
};

