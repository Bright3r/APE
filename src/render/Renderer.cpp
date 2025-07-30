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
	vertex_buffer(nullptr),
	wireframe_mode(false), 
	clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	// Construct default shader
	std::unique_ptr<Shader> shader = createShader(default_shader_desc);

	useShader(shader.get());

	std::vector<PositionColorVertex> vertex_data = {
		{    -1,    -1, 0, 255,   0,   0, 255 },
		{     1,    -1, 0,   0, 255,   0, 255 },
		{     0,     1, 0,   0,   0, 255, 255 }
	};
	useVertexData(vertex_data);
}

Renderer::Renderer(std::shared_ptr<Context> context, Shader* shader)
	: context(context), 
	fill_pipeline(nullptr),
	line_pipeline(nullptr),
	vertex_buffer(nullptr),
	wireframe_mode(false), 
	clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	useShader(shader);

	std::vector<PositionColorVertex> vertex_data = {
		{    -1,    -1, 0, 255,   0,   0, 255 },
		{     1,    -1, 0,   0, 255,   0, 255 },
		{     0,     1, 0,   0,   0, 255, 255 }
	};
	useVertexData(vertex_data);
}

Renderer::~Renderer()
{
	if (fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, fill_pipeline);

	if (line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, line_pipeline);

	if (vertex_buffer)
		SDL_ReleaseGPUBuffer(context->device, vertex_buffer);
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

	// Destroy previous pipelines
	if (fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, fill_pipeline);

	if (line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(context->device, line_pipeline);

	// Setup new pipeline config
	SDL_GPUVertexBufferDescription vertex_buffer_description[] = {{
			.slot = 0,
			.pitch = sizeof(PositionColorVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0,
	}};

	SDL_GPUVertexAttribute vertex_attributes[] = {{
		.location = 0,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
		.offset = 0,
	}, {
		.location = 1,
		.buffer_slot = 0,
		.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
		.offset = sizeof(float) * 3,
	}};

	SDL_GPUVertexInputState vertex_input_state = {
		.vertex_buffer_descriptions = vertex_buffer_description,
		.num_vertex_buffers = 1,
		.vertex_attributes = vertex_attributes,
		.num_vertex_attributes = 2,
	};

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
		.vertex_input_state = vertex_input_state,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = color_target_description,
			.num_color_targets = 1,
		},
	};

	// Create fill pipeline
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	fill_pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipelineCreateInfo);
	if (!fill_pipeline) {
		std::cerr << "SDL_CreateGPUGraphicsPipeline Failed \
			for fill_pipeline: " << SDL_GetError() << std::endl;
		return;
	}

	// Create wireframe pipeline
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	line_pipeline = SDL_CreateGPUGraphicsPipeline(context->device, &pipelineCreateInfo);
	if (!line_pipeline) {
		std::cerr << "SDL_CreateGPUGraphicsPipeline Failed \
			for line_pipeline: " << SDL_GetError() << std::endl;
		return;
	}
}

void Renderer::useVertexData(std::vector<PositionColorVertex> vertex_data)
{
	// Create a vertex buffer
	Uint32 buffer_size = static_cast<Uint32>(
			sizeof(PositionColorVertex) * vertex_data.size()
	);

	SDL_GPUBufferCreateInfo buffer_create_info = {
		.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
		.size = buffer_size,
	};

	vertex_buffer = SDL_CreateGPUBuffer(
		context->device,
		&buffer_create_info
	);

	// Create a transfer buffer
	SDL_GPUTransferBufferCreateInfo transfer_buffer_create_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = buffer_size,
	};

	SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
		context->device,
		&transfer_buffer_create_info
	);

	PositionColorVertex *transfer_data = static_cast<PositionColorVertex*>(
		SDL_MapGPUTransferBuffer(
			context->device,
			transfer_buffer,
			false
		)
   	);

	// Use transfer buffer to submit data to vertex buffer
	for (int i = 0; i < vertex_data.size(); ++i) {
		transfer_data[i] = vertex_data[i];
	}

	SDL_UnmapGPUTransferBuffer(context->device, transfer_buffer);

	// Upload transfer data to vertex buffer
	SDL_GPUCommandBuffer *upload_cmd_buffer = SDL_AcquireGPUCommandBuffer(
		context->device
	);
	SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(upload_cmd_buffer);

	SDL_GPUTransferBufferLocation transfer_buffer_loc = {
		.transfer_buffer = transfer_buffer,
		.offset = 0,
	};

	SDL_GPUBufferRegion buffer_region = {
		.buffer = vertex_buffer,
		.offset = 0,
		.size = buffer_size,
	};

	SDL_UploadToGPUBuffer(
		copy_pass,
		&transfer_buffer_loc,
		&buffer_region,
		false
	);

	// Clean up resources
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(upload_cmd_buffer);
	SDL_ReleaseGPUTransferBuffer(context->device, transfer_buffer);
}

void Renderer::draw(std::function<void(SDL_GPURenderPass*)> draw_scene)
{
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(
		context->device
	);
	if (!cmd_buffer) {
		std::cerr << "SDL_AcquireGPUCommandBuffer Failed: "
			<< SDL_GetError() << std::endl;
		return;
	}
	
	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
		cmd_buffer, context->window, &swapchain_texture, NULL, NULL
	)) {
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

		SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(
				cmd_buffer, 
				&color_target_info, 
				1, 
				NULL
		);

		SDL_GPUGraphicsPipeline *render_pipeline = 
			wireframe_mode ? line_pipeline : fill_pipeline;
		if (!render_pipeline) {
			std::cerr << "No render pipeline available in draw()." 
				<< "\n";
			return;
		}

		SDL_BindGPUGraphicsPipeline(render_pass, render_pipeline);

		// draw_scene(render_pass);
		SDL_GPUBufferBinding buffer_binding = {
			.buffer = vertex_buffer,
			.offset = 0,
		};
		SDL_BindGPUVertexBuffers(
			render_pass,
			0,
			&buffer_binding,
			1
		);
		SDL_DrawGPUPrimitives(render_pass, 3, 1, 0, 0);

		SDL_EndGPURenderPass(render_pass);
	}

	SDL_SubmitGPUCommandBuffer(cmd_buffer);
}

}; // end of namespace Render
}; // end of namespace APE
