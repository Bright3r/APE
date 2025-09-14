#pragma once

#include "Application.h"
#include "core/scene/Scene.h"
#include "core/render/Camera.h"
#include "core/render/Context.h"
#include "core/render/Renderer.h"
#include "util/Timing.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_events.h>

#include <filesystem>
#include <memory>
#include <string_view>

namespace APE {

class Engine {
private:
	// Application State
	//
	static inline std::unique_ptr<Application> s_app;
	static inline std::unordered_map<SDL_Keycode, bool> s_key_state;

	// Rendering
	//
	static inline std::shared_ptr<Render::Context> s_context;
	static inline std::shared_ptr<Render::Camera> s_camera;
	static inline std::unique_ptr<Render::Renderer> s_renderer;

	// Timing
	//
	static inline bool s_quit;
	static inline int s_framerate;
	static inline Timing::millis s_last_frame_time;

public:
	// Main Functions
	//
	static void init(
		std::unique_ptr<Application> app,
		std::string_view window_title,
		int window_width,
		int window_height) noexcept;

	static void pollEvents() noexcept;

	static void stepGameloop() noexcept;

	static void run() noexcept;

	static bool keyDown(SDL_Keycode key) noexcept;

	static void setTabIn(bool is_tabbed_in) noexcept;

	static void saveScene(
		std::filesystem::path save_path,
		Scene& world) noexcept;

	static bool loadScene(
		std::filesystem::path load_path,
		Scene& world) noexcept;


	// Graphics Functions
	[[nodiscard]] static Render::Renderer* getRenderer() noexcept;

	[[nodiscard]] static std::weak_ptr<Render::Camera> getCamera() noexcept;

	static void setCamera(std::shared_ptr<Render::Camera> cam) noexcept;

	static void draw(Scene& scene) noexcept;

	static void resizeWindow(const SDL_Event& event) noexcept;

	static void setWindowTitle(const std::string& window_title) noexcept;


	// Utility Functions
	//
	[[nodiscard]] static bool getQuit() noexcept;

	static void setQuit(bool quit) noexcept;

	[[nodiscard]] static int getFramerate() noexcept;

	static void setFramerate(int fps) noexcept;

	[[nodiscard]] static Timing::seconds getLastFrameTimeSec() noexcept;

	[[nodiscard]] static Timing::millis getLastFrameTimeMS() noexcept;
};

};	// end of namespace

