#include "render/Renderer.h"
#include "render/SafeGPU.h"
#include "render/Vertex.h"
#include "util/Logger.h"

#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include "ImGuizmo.h"

#include <utility>

namespace APE::Render {

Renderer::Renderer(std::shared_ptr<Context> context) noexcept
	: m_context(context)
	, wireframe_mode(false) 
	, clear_color(SDL_FColor { 0.f, 1.f, 1.f, 1.f })
	, m_shader(nullptr)
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_swapchain_texture(nullptr)
	, m_render_pass(nullptr)
	, m_cmd_buf(nullptr)
	, m_is_drawing(false)
	, debug_mode(false)
	, light({})
	, m_imgui_session(nullptr)
{
	// Construct default shader
	m_shader = std::make_shared<Shader>(
		default_vert_shader_desc,
		default_frag_shader_desc,
		context->device
	);

	reset();
}

Renderer::Renderer(std::shared_ptr<Context> context, 
		   std::shared_ptr<Shader> shader) noexcept
	: m_context(context)
	, wireframe_mode(false) 
	, clear_color(SDL_FColor { 0.f, 1.f, 1.f, 1.f })
	, m_shader(shader)
	, m_fill_pipeline(nullptr)
	, m_line_pipeline(nullptr)
	, m_swapchain_texture(nullptr)
	, m_render_pass(nullptr)
	, m_cmd_buf(nullptr)
	, m_is_drawing(false)
	, debug_mode(false)
	, light({})
	, m_imgui_session(nullptr)
{
	reset();
}

void Renderer::reset() noexcept
{
	useShader(m_shader.get());
	createSampler();
	createDepthTexture();

	// Ensure previous imgui session is destroyed before creating a new one
	m_imgui_session = nullptr;
	m_imgui_session = std::make_unique<ImGuiSession>(m_context.get());

	ImGuizmo::Enable(true);
}

std::unique_ptr<Shader> Renderer::createShader(
	const ShaderDescription& vert_shader_desc,
	const ShaderDescription& frag_shader_desc) const noexcept
{
	return std::make_unique<Shader>(
		vert_shader_desc,
		frag_shader_desc,
		m_context->device
	);
}

SafeGPU::UniqueGPUGraphicsPipeline Renderer::createPipeline(
	const SDL_GPUGraphicsPipelineCreateInfo& create_info) const noexcept
{
	SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(
		m_context->device, 
		&create_info
	);
	APE_CHECK((pipeline != nullptr),
		"SDL_CreateGPUGraphicsPipeline Failed - {}",
		SDL_GetError()
	);

	return SafeGPU::makeUnique<SDL_GPUGraphicsPipeline>(
		pipeline,
		[=, this](SDL_GPUGraphicsPipeline* pipeline) {
			SDL_ReleaseGPUGraphicsPipeline(m_context->device, pipeline);
		}
	);
}

void Renderer::useShader(Shader* shader) noexcept {
	if (!shader) {
		APE_ERROR(
			"Renderer::useShader Failed: shader == nullptr"
		);
		return;
	}

	std::vector<SDL_GPUColorTargetDescription> color_target_descriptions = {{
		.format = SDL_GetGPUSwapchainTextureFormat(
			m_context->device, 
			m_context->window
		),
		.blend_state = {},
	}};

	SDL_GPURasterizerState rasterizer_state = {
		.cull_mode = SDL_GPU_CULLMODE_BACK,
		.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
	};

	SDL_GPUDepthStencilState depth_stencil_state = {
		.compare_op = SDL_GPU_COMPAREOP_LESS,
		.write_mask = 0xff,
		.enable_depth_test = true,
		.enable_depth_write = true,
		.enable_stencil_test = false,
	};
	
	SDL_GPUGraphicsPipelineTargetInfo target_info = {
		.color_target_descriptions = color_target_descriptions.data(),
		.num_color_targets =
			static_cast<Uint32>(color_target_descriptions.size()),
		.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.has_depth_stencil_target = true,
	};

	// Vertex layout
	VertexFormat vertex_format = shader->getVertexFormat();
	SDL_GPUVertexInputState vertex_input_state = vertex_format.getInputState();

	SDL_GPUGraphicsPipelineCreateInfo pipeline_create_info = {
		.vertex_shader = shader->getVertexShader(),
		.fragment_shader = shader->getFragmentShader(),
		.vertex_input_state = vertex_input_state,
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.rasterizer_state = rasterizer_state,
		.depth_stencil_state = depth_stencil_state,
		.target_info = target_info
	};

	// Create fill pipeline
	pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	m_fill_pipeline = createPipeline(pipeline_create_info);

	// Create wireframe pipeline
	pipeline_create_info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;
	m_line_pipeline = createPipeline(pipeline_create_info);
}

void Renderer::createDepthTexture() noexcept
{
	SDL_GPUTextureCreateInfo tex_desc = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | 
			SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
		.width = static_cast<Uint32>(m_context->window_width),
		.height = static_cast<Uint32>(m_context->window_height),
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.sample_count = SDL_GPU_SAMPLECOUNT_1,
	};

