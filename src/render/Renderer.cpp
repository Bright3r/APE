#include "render/Renderer.h"
#include "render/Shader.h"
#include <SDL3/SDL_gpu.h>
#include <cassert>
#include <functional>
#include <iostream>
#include <memory>

namespace APE {
namespace Render {

Renderer::Renderer(std::shared_ptr<Context> context)
	: context(context),
	fill_pipeline(nullptr),
	line_pipeline(nullptr),
	wireframe_mode(false), 
	clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	// Construct default shader
	std::unique_ptr<Shader> shader = createShader(default_shader_desc);

	useShader(shader.get());
}

Renderer::Renderer(std::shared_ptr<Context> context, Shader* shader)
	: context(context), 
	fill_pipeline(nullptr),
	line_pipeline(nullptr),
	wireframe_mode(false), 
	clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	useShader(shader);
}

Renderer::~Renderer()
{
	if (fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, fill_pipeline);

	if (line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, line_pipeline);
}

std::unique_ptr<Shader> Renderer::createShader(ShaderDescription shader_desc) const
{
	return std::make_unique<Shader>(shader_desc, context->device);
}

void Renderer::useShader(Shader* shader) {
	if (!shader) {
		std::cerr << "useShader Failed: Cannot apply null shader\n";
		return;
	}

	if (fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, fill_pipeline);

	if (line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, line_pipeline);

	SDL_GPUColorTargetDescription color_target_description[] = {{
		.format = SDL_GetGPUSwapchainTextureFormat(
			context->device, 
			context->window
		),
		.blend_state = {},
	}};

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.vertex_shader = shader->vert_shader,
		.fragment_shader = shader->frag_shader,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = color_target_description,
			.num_color_targets = 1,
		},
	};

	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	fill_pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipelineCreateInfo);
	if (!fill_pipeline) {
		std::cerr << "SDL_CreateGPUGraphicsPipeline Failed \
			for fill_pipeline: " << SDL_GetError() << std::endl;
		return;
	}

	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	line_pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipelineCreateInfo);
	if (!line_pipeline) {
		std::cerr << "SDL_CreateGPUGraphicsPipeline Failed \
			for line_pipeline: " << SDL_GetError() << std::endl;
		return;
	}
}

void Renderer::draw(std::function<void(SDL_GPURenderPass*)> draw_scene)
{
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(context->device);
	if (!cmd_buffer) {
		std::cerr << "SDL_AcquireGPUCommandBuffer Failed: "
			<< SDL_GetError() << std::endl;
		return;
	}
	
	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
		cmd_buffer, context->window, &swapchain_texture, NULL, NULL)) 
	{
		std::cerr << "SDL_WaitAndAcquireGPUSwapchainTexture Failed: "
			<< SDL_GetError() << std::endl;
		return;
	}

	if (swapchain_texture) {
		SDL_GPUColorTargetInfo color_target_info = {
			.texture = swapchain_texture,
			.clear_color = clear_color,
			.load_op = SDL_GPU_LOADOP_CLEAR,
			.store_op = SDL_GPU_STOREOP_STORE,
		};

		SDL_GPURenderPass *render_pass = 
			SDL_BeginGPURenderPass(cmd_buffer, &color_target_info, 1, NULL);

		SDL_GPUGraphicsPipeline *render_pipeline = 
			wireframe_mode ? line_pipeline : fill_pipeline;
		if (!render_pipeline) {
			std::cerr << "No render pipeline available in draw()." << "\n";
			return;
		}

		SDL_BindGPUGraphicsPipeline(render_pass, render_pipeline);

		// SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);
		draw_scene(render_pass);

		SDL_EndGPURenderPass(render_pass);
	}

	SDL_SubmitGPUCommandBuffer(cmd_buffer);
}

}; // end of namespace Render
}; // end of namespace APE
