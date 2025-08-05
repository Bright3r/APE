#include "render/Renderer.h"
#include "render/SafeGPU.h"
#include "util/Logger.h"

#include <SDL3/SDL_gpu.h>
#include <utility>

namespace APE {
namespace Render {

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam)
	: m_context(context)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_cam(cam)
	, m_render_pass(nullptr)
	, m_cmd_buf(nullptr)
	, m_is_drawing(false)
{
	APE_CHECK((cam != nullptr),
		"Renderer::Renderer(std::shared_ptr<Context> context, \
		Camera *cam) Failed: cam == nullptr"
	);

	// Construct default shader
	std::unique_ptr<Shader> shader = std::make_unique<Shader>(context->device);
	useShader(shader.get());
}

Renderer::Renderer(std::shared_ptr<Context> context, Camera *cam, Shader* shader)
	: m_context(context)
	, m_wireframe_mode(false) 
	, m_clear_color(SDL_FColor { 0.f, 255.f, 255.f, 1.f })
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_cam(cam)
	, m_render_pass(nullptr)
	, m_cmd_buf(nullptr)
	, m_is_drawing(false)
{
	APE_CHECK((cam != nullptr),
		"Renderer::Renderer(std::shared_ptr<Context> context, \
		Camera *cam, Shader* shader) Failed: cam == nullptr"
	);

	useShader(shader);
}

std::unique_ptr<Shader> Renderer::createShader(ShaderDescription shader_desc) const
{
	return std::make_unique<Shader>(shader_desc, m_context->device);
}

SafeGPU::UniqueGPUGraphicsPipeline Renderer::createPipeline(
	const SDL_GPUGraphicsPipelineCreateInfo& create_info) const
{
	// Create pipeline
	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(
		m_context->device, 
		&create_info
	);

	// Check that pipeline was created
	APE_CHECK((pipeline != nullptr),
		"SDL_CreateGPUGraphicsPipeline Failed - {}",
		SDL_GetError()
	);

	// Return wrapped pipeline for memory safety
	return SafeGPU::makeUnique<SDL_GPUGraphicsPipeline>(
		pipeline,
		[=](SDL_GPUGraphicsPipeline* pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(m_context->device, pipeline);
		}
	);
}

void Renderer::useShader(Shader* shader) {
	if (!shader) {
		APE_ERROR(
			"Renderer::useShader Failed: shader == nullptr"
		);
		return;
	}

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

	SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
		.vertex_shader = shader->getVertexShader(),
		.fragment_shader = shader->getFragmentShader(),
		.vertex_input_state = vertex_input_state,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.target_info = {
			.color_target_descriptions = color_target_description,
			.num_color_targets = 1,
		},
	};
	pipeline_create_info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_BACK;

	// Create fill pipeline
	pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	m_fill_pipeline = createPipeline(pipeline_create_info);

	// Create wireframe pipeline
	pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	m_line_pipeline = createPipeline(pipeline_create_info);
}

float Renderer::getAspectRatio() const
{
	return m_context->window_width / static_cast<float>(m_context->window_height);
}

void Renderer::beginDrawing()
{
	// Check that we are not already drawing
	APE_CHECK(!m_is_drawing,
	   "Renderer::beginDrawing() Failed: endDrawing() not called from \
	   previous beginDrawing() call"
	);
	m_is_drawing = true;

	// Acquire cmd buffer
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	APE_CHECK((cmd_buffer != nullptr),
		"SDL_AcquiredGPUCommandBuffer Failed - {}",
		SDL_GetError()
	);
	m_cmd_buf = cmd_buffer;

	// Acquire swapchain texture
	SDL_GPUTexture *swapchain_texture;
	bool succ_acquire_swapchain = SDL_WaitAndAcquireGPUSwapchainTexture(
		cmd_buffer, m_context->window, &swapchain_texture, NULL, NULL
	);
	APE_CHECK(succ_acquire_swapchain,
		"SDL_WaitAndAcquireGPUSwapchainTexture Failed - {}",
		SDL_GetError()
	);

	// Setup render pass
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
	m_render_pass = render_pass;

	// Bind render pipeline
	SDL_GPUGraphicsPipeline* render_pipeline = 
		m_wireframe_mode ? m_line_pipeline.get() : m_fill_pipeline.get();
	APE_CHECK((render_pipeline != nullptr),
		"Renderer::draw Failed: render_pipeline == nullptr"
	);
	SDL_BindGPUGraphicsPipeline(render_pass, render_pipeline);
}

