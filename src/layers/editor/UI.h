#pragma once

#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/components/Physics.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/scene/ImageLoader.h"
#include "core/scene/ModelLoader.h"
#include "core/scene/Scene.h"
#include "phys/Physics.h"
#include "util/Files.h"

#include <Jolt/Jolt.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/EActivation.h>

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_impl_sdl3.h>
#include <ImGuizmo.h>
#include <glm/gtc/quaternion.hpp>

#include <filesystem>
#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

namespace APE::Editor {

static inline void drawDebugPanel(
	APE::Scene& world,
	bool& b_lock_selection,
	bool& b_show_hitboxes,
	bool& b_play_simulation,
	float& mouse_force) noexcept
{
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) 
	{
		if (ImGui::BeginMenu("File")) 
		{
			if (ImGui::MenuItem("Open")) 
			{
				std::filesystem::path path;
				Files::Status status = 
					Files::openDialog(path);
				if (status == Files::Status::Sucess) {
					Engine::loadScene(path, world);
				}
			}
			if (ImGui::MenuItem("Save As")) 
			{
				std::filesystem::path path;
				Files::Status status = 
					Files::openDialog(path);
				if (status == Files::Status::Sucess) {
					Engine::saveScene(path, world);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}


	ImGui::Text("Camera");
	auto cam = Engine::getCamera().lock();

	glm::vec3 pos { cam->getPosition() };
	ImGui::SliderFloat3("pos", &pos[0], -100.f, 100.f, "%.2f");
	cam->setPosition(pos);

	float pitch { cam->getPitch() };
	ImGui::SliderFloat("pitch", &pitch, -90.f, 90.f, "%.2f");
	cam->setPitch(pitch);

	float yaw { cam->getYaw() };
	ImGui::SliderFloat("yaw", &yaw, -360.f, 360.f, "%.2f");
	cam->setYaw(yaw);

	float fov { cam->getFOV() };
	ImGui::SliderFloat("fov", &fov, 10.f, 120.f, "%.1f");
	cam->setFOV(fov);

	float sensitivity { cam->getSensitivity() };
	ImGui::SliderFloat("sensitivity", &sensitivity, 0.01f, 1.f, "%.2f");
	cam->setSensitivity(sensitivity);

	if (ImGui::RadioButton("lock camera", cam->isLocked())) {
		cam->setLocked(!cam->isLocked());
	}
	if (ImGui::RadioButton("lock selection", b_lock_selection)) {
		b_lock_selection = !b_lock_selection;
	}
	if (ImGui::RadioButton("show hitboxes", b_show_hitboxes)) {
		b_show_hitboxes = !b_show_hitboxes;
	}
	if (ImGui::RadioButton("play simulation", b_play_simulation)) {
		b_play_simulation = !b_play_simulation;
	}

	auto renderer = Engine::renderer();
	if (ImGui::RadioButton("show normals", renderer->debug_mode.show_normals)) {
		renderer->debug_mode.show_normals = !renderer->debug_mode.show_normals;
	}
	if (ImGui::RadioButton("wireframe mode", renderer->wireframe_mode)) {
		renderer->wireframe_mode = !renderer->wireframe_mode;
	}


	ImGui::Text("Lighting");
	auto& light = renderer->light;
	ImGui::SliderInt("type", reinterpret_cast<int*>(&light.type), 0, Render::LightType::Size);
	ImGui::InputFloat3("position", glm::value_ptr(light.position));
	ImGui::InputFloat3("attenuation", glm::value_ptr(light.attenuation));
	ImGui::InputFloat3("direction", glm::value_ptr(light.dir));
	ImGui::ColorPicker4("ambient", glm::value_ptr(light.ambient_color));
	ImGui::ColorPicker4("diffuse", glm::value_ptr(light.diffuse_color));
	ImGui::ColorPicker4("specular", glm::value_ptr(light.specular_color));

	ImGui::End();
}

static inline void drawSceneHierarchyPanel(
	Scene& world,
	ECS::EntityHandle& selected_ent) noexcept
{
	ImGui::Begin("Scene Hierarchy Panel");

	// Draw a button for each entity in the hierarchy
	// with padding to visualize nesting
	using EntityWithPad = std::tuple<ECS::EntityHandle, std::string, float>;
	std::vector<EntityWithPad> draw_list;

	// DFS over world entities
	std::vector<EntityWithPad> stack;
	stack.push_back({ world.root, "", 0.f });
	ImVec2 button_sz { 0.f, 0.f };
	while (!stack.empty()) {
		auto [ent, x, pad] = stack.back();
		stack.pop_back();

		auto& hierarchy = world.registry.getComponent<HierarchyComponent>(ent);

		// Create a unique tag for each entity, indented past its parent
		auto padded_tag = std::format(
			"{}###{}",
			hierarchy.tag.c_str(),
			ent.id
		);
		ImVec2 pad_sz { ImGui::CalcTextSize(padded_tag.c_str()) };
		button_sz = { 
			std::max(button_sz.x, pad_sz.x),
			std::max(button_sz.y, pad_sz.y) 
		};
		draw_list.emplace_back(ent, padded_tag, pad);

		// Add padded children
		float child_pad = pad + 1;
		for (auto child : hierarchy.children) {
			if (world.registry.hasComponent<HierarchyComponent>(child)) {
				stack.push_back({ child, "", child_pad  });
			}
		}
	}

	// Draw a button to select each entity
	for (auto [ent, padded_tag, pad] : draw_list) {
		ImVec2 cursor_pos = ImGui::GetCursorPos();
		float cursor_offset = pad * button_sz.x;
		ImGui::SetCursorPos({ cursor_pos.x + cursor_offset, cursor_pos.y });
		if (ImGui::Button(padded_tag.c_str(), { button_sz.x, 2*button_sz.y })) {
			selected_ent = ent;
		}
	}

	ImGui::End();
}

enum ColliderShape
{
	Box = 0,
	Sphere,
	END
};
std::vector<std::string> collider_shapes {
	"Box",
	"Sphere"
};

ColliderShape getColliderShape() noexcept
{
	static ColliderShape curr_shape = Box;
	if (ImGui::BeginCombo("Collider Shape", collider_shapes.data()[curr_shape].c_str()))
	{
		for (auto s = 0; s < ColliderShape::END; ++s)
		{
			bool is_selected = (curr_shape == s);
			if (ImGui::Selectable(collider_shapes.data()[s].c_str(), is_selected))
			{
				curr_shape = static_cast<ColliderShape>(s);
			}

			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	return curr_shape;
}

JPH::Shape* createColliderShape(ColliderShape selected_shape, bool make_shape) noexcept
{
	JPH::Shape* shape {};
	switch (selected_shape)
	{
	case Box:
	{
		static glm::vec3 bounds(1.f);
		ImGui::InputFloat3("Half Extents", glm::value_ptr(bounds), "%.2f");
		if (make_shape) shape = new JPH::BoxShape(JPH::Vec3(bounds.x, bounds.y, bounds.z));
		break;
	}
	case Sphere:
	{
		static float radius = 1.f;
		ImGui::InputFloat("Radius", &radius);
		if (make_shape) shape = new JPH::SphereShape(radius);
		break;
	}
	default:
	{
		shape = nullptr;
		break;
	}
	}

	return shape;
}


static inline void drawManipulatorPanel(
	Scene& world,
	const ECS::EntityHandle& ent,
	ImGuizmo::OPERATION& gizmo_op) noexcept
{
	ImGui::Begin("Manipulator Panel");

	std::stringstream id_ss;
	id_ss << "Entity ID: " << ent.id;
	ImGui::Text("%s", id_ss.str().c_str());

	// Tag
	if (world.registry.hasComponent<HierarchyComponent>(ent)) {
		ImGui::Text("Hierarchy");
		auto& hierarchy = world.registry.getComponent<HierarchyComponent>(ent);

		char buf[128];
		strncpy(buf, hierarchy.tag.c_str(), sizeof(buf));
		if (ImGui::InputText("Entity Tag", buf, sizeof(buf))) {
			hierarchy.tag = buf;
		}

		std::string children = std::format("Num Children: {}", hierarchy.children.size());
		ImGui::Text("%s", children.c_str());
	}

	// Create child entity
	if (ImGui::Button("Add child"))
	{
		world.createEntity(ent);
	}

	// Transform
	if (world.registry.hasComponent<TransformComponent>(ent)) {
		ImGui::Text("Transform");
		auto& transform = world.registry.getComponent<TransformComponent>(ent);

		// Select gizmo operation
		if (ImGui::RadioButton("Translate", gizmo_op == ImGuizmo::TRANSLATE)) {
			gizmo_op = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", gizmo_op == ImGuizmo::ROTATE)) {
			gizmo_op = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", gizmo_op == ImGuizmo::SCALE)) {
			gizmo_op = ImGuizmo::SCALE;
		}

		// Manually edit transform
		auto matrix = transform.getModelMatrix();
		glm::vec3 translate, rotate, scale;
		ImGuizmo::DecomposeMatrixToComponents(
			&matrix[0][0],
			&translate[0],
			&rotate[0],
			&scale[0]
		);
		
		ImGui::InputFloat3("Translate", &translate[0]);
		ImGui::InputFloat3("Rotate", &rotate[0]);
		ImGui::InputFloat3("Scale", &scale[0]);

		ImGuizmo::RecomposeMatrixFromComponents(
			&translate[0],
			&rotate[0],
			&scale[0],
			&matrix[0][0]
		);
		transform = TransformComponent::fromMatrix(matrix);
	}
	else if (ImGui::Button("Add Transform"))
	{
		world.registry.emplaceComponent<TransformComponent>(ent);
	}

	// Create nested model
	if (ImGui::Button("Add model"))
	{
		std::filesystem::path model_path;
		auto status = Files::openDialog(model_path);
		if (status == Files::Status::Sucess) 
		{
			auto model_handle = ModelLoader::load(model_path);
			auto model = world.addModel(model_handle);
			world.setParent(model, ent);
		}
	}

	// Material
	if (world.registry.hasComponent<Render::MaterialComponent>(ent)) {
		ImGui::Text("Material");
		if (ImGui::Button("Change Texture")) {
			std::filesystem::path tex_path;
			auto status = Files::openDialog(tex_path);
			if (status == Files::Status::Sucess) {
				auto tex_handle = ImageLoader::load(tex_path);
				world.registry.replaceComponent<
					Render::MaterialComponent>(
						ent,
						tex_handle
					);
			}
		}
	}

	// Physics
	ImGui::Text("Physics");
	auto& body_if = world.phys_system->phys_system.GetBodyInterface();
	if (world.registry.hasComponent<Phys::PhysicsComponent>(ent))
	{
		auto& pbody = world.registry.getComponent<Phys::PhysicsComponent>(ent);

		static glm::vec3 vel {};
		ImGui::SliderFloat3("Velocity Addition", glm::value_ptr(vel), -20.f, 20.f, "%.1f");
		if (ImGui::Button("Add Velocity"))
		{
			body_if.AddLinearVelocity(pbody.body_id, JPH::Vec3(vel.x, vel.y, vel.z));
		}

		if (ImGui::Button("Remove Body"))
		{
			body_if.RemoveBody(pbody.body_id);
			Engine::world().registry.removeComponent<Phys::PhysicsComponent>(ent);
		}
	}
	else
	{
		ImGui::Text("Create Body");

		TransformComponent transform {};
		if (Engine::world().registry.hasComponent<TransformComponent>(ent))
		{
			transform = Engine::world().registry.getComponent<TransformComponent>(ent);
		}

		static bool is_static = false;
		if (ImGui::RadioButton("Static", is_static))
		{
			is_static = !is_static;
		}

		static bool is_moving = true;
		if (ImGui::RadioButton("Movable", is_moving))
		{
			is_moving = !is_moving;
		}

		static glm::vec3 pos = transform.position;
		ImGui::InputFloat3("Position", glm::value_ptr(pos), "%.2f");

		static glm::quat rot = transform.rotation;
		ImGui::InputFloat4("Orientation", glm::value_ptr(rot), "%.2f");

		auto selected_shape = getColliderShape();
		createColliderShape(selected_shape, false);

		if (ImGui::Button("Create Body"))
		{
			auto shape = createColliderShape(selected_shape, true);
			JPH::BodyCreationSettings settings(
				shape,
				JPH::RVec3(pos.x, pos.y, pos.z),
				JPH::Quat(rot.x, rot.y, rot.z, rot.w),
				(is_static) ? JPH::EMotionType::Static : JPH::EMotionType::Dynamic,
				(is_moving) ? Phys::Layers::MOVING : Phys::Layers::NON_MOVING
			);
			Engine::world().createAndRegisterPhysicsBody(ent, settings);

			Engine::world().phys_system->phys_system.OptimizeBroadPhase();
		}
	}

	ImGui::End();
}


static inline void drawGizmo(
	Scene& world,
	const ECS::EntityHandle& ent,
	ImGuizmo::OPERATION gizmo_op) noexcept 
{
	if (world.registry.hasAllComponents<TransformComponent, HierarchyComponent>(ent)) {
		// Get transform
		auto [transform, hierarchy] = world.registry.getComponents<
			TransformComponent, HierarchyComponent>(ent);

		auto parent_world_mat = world.getModelMatrix(hierarchy.parent);
		auto world_mat = parent_world_mat * transform.getModelMatrix();

		// Draw gizmo
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		auto camera = Engine::getCamera();
		APE_CHECK(!camera.expired(),
		   "UI::drawGizmo() Failed: camera is nullptr."
		);
		auto cam = camera.lock();

		auto view = cam->getViewMatrix();
		auto proj = cam->getProjectionMatrix(Engine::context()->getAspectRatio());
		ImGuizmo::Manipulate(
			glm::value_ptr(view),
			glm::value_ptr(proj),
			gizmo_op,
			ImGuizmo::MODE::LOCAL,
			glm::value_ptr(world_mat),
			NULL,
			NULL
		);

		// Update transform with gizmo changes
		auto new_loc_mat = glm::inverse(parent_world_mat) * world_mat;
		auto new_transform = TransformComponent::fromMatrix(new_loc_mat);

		constexpr float epsilon { 0.001f };
		auto vec_equal = [=](const auto& a, const auto& b) {
			auto diff = glm::abs(a - b);
			return diff.x < epsilon &&
				diff.y < epsilon &&
				diff.z < epsilon;
		};
		auto quat_equal = [&](const auto& a, const auto& b) {
			auto dot = glm::abs(glm::dot(a, b));
			return dot > (1.f - epsilon);
		};

		bool b_degenerate = 
			!vec_equal(new_transform.scale, transform.scale) &&
			(!vec_equal(new_transform.position, transform.position) ||
			!quat_equal(new_transform.rotation, transform.rotation));
		if (!b_degenerate)
		{
			transform = new_transform;
		}


		// Sync physics state
		if (world.registry.hasComponent<Phys::PhysicsComponent>(ent))
		{
			auto& body_if = world.phys_system->phys_system.GetBodyInterface();

			auto pbody = world.registry.getComponent<Phys::PhysicsComponent>(ent);
			JPH::Vec3 ppos { 
				transform.position.x,
				transform.position.y,
				transform.position.z 
			};
			JPH::Quat prot {
				transform.rotation.x,
				transform.rotation.y,
				transform.rotation.z,
				transform.rotation.w
			};
			body_if.SetPosition(pbody.body_id, ppos, JPH::EActivation::Activate);
			body_if.SetRotation(pbody.body_id, prot, JPH::EActivation::Activate);
		}

		if (world.registry.hasComponent<Phys::PlayerComponent>(ent))
		{
			auto& player_comp = world.registry.getComponent<Phys::PlayerComponent>(ent);
			auto& controller = player_comp.controller;
			controller.setPosition(transform.position);
			controller.setRotation(transform.rotation);
		}
	}
}



};

