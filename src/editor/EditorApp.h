#pragma once

#include "App.h"
#include "core/render/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <memory>

namespace APE::Editor {

class EditorApp : public App {
private:
	std::shared_ptr<Render::Camera> cam;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::TRANSLATE;

public:
	void setup() noexcept override;
	void update() noexcept override;
	void drawGUI() noexcept override;
	void onKeyDown(SDL_KeyboardEvent key) noexcept override;
	void onKeyUp(SDL_KeyboardEvent key) noexcept override;
	void onMouseDown(SDL_MouseButtonEvent mButton) noexcept override;
	void onMouseUp(SDL_MouseButtonEvent mButton) noexcept override;
	void onMouseMove(SDL_MouseMotionEvent mEvent) noexcept override;
};

};	// end of namespace

