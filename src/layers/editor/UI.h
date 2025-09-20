#pragma once

#include "core/Engine.h"
#include "core/components/Object.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/scene/ImageLoader.h"
#include "core/scene/Scene.h"
#include "util/Files.h"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
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
	bool& b_show_hitboxes) noexcept
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

		auto& hierarchy = 
			world.registry.getComponent<HierarchyComponent>(ent);

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

static inline void drawManipulatorPanel(
	Scene& world,
	const ECS::EntityHandle& ent,
	ImGuizmo::OPERATION& gizmo_op) noexcept
{
	ImGui::Begin("Manipulator Panel");

	// Tag
	if (world.registry.hasComponent<HierarchyComponent>(ent)) {
		auto& hierarchy = 
			world.registry.getComponent<HierarchyComponent>(ent);

		char buf[128];
		strncpy(buf, hierarchy.tag.c_str(), sizeof(buf));
		if (ImGui::InputText("Entity Tag", buf, sizeof(buf))) {
			hierarchy.tag = buf;
		}

		std::string children = 
			std::format("Num Children: {}", hierarchy.children.size());
		ImGui::Text("%s", children.c_str());
	}

	// Transform
	if (world.registry.hasComponent<TransformComponent>(ent)) {
		auto& transform = 
			world.registry.getComponent<TransformComponent>(ent);

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

	// Material
	if (world.registry.hasComponent<Render::MaterialComponent>(ent)) {
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
			ImGuizmo::MODE::WORLD,
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
	}
}



};