	SDL_GPUTexture* depth_tex = SDL_CreateGPUTexture(
		m_context->device, 
		&tex_desc
	);
	m_depth_texture = SafeGPU::makeUnique<SDL_GPUTexture>(
		depth_tex,
		[=, this](SDL_GPUTexture* tex) {
			SDL_ReleaseGPUTexture(m_context->device, tex);
		}
	);
}

float Renderer::getAspectRatio() const noexcept
{
	return m_context->window_width / static_cast<float>(m_context->window_height);
}

void Renderer::beginRenderPass(bool b_clear, bool b_depth) noexcept
{
	APE_CHECK((m_cmd_buf != nullptr),
		"Renderer::beginRenderPass Failed: command_buffer == null"
	);

	APE_CHECK((m_swapchain_texture != nullptr),
		"Renderer::beginRenderPass Failed: swapchain_texture == null"
	);

	SDL_GPUColorTargetInfo color_target_info = {
		.texture = m_swapchain_texture,
		.clear_color = clear_color,
		.load_op = b_clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
	};

	SDL_GPUDepthStencilTargetInfo depth_target_info = {
		.texture = m_depth_texture.get(),
		.clear_depth = 1,
		.load_op = b_clear ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD,
		.store_op = SDL_GPU_STOREOP_STORE,
		.stencil_load_op = SDL_GPU_LOADOP_CLEAR,
		.stencil_store_op = SDL_GPU_STOREOP_STORE,
		.cycle = true,
		.clear_stencil = 0,
	};
	m_render_pass = SDL_BeginGPURenderPass(
		m_cmd_buf, 
		&color_target_info, 
		1, 
		(b_depth ? &depth_target_info : nullptr)
	);
}

void Renderer::beginDrawing() noexcept
{
	// Check that we are not already drawing
	APE_CHECK(!m_is_drawing,
	   "Renderer::beginDrawing() Failed: endDrawing() not called from previous beginDrawing() call"
	);
	m_is_drawing = true;

	// Acquire cmd buffer
	m_cmd_buf = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	APE_CHECK((m_cmd_buf != nullptr),
		"SDL_AcquiredGPUCommandBuffer Failed - {}",
		SDL_GetError()
	);

	// Acquire swapchain texture
	bool succ_acquire_swapchain = SDL_WaitAndAcquireGPUSwapchainTexture(
		m_cmd_buf,
		m_context->window,
		&m_swapchain_texture,
		NULL,
		NULL
	);
	APE_CHECK(succ_acquire_swapchain,
		"SDL_WaitAndAcquireGPUSwapchainTexture Failed - {}",
		SDL_GetError()
	);

	// Start ImGUI frame
	ImGui_ImplSDLGPU3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	

	// Setup render pass
	beginRenderPass(true, true);

	// Bind render pipeline
	SDL_GPUGraphicsPipeline* render_pipeline = 
		wireframe_mode ? m_line_pipeline.get() : m_fill_pipeline.get();

	APE_CHECK((render_pipeline != nullptr),
		"Renderer::draw Failed: render_pipeline == nullptr"
	);
	SDL_BindGPUGraphicsPipeline(m_render_pass, render_pipeline);
}

