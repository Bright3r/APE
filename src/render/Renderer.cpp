#include "render/Renderer.h"
#include "render/Shader.h"
#include "util/Logger.h"

#include <SDL3/SDL_gpu.h>
#include <functional>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

namespace APE {
namespace Render {

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam)
	: m_context(context)
	, m_cam(cam)
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_vertex_buffer(nullptr)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	if (!cam) {
		APE_FATAL(
			"Renderer::Renderer(std::shared_ptr<Context> context, \
			Camera *cam) Failed: cam == nullptr"
		);
		return;
	}

	// Construct default shader
	std::unique_ptr<Shader> shader = createShader(default_shader_desc);
	useShader(shader.get());

	useVertexData(vertex_data);
}

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam, Shader* shader)
	: m_context(context)
	, m_cam(cam)
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_vertex_buffer(nullptr)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
{
	if (!cam) {
		APE_FATAL(
			"Renderer::Renderer(std::shared_ptr<Context> context, \
			Camera *cam, Shader* shader) Failed: cam == nullptr"
		);
		return;
	}

	useShader(shader);

	useVertexData(vertex_data);
}

Renderer::~Renderer()
{
	if (m_fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_fill_pipeline);

	if (m_line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_line_pipeline);

	if (m_vertex_buffer)
		SDL_ReleaseGPUBuffer(m_context->device, m_vertex_buffer);
}

std::unique_ptr<Shader> Renderer::createShader(ShaderDescription shader_desc) const
{
	return std::make_unique<Shader>(shader_desc, m_context->device);
}

void Renderer::useShader(Shader* shader) {
	if (!shader) {
		APE_ERROR(
			"Renderer::useShader Failed: shader == nullptr"
		);
		return;
	}

	// Destroy previous pipelines
	if (m_fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_fill_pipeline);

	if (m_line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_line_pipeline);

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
			m_context->device, 
			m_context->window
		),
		.blend_state = {},
	}};

	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.vertex_shader = shader->getVertexShader(),
		.fragment_shader = shader->getFragmentShader(),
		.vertex_input_state = vertex_input_state,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = color_target_description,
			.num_color_targets = 1,
		},
	};

	// Create fill pipeline
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	m_fill_pipeline = SDL_CreateGPUGraphicsPipeline(m_context->device, &pipelineCreateInfo);
	if (!m_fill_pipeline) {
		APE_FATAL(
			"SDL_CreateGPUGraphicsPipeline Failed on fill_pipeline - {}",
			SDL_GetError()
		);
		return;
	}

	// Create wireframe pipeline
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	m_line_pipeline = SDL_CreateGPUGraphicsPipeline(m_context->device, &pipelineCreateInfo);
	if (!m_line_pipeline) {
		APE_FATAL(
			"SDL_CreateGPUGraphicsPipeline Failed on line_pipeline - {}",
			SDL_GetError()
		);
		return;
	}
}

void Renderer::useVertexData(std::vector<PositionColorVertex> vertex_data)
{
	m_vertex_buffer = uploadBuffer<PositionColorVertex>(
		vertex_data, 
		SDL_GPU_BUFFERUSAGE_VERTEX
	);
}

float Renderer::getAspectRatio() const
{
	return m_context->window_width / static_cast<float>(m_context->window_height);
}

void Renderer::draw(std::function<void(SDL_GPURenderPass*)> draw_scene)
{
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	if (!cmd_buffer) {
		APE_ERROR(
			"SDL_AcquiredGPUCommandBuffer Failed - {}",
			SDL_GetError()
		);
		return;
	}

	SDL_GPUTexture *swapchain_texture;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(
		cmd_buffer, m_context->window, &swapchain_texture, NULL, NULL
	)) {
		APE_ERROR(
			"SDL_WaitAndAcquireGPUSwapchainTexture Failed - {}",
			SDL_GetError()
		);
		return;
	}

	if (swapchain_texture) {
		SDL_GPUColorTargetInfo color_target_info = {
			.texture = swapchain_texture,
			.clear_color = m_clear_color,
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
			m_wireframe_mode ? m_line_pipeline : m_fill_pipeline;
		if (!render_pipeline) {
			APE_FATAL(
				"Renderer::draw Failed: render_pipeline == nullptr"
			);
			return;
		}

		SDL_BindGPUGraphicsPipeline(render_pass, render_pipeline);

		/* Push Uniform Buffers */
		// Bind camera uniform
		glm::mat4 model = glm::translate(
			glm::mat4(1.0f), 
			glm::vec3(0.f, 0.f, 0.0f)
		);
		ModelViewProjUniform mvp_uniform { 
			glm::transpose(model),		
			glm::transpose(m_cam->getViewMatrix()), 
			glm::transpose(m_cam->getProjectionMatrix(getAspectRatio()))
		};
		SDL_PushGPUVertexUniformData(
			cmd_buffer,
			0,
			&mvp_uniform,
			sizeof(mvp_uniform)
		);

		// Bind vertex buffers
		SDL_GPUBufferBinding buffer_binding = {
			.buffer = m_vertex_buffer,
			.offset = 0,
		};
		SDL_BindGPUVertexBuffers(
			render_pass,
			0,
			&buffer_binding,
			1
		);

		// Draw Scene
		SDL_DrawGPUPrimitives(render_pass, vertex_data.size(), 1, 0, 0);

		// Cleanup
		SDL_EndGPURenderPass(render_pass);
	}

	SDL_SubmitGPUCommandBuffer(cmd_buffer);
}

}; // end of namespace Render
}; // end of namespace APE
