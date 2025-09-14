#pragma once

#include "App.h"
#include "core/scene/Scene.h"
#include "core/render/Camera.h"
#include "core/render/Context.h"
#include "core/render/Renderer.h"
#include "core/render/Shader.h"
#include "util/Timing.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_events.h>

#include <filesystem>
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
	static inline APE::ECS::EntityHandle s_selected_ent;

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
		std::unique_ptr<App> app,
		std::string_view window_title,
		int window_width,
		int window_height) noexcept;

	static void pollEvents() noexcept;

	static void stepGameloop() noexcept;

	static void run() noexcept;

	static bool keyDown(SDL_Keycode key) noexcept;

	static void setTabIn(bool is_tabbed_in) noexcept;

	static APE::Scene& getWorld() noexcept;

	static void setSelectedEntity(const APE::ECS::EntityHandle& ent) noexcept;

	static APE::ECS::EntityHandle getSelectedEntity() noexcept;

	static void saveScene(std::filesystem::path save_path) noexcept;

	static bool loadScene(std::filesystem::path load_path) noexcept;


	// Graphics Functions
	[[nodiscard]] static APE::Render::Renderer* getRenderer() noexcept;

	[[nodiscard]] static std::unique_ptr<APE::Render::Shader> createShader(
		const APE::Render::ShaderDescription& vert_shader_desc, 
		const APE::Render::ShaderDescription& frag_shader_desc) noexcept;

	static void useShader(std::shared_ptr<APE::Render::Shader> shader) noexcept;

	[[nodiscard]] static std::weak_ptr<APE::Render::Camera> getCamera() noexcept;

	static void setCamera(std::shared_ptr<APE::Render::Camera> cam) noexcept;

	static glm::mat4 getModelMatrix(APE::ECS::EntityHandle ent) noexcept;

	static void draw() noexcept;

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

