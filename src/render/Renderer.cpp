#include "render/Renderer.h"
#include "util/Logger.h"

namespace APE {
namespace Render {

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam)
	: m_context(context)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
	, m_cam(cam)
	, m_scene({})
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
{
	APE_CHECK((cam != nullptr),
		"Renderer::Renderer(std::shared_ptr<Context> context, \
		Camera *cam) Failed: cam == nullptr"
	);

	// Construct default shader
	std::unique_ptr<Shader> shader = createShader(default_shader_desc);
	useShader(shader.get());
}

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam, Shader* shader)
	: m_context(context)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
	, m_cam(cam)
	, m_scene({})
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
{
	APE_CHECK((cam != nullptr),
		"Renderer::Renderer(std::shared_ptr<Context> context, \
		Camera *cam, Shader* shader) Failed: cam == nullptr"
	);

	useShader(shader);
}

Renderer::~Renderer()
{
	if (m_fill_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_fill_pipeline);

	if (m_line_pipeline)
		SDL_ReleaseGPUGraphicsPipeline(m_context->device, m_line_pipeline);
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
		.offset = sizeof(glm::vec3),
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
	APE_CHECK((m_fill_pipeline != nullptr),
		"SDL_CreateGPUGraphicsPipeline Failed on fill_pipeline - {}",
		SDL_GetError()
	);

	// Create wireframe pipeline
	pipelineCreateInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	m_line_pipeline = SDL_CreateGPUGraphicsPipeline(m_context->device, &pipelineCreateInfo);
	APE_CHECK((m_line_pipeline != nullptr),
		"SDL_CreateGPUGraphicsPipeline Failed on line_pipeline - {}",
		SDL_GetError()
	);
}

float Renderer::getAspectRatio() const
{
	return m_context->window_width / static_cast<float>(m_context->window_height);
}

void Renderer::drawMesh(Mesh* mesh, SDL_GPURenderPass* render_pass)
{
	// Check if gpu buffer was already created
	if (!mesh->getVertexBuffer()) {
		// Create GPU buffer with vertex data
		SDL_GPUBuffer* vertex_buffer = uploadBuffer<PositionColorVertex>(
			mesh->getVertices(),
			SDL_GPU_BUFFERUSAGE_VERTEX
		);

		// Give mesh gpu buffer wrapped with deleter
		auto safe_vertex_buffer = SafeGPU::make_shared_gpu_buffer(
			vertex_buffer,
			[&](SDL_GPUBuffer* buf) {
				if (m_context && m_context->device) {
					SDL_ReleaseGPUBuffer(m_context->device, buf);
				}
			}
		);
		mesh->setVertexBuffer(safe_vertex_buffer);
	}

	
	// Bind vertex buffers
	SDL_GPUBufferBinding buffer_binding = {
		.buffer = mesh->getVertexBuffer().get(),
		.offset = 0,
	};
	SDL_BindGPUVertexBuffers(
		render_pass,
		0,
		&buffer_binding,
		1
	);

	// Draw Scene
	SDL_DrawGPUPrimitives(render_pass, mesh->getVertices().size(), 1, 0, 0);
}

void Renderer::draw(std::function<void(SDL_GPURenderPass*)> draw_scene)
{
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	APE_CHECK((cmd_buffer != nullptr),
		"SDL_AcquiredGPUCommandBuffer Failed - {}",
		SDL_GetError()
	);

	SDL_GPUTexture *swapchain_texture;
	bool succ_acquire_swapchain = SDL_WaitAndAcquireGPUSwapchainTexture(
		cmd_buffer, m_context->window, &swapchain_texture, NULL, NULL
	);
	APE_CHECK(succ_acquire_swapchain,
		"SDL_WaitAndAcquireGPUSwapchainTexture Failed - {}",
		SDL_GetError()
	);

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
		APE_CHECK((render_pipeline != nullptr),
			"Renderer::draw Failed: render_pipeline == nullptr"
		);

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

		draw_scene(render_pass);

		// Cleanup
		SDL_EndGPURenderPass(render_pass);
	}

	SDL_SubmitGPUCommandBuffer(cmd_buffer);
}

}; // end of namespace Render
}; // end of namespace APE