void Renderer::draw(MeshComponent& mesh,
		MaterialComponent& material,
		std::weak_ptr<Camera> camera,
		const glm::mat4& model_matrix) noexcept
{
	// Check that we are already drawing
	APE_CHECK(m_is_drawing,
		"Renderer::draw(Model& mesh) Failed: beginDrawing() not called"
	);

	// Check that camera is valid
	APE_CHECK(!camera.expired(),
		"Renderer::draw() Failed: camera does not exist."
	);
	auto cam = camera.lock();

	// Check if gpu vertex buffer was already created
	auto& raw_mesh = mesh.model_handle.data->meshes[mesh.mesh_index];
	if (!raw_mesh.vertex_buffer) {
		// Create GPU buffer with vertex data
		SafeGPU::UniqueGPUBuffer vertex_buffer = uploadBuffer(
			vectorToRawBytes(raw_mesh.vertices),
			SDL_GPU_BUFFERUSAGE_VERTEX
		);

		raw_mesh.vertex_buffer = std::move(vertex_buffer);
	}

	// Bind vertex buffer
	SDL_GPUBufferBinding vertex_buffer_binding = {
		.buffer = raw_mesh.vertex_buffer.get(),
		.offset = 0,
	};
	SDL_BindGPUVertexBuffers(
		m_render_pass,
		0,
		&vertex_buffer_binding,
		1
	);


	// Check if gpu index buffer was already created
	if (!raw_mesh.index_buffer) {
		// Create GPU buffer with index data
		SafeGPU::UniqueGPUBuffer index_buffer = uploadBuffer(
			vectorToRawBytes(raw_mesh.indices),
			SDL_GPU_BUFFERUSAGE_INDEX
		);

		raw_mesh.index_buffer = std::move(index_buffer);
	}

	// Bind index buffer
	SDL_GPUBufferBinding index_buffer_binding = {
		.buffer = raw_mesh.index_buffer.get(),
		.offset = 0,
	};
	SDL_BindGPUIndexBuffer(
		m_render_pass,
		&index_buffer_binding,
		SDL_GPU_INDEXELEMENTSIZE_32BIT
	);


	// Check if mesh texture was uploaded yet
	auto& texture = material.texture_handle.data;
	if (!texture->textureBuffer()) {
		// Create GPU Texture
		SafeGPU::UniqueGPUTexture gpu_tex = createTexture(
			texture.get()
		);
		texture->textureBuffer() = std::move(gpu_tex);
	}

	// Bind texture sampler
	std::vector<SDL_GPUTextureSamplerBinding> sampler_bindings = {{
		.texture = texture->textureBuffer().get(),
		.sampler = m_sampler.get(),
	}};
	SDL_BindGPUFragmentSamplers(
		m_render_pass,
		0,
		sampler_bindings.data(),
		sampler_bindings.size()
	);


	// Bind Camera Uniform
	CameraUniform cam_uniform {
		.position = glm::vec4(cam->getPosition(), 0.f),
	};
	SDL_PushGPUVertexUniformData(
		m_cmd_buf,
		0,
		&cam_uniform,
		sizeof(cam_uniform)
	);

	// Bind MVP matrix uniform
	ModelViewProjUniform mvp_uniform { 
		glm::transpose(model_matrix),
		glm::transpose(cam->getViewMatrix()), 
		glm::transpose(cam->getProjectionMatrix(getAspectRatio()))
	};
	SDL_PushGPUVertexUniformData(
		m_cmd_buf,
		1,
		&mvp_uniform,
		sizeof(mvp_uniform)
	);


	// Bind DebugMode Uniform
	SDL_PushGPUFragmentUniformData(
		m_cmd_buf,
		0,
		&debug_mode,
		sizeof(debug_mode)
	);

	// Bind Light Uniform
	SDL_PushGPUFragmentUniformData(
		m_cmd_buf,
		1,
		&light,
		sizeof(light)
	);




	// Draw mesh
	SDL_DrawGPUIndexedPrimitives(
		m_render_pass, 
		raw_mesh.indices.size(), 
		1, 0, 0, 0
	);
}