void Renderer::endDrawing()
{
	// Check that we are already drawing
	APE_CHECK(m_is_drawing,
	   "Renderer::endDrawing() Failed: beginDrawing() not yet called"
	);
	m_is_drawing = false;

	// Cleanup render pass
	SDL_EndGPURenderPass(m_render_pass);
	m_render_pass = nullptr;

	// Draw scene
	SDL_SubmitGPUCommandBuffer(m_cmd_buf);
}

void Renderer::draw(Model& model)
{
	// Check that we are already drawing
	APE_CHECK(m_is_drawing,
		"Renderer::draw(Model& mesh) Failed: beginDrawing() not called"
	);

	glm::mat4 model_mat = model.getTransform().getModelMatrix();
	for (Mesh& mesh : model.getMeshes()) {
		draw(mesh, model_mat);
	}
}

void Renderer::draw(Mesh& mesh, const glm::mat4& model_mat)
{
	// Check that we are already drawing
	APE_CHECK(m_is_drawing,
		"Renderer::draw(Mesh& mesh) Failed: beginDrawing() not called"
	);

	// Check if gpu vertex buffer was already created
	if (!mesh.getVertexBuffer()) {
		// Create GPU buffer with vertex data
		SDL_GPUBuffer* vertex_buffer = uploadBuffer<PositionColorVertex>(
			mesh.getVertices(),
			SDL_GPU_BUFFERUSAGE_VERTEX
		);

		// Give mesh gpu buffer wrapped with deleter
		auto safe_vertex_buffer = SafeGPU::makeUnique<SDL_GPUBuffer>(
			vertex_buffer,
			[=](SDL_GPUBuffer* buf) {
				SDL_ReleaseGPUBuffer(m_context->device, buf);
			}
		);
		mesh.setVertexBuffer(std::move(safe_vertex_buffer));
	}

	// Bind vertex buffer
	SDL_GPUBufferBinding vertex_buffer_binding = {
		.buffer = mesh.getVertexBuffer(),
		.offset = 0,
	};
	SDL_BindGPUVertexBuffers(
		m_render_pass,
		0,
		&vertex_buffer_binding,
		1
	);

	// Check if gpu index buffer was already created
	if (!mesh.getIndexBuffer()) {
		// Create GPU buffer with index data
		SDL_GPUBuffer* index_buffer = uploadBuffer<VertexIndex>(
			mesh.getIndices(),
			SDL_GPU_BUFFERUSAGE_INDEX
		);

		// Give mesh gpu buffer wrapped with deleter
		auto safe_index_buffer = SafeGPU::makeUnique<SDL_GPUBuffer>(
			index_buffer,
			[=](SDL_GPUBuffer* buf) {
				SDL_ReleaseGPUBuffer(m_context->device, buf);
			}
		);
		mesh.setIndexBuffer(std::move(safe_index_buffer));
	}

	// Bind index buffer
	SDL_GPUBufferBinding index_buffer_binding = {
		.buffer = mesh.getIndexBuffer(),
		.offset = 0,
	};
	SDL_BindGPUIndexBuffer(
		m_render_pass,
		&index_buffer_binding,
		SDL_GPU_INDEXELEMENTSIZE_16BIT
	);


	// Bind MVP matrix uniform
	ModelViewProjUniform mvp_uniform { 
		glm::transpose(model_mat),
		glm::transpose(m_cam->getViewMatrix()), 
		glm::transpose(m_cam->getProjectionMatrix(getAspectRatio()))
	};
	SDL_PushGPUVertexUniformData(
		m_cmd_buf,
		0,
		&mvp_uniform,
		sizeof(mvp_uniform)
	);

	// Draw mesh
	SDL_DrawGPUIndexedPrimitives(
		m_render_pass, 
		mesh.getIndices().size(), 
		1, 0, 0, 0
	);
}

}; // end of namespace Render
}; // end of namespace APE
