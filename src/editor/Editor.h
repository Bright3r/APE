#pragma once

#include "Application.h"
#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/ecs/Registry.h"
#include "core/render/Camera.h"

#include <imgui.h>
#include <ImGuizmo.h>

#include <memory>

namespace APE::Editor {

void run() noexcept;

class EditorApplication : public Application {
private:
	std::shared_ptr<Render::Camera> cam;
	ECS::EntityHandle selected_ent;
	ImGuizmo::OPERATION gizmo_op = ImGuizmo::TRANSLATE;
	Physics::RigidBodyComponent* rbd;

public:
	static void run() noexcept;
	void draw() noexcept override;
	void setup() noexcept override;
	void update() noexcept override;
	void drawGUI() noexcept override;
	void onKeyDown(SDL_KeyboardEvent key) noexcept override;
	void onKeyUp(SDL_KeyboardEvent key) noexcept override;
	void onMouseDown(SDL_MouseButtonEvent mButton) noexcept override;
	void onMouseUp(SDL_MouseButtonEvent mButton) noexcept override;
	void onMouseMove(SDL_MouseMotionEvent mEvent) noexcept override;


	// Helper functions
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

