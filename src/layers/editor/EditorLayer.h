#pragma once

#include "core/Application.h"
#include "core/ecs/Registry.h"
#include "core/render/Camera.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include <imgui.h>
#include <ImGuizmo.h>

#include <memory>

namespace APE::Editor {

void run() noexcept;

class EditorLayer : public Application {
	std::shared_ptr<Render::Camera> cam;
	ECS::EntityHandle selected_ent;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::TRANSLATE;
	bool b_lock_selection = false;
	bool b_show_hitboxes = true;
	float mouse_force = 0.f;

	bool b_play_simulation = false;

public:
	void draw() noexcept override;
	void setup() noexcept override;
	void update() noexcept override;
	void drawGUI() noexcept override;


	// Helper functions
	void handleMouseButtonEvent(SDL_MouseButtonEvent m_button) noexcept;

	void drawAABB(JPH::BodyID body_id) noexcept;

	glm::vec3 screenToWorld(glm::vec2 screen_coords) noexcept;
};

};	// end of namespace

