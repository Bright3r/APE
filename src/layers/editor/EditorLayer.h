#pragma once

#include "core/Application.h"
#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/ecs/Registry.h"
#include "core/render/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <memory>

namespace APE::Editor {

void run() noexcept;

class EditorLayer : public Application {
	std::shared_ptr<Render::Camera> cam;
	ECS::EntityHandle selected_ent;
	bool b_lock_selection = false;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::TRANSLATE;

public:
	void draw() noexcept override;
	void setup() noexcept override;
	void update() noexcept override;
	void drawGUI() noexcept override;


	// Helper functions
	void handleMouseButtonEvent(SDL_MouseButtonEvent m_button) noexcept;

	void drawAABB(
		Physics::Collider::AABB& aabb,
		TransformComponent& transform) noexcept;
	void drawNode(
		Physics::Collider::BVHNode& node,
		TransformComponent& transform) noexcept;
	void drawBVH(
		Physics::Collider::BVH& bvh,
		TransformComponent& transform) noexcept;

	glm::vec3 screenToWorld(glm::vec2 screen_coords) noexcept;
};

};	// end of namespace

