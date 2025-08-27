#pragma once

#include "render/Context.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

namespace APE::Render {

struct ImGuiSession {

	ImGuiSession(Context* context) noexcept
	{
		// Setup context
		ImGui::CreateContext();
		ImGuiIO io = ImGui::GetIO(); 
		(void) io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		// Configure styles
		ImGui::StyleColorsDark();

		// Setup backend
		ImGui_ImplSDL3_InitForSDLGPU(context->window);
		ImGui_ImplSDLGPU3_InitInfo init_info = {
			.Device = context->device,
			.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(
				context->device,
				context->window
			),
			.MSAASamples = SDL_GPU_SAMPLECOUNT_1,
		};
		ImGui_ImplSDLGPU3_Init(&init_info);
	}

	~ImGuiSession() noexcept
	{
		ImGui_ImplSDLGPU3_Shutdown();
		ImGui_ImplSDL3_Shutdown();
		ImGui::DestroyContext();
	}
};

};	// end of namespace