void Renderer::drawGizmo(std::weak_ptr<Camera> camera,
	glm::mat4& matrix,
	ImGuizmo::OPERATION gizmo_op,
	ImGuizmo::MODE gizmo_mode) noexcept
{
	APE_CHECK(!camera.expired(),
	   "Renderer::drawGizmo() Failed: camera is nullptr."
	);
	auto cam = camera.lock();

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	auto view = cam->getViewMatrix();
	auto proj = cam->getProjectionMatrix(getAspectRatio());
	ImGuizmo::Manipulate(
		glm::value_ptr(view),
		glm::value_ptr(proj),
		gizmo_op,
		gizmo_mode,
		glm::value_ptr(matrix),
		NULL,
		NULL
	);
}

void Renderer::endDrawing() noexcept
{
	// Check that we are already drawing
	APE_CHECK(m_is_drawing,
	   "Renderer::endDrawing() Failed: beginDrawing() not yet called"
	);
	m_is_drawing = false;

	// Finish scene render pass
	SDL_EndGPURenderPass(m_render_pass);
	m_render_pass = nullptr;

	// Build ImGUI drawing data
	ImGui::Render();
	ImDrawData* gui_data = ImGui::GetDrawData();
	Imgui_ImplSDLGPU3_PrepareDrawData(gui_data, m_cmd_buf);

	// Dispatch render pass for ImGUI
	beginRenderPass(false, false);
	ImGui_ImplSDLGPU3_RenderDrawData(gui_data, m_cmd_buf, m_render_pass);
	
	// Finish ImGUI render pass
	SDL_EndGPURenderPass(m_render_pass);
	m_render_pass = nullptr;

	// Draw to swapchain texture
	SDL_SubmitGPUCommandBuffer(m_cmd_buf);
	m_cmd_buf = nullptr;
	m_swapchain_texture = nullptr;
}

SafeGPU::UniqueGPUBuffer Renderer::uploadBuffer(
	const std::vector<std::byte>& data,
	Uint32 usage) noexcept
{
	// Create GPU buffer
	Uint32 buffer_size = data.size();
	SDL_GPUBufferCreateInfo buffer_info = {
		.usage = usage,
		.size = buffer_size,
	};

	SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(
		m_context->device,
		&buffer_info
	);
	auto safe_buffer = SafeGPU::makeUnique<SDL_GPUBuffer>(
		buffer,
		[=, this](SDL_GPUBuffer* buf) {
			SDL_ReleaseGPUBuffer(m_context->device, buf);
		}
	);

	// Create transfer buffer
	SDL_GPUTransferBufferCreateInfo transfer_info = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = buffer_size,
	};

	SDL_GPUTransferBuffer *transfer_buffer = SDL_CreateGPUTransferBuffer(
		m_context->device, 
		&transfer_info
	);

	// Write data to transfer buffer
	std::byte* mapped = static_cast<std::byte*>(
		SDL_MapGPUTransferBuffer(
			m_context->device, 
			transfer_buffer, 
			false
		)
	);

	std::memcpy(mapped, data.data(), buffer_size);

	SDL_UnmapGPUTransferBuffer(m_context->device, transfer_buffer);

	// Upload transfer buffer to GPU read-only memory
	SDL_GPUCommandBuffer *cmd_buffer = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	SDL_GPUCopyPass *copy_pass = SDL_BeginGPUCopyPass(cmd_buffer);

	SDL_GPUTransferBufferLocation src = {
		.transfer_buffer = transfer_buffer,
		.offset = 0,
	};
	SDL_GPUBufferRegion dest = {
		.buffer = buffer,
		.offset = 0,
		.size = buffer_size,
	};
	SDL_UploadToGPUBuffer(copy_pass, &src, &dest, false);

	// Execute copy pass
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(cmd_buffer);

	// Cleanup resources
	SDL_ReleaseGPUTransferBuffer(m_context->device, transfer_buffer);

	return safe_buffer;
}

