#pragma once

#include "core/AppRunner.h"
#include "core/components/Object.h"
#include "core/components/Render.h"
#include "core/ecs/Registry.h"
#include "core/scene/ImageLoader.h"
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

static inline void drawDebugPanel() noexcept
{
	ImGui::Begin("Debug", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) 
	{
		if (ImGui::BeginMenu("File")) 
		{
			if (ImGui::MenuItem("Open")) 
			{
				std::filesystem::path path;
				APE::Files::Status status = 
					APE::Files::openDialog(path);
				if (status == APE::Files::Status::Sucess) {
					AppRunner::loadScene(path);
				}
			}
			if (ImGui::MenuItem("Save As")) 
			{
				std::filesystem::path path;
				APE::Files::Status status = 
					APE::Files::openDialog(path);
				if (status == APE::Files::Status::Sucess) {
					AppRunner::saveScene(path);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}


	ImGui::Text("Camera");
	auto cam = AppRunner::getCamera().lock();

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

	auto renderer = AppRunner::getRenderer();
	if (ImGui::RadioButton("show normals", renderer->debug_mode.show_normals)) {
		renderer->debug_mode.show_normals = !renderer->debug_mode.show_normals;
	}


	ImGui::Text("Lighting");
	auto& light = renderer->light;
	ImGui::SliderInt("type", reinterpret_cast<int*>(&light.type), 0, APE::Render::LightType::Size);
	ImGui::InputFloat3("position", glm::value_ptr(light.position));
	ImGui::InputFloat3("attenuation", glm::value_ptr(light.attenuation));
	ImGui::InputFloat3("direction", glm::value_ptr(light.dir));
	ImGui::ColorPicker4("ambient", glm::value_ptr(light.ambient_color));
	ImGui::ColorPicker4("diffuse", glm::value_ptr(light.diffuse_color));
	ImGui::ColorPicker4("specular", glm::value_ptr(light.specular_color));

	ImGui::End();
}

static inline void drawSceneHierarchyPanel() noexcept
{
	ImGui::Begin("Scene Hierarchy Panel");

	// Draw a button for each entity in the hierarchy
	// with padding to visualize nesting
	using EntityWithPad = std::tuple<APE::ECS::EntityHandle, std::string, float>;
	std::vector<EntityWithPad> draw_list;

	// DFS over world entities
	std::vector<EntityWithPad> stack;
	auto& world = AppRunner::getWorld();
	stack.push_back({ world.root, "", 0.f });
	ImVec2 button_sz { 0.f, 0.f };
	while (!stack.empty()) {
		auto [ent, x, pad] = stack.back();
		stack.pop_back();

		auto& hierarchy = 
			world.registry.getComponent<APE::HierarchyComponent>(ent);

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
			if (world.registry.hasComponent<APE::HierarchyComponent>(child)) {
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
			AppRunner::setSelectedEntity(ent);
		}
	}

	ImGui::End();
}

static inline void drawManipulatorPanel(ImGuizmo::OPERATION& gizmo_op) noexcept
{
	ImGui::Begin("Manipulator Panel");
	auto ent = AppRunner::getSelectedEntity();
	auto& world = AppRunner::getWorld();

	// Tag
	if (world.registry.hasComponent<APE::HierarchyComponent>(ent)) {
		auto& hierarchy = 
			world.registry.getComponent<APE::HierarchyComponent>(ent);

		char buf[128];
		strncpy(buf, hierarchy.tag.c_str(), sizeof(buf));
		if (ImGui::InputText("Entity Tag", buf, sizeof(buf))) {
			hierarchy.tag = buf;
		}

		std::string children = 
			std::format("Num Children: {}", hierarchy.children.size());
		ImGui::Text(children.c_str());
	}

	// Transform
	if (world.registry.hasComponent<APE::TransformComponent>(ent)) {
		auto& transform = 
			world.registry.getComponent<APE::TransformComponent>(ent);

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
		transform = APE::TransformComponent::fromMatrix(matrix);
	}

	// Material
	if (world.registry.hasComponent<APE::Render::MaterialComponent>(ent)) {
		if (ImGui::Button("Change Texture")) {
			std::filesystem::path tex_path;
			auto status = APE::Files::openDialog(tex_path);
			if (status == APE::Files::Status::Sucess) {
				auto tex_handle = APE::ImageLoader::load(tex_path);
				world.registry.replaceComponent<
					APE::Render::MaterialComponent>(
						ent,
						tex_handle
					);
			}
		}
	}

	ImGui::End();
}

static inline void drawGizmo(ImGuizmo::OPERATION gizmo_op) noexcept {
	auto& world = AppRunner::getWorld();
	auto ent = AppRunner::getSelectedEntity();
	if (world.registry.hasAllComponents<
		APE::TransformComponent, APE::HierarchyComponent>(ent)) 
	{
		auto [transform, hierarchy] = world.registry.getComponents<
			APE::TransformComponent, APE::HierarchyComponent>(ent);

		auto parent_world_mat = world.getModelMatrix(hierarchy.parent);
		auto world_mat = parent_world_mat * transform.getModelMatrix();
		auto renderer = AppRunner::getRenderer();
		renderer->drawGizmo(AppRunner::getCamera(), world_mat, gizmo_op);

		auto new_loc_mat = glm::inverse(parent_world_mat) * world_mat;
		auto new_transform = APE::TransformComponent::fromMatrix(new_loc_mat);

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