SDL_GPUTextureFormat Renderer::getTextureFormat(Image* image) noexcept
{
	switch (image->getNumChannels()) {
	case 1:
		return SDL_GPU_TEXTUREFORMAT_R8_UNORM;
	case 2:
		return SDL_GPU_TEXTUREFORMAT_R8G8_UNORM;
	case 4:
		return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	default:
		APE_ERROR(
			"Unsupported texture channel count: {}",
	    		image->getNumChannels()
		);
		return SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	}
}

SafeGPU::UniqueGPUTexture Renderer::createTexture(Image* image) noexcept
{
	// Create texture
	SDL_GPUTextureCreateInfo tex_desc = {
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = getTextureFormat(image),
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
		.width = image->getWidth(),
		.height = image->getHeight(),
		.layer_count_or_depth = 1,
		.num_levels = 1,
	};
	SDL_GPUTexture* texture = SDL_CreateGPUTexture(m_context->device, &tex_desc);

	// Make safe wrapper around texture
	SafeGPU::UniqueGPUTexture safe_tex = SafeGPU::makeUnique<SDL_GPUTexture>(
		texture,
		[=, this](SDL_GPUTexture* tex) {
			SDL_ReleaseGPUTexture(m_context->device, tex);
		}
	);

	// Create transfer buffer
	SDL_GPUTransferBufferCreateInfo transfer_desc = {
		.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
		.size = image->getSizeBytes(),
	};
	SDL_GPUTransferBuffer* transfer_buf = SDL_CreateGPUTransferBuffer(
		m_context->device,
		&transfer_desc
	);

	// Write data to transfer buffer
	std::byte* mapped = static_cast<std::byte*>(SDL_MapGPUTransferBuffer(
		m_context->device, 
		transfer_buf, 
		false
	));

	std::memcpy(
		mapped,
		static_cast<void*>(image->getPixels()),
		image->getSizeBytes()
	);

	SDL_UnmapGPUTransferBuffer(m_context->device, transfer_buf);

	// Upload transfer buffer to gpu
	SDL_GPUCommandBuffer* cmd_buf = SDL_AcquireGPUCommandBuffer(
		m_context->device
	);
	SDL_GPUCopyPass* copy_pass = SDL_BeginGPUCopyPass(cmd_buf);

	SDL_GPUTextureTransferInfo src = {
		.transfer_buffer = transfer_buf,
		.offset = 0,
	};
	SDL_GPUTextureRegion dest = {
		.texture = texture,
		.w = image->getWidth(),
		.h = image->getHeight(),
		.d = 1,
	};
	SDL_UploadToGPUTexture(copy_pass, &src, &dest, false);

	// Cleanup resources
	SDL_EndGPUCopyPass(copy_pass);
	SDL_SubmitGPUCommandBuffer(cmd_buf);
	SDL_ReleaseGPUTransferBuffer(m_context->device, transfer_buf);

	return safe_tex;
}

void Renderer::createSampler() noexcept
{
	SDL_GPUSamplerCreateInfo sampler_desc = {
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	};
	SDL_GPUSampler* sampler = SDL_CreateGPUSampler(
		m_context->device,
		&sampler_desc
	);

	m_sampler = SafeGPU::makeUnique<SDL_GPUSampler>(
		sampler,
		[=, this](SDL_GPUSampler* sam) {
			SDL_ReleaseGPUSampler(m_context->device, sam);
		}
	);
}

}; // end of namespace

