#pragma once

#include "core/components/Render.h"
#include "core/render/Camera.h"
#include "core/render/Context.h"
#include "core/render/ImGuiSession.h"
#include "core/render/SafeGPU.h"
#include "core/render/Shader.h"
#include "core/render/Model.h"
#include "core/render/Image.h"
#include "core/render/Vertex.h"

#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>
#include <ImGuizmo.h>

#include <cstddef>
#include <memory>
#include <vector>

// Forward declare Engine
namespace APE 
{
	class Engine;
};


namespace APE::Render 
{

struct CameraUniform 
{
	glm::vec4 position;
};

struct ModelViewProjUniform 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct DebugModeUniform 
{
	int show_normals;
	float pad[3];
};

struct LightInfoUniform 
{
	int light_count;
	float pad[3];
};


static const ShaderDescription default_vert_shader_desc {
	.filepath = "res/shaders/Default.vert.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 2, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};

static const ShaderDescription default_frag_shader_desc {
	.filepath = "res/shaders/Default.frag.spv",
	.num_samplers = 1, 
	.num_uniform_buffers = 2, 
	.num_storage_buffers = 1, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};

static const ShaderDescription debug_vert_shader_desc {
	.filepath = "res/shaders/PositionColor.vert.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 0, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = PositionColorVertex::getLayout(),
};

static const ShaderDescription debug_frag_shader_desc {
	.filepath = "res/shaders/SolidColor.frag.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 0, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = PositionColorVertex::getLayout(),
};

struct SafePipeline 
{
	SafeGPU::UniqueGPUGraphicsPipeline fill = nullptr;
	SafeGPU::UniqueGPUGraphicsPipeline line = nullptr;
};

enum class RenderStage
{
	FrameFinished,
	FrameStarted,
	CopyPass,
	RenderPass,
	RenderGUI,
	FrameReady,
};


class Renderer 
{
	friend class APE::Engine;

	// Static resources
	std::shared_ptr<Context> m_context;
	std::shared_ptr<Shader> m_shader;
	SafePipeline m_pipeline;
	std::unique_ptr<Shader> m_debug_shader;
	SafePipeline m_debug_pipeline;
	SafeGPU::UniqueGPUSampler m_sampler;
	SafeGPU::UniqueGPUTexture m_depth_texture;
	std::unique_ptr<ImGuiSession> m_imgui_session;

	// Per-frame resources
	SDL_GPUCommandBuffer *m_cmd_buf;
	SDL_GPUTexture *m_swapchain_texture;
	SDL_GPUCopyPass *m_copy_pass;
	SDL_GPURenderPass *m_render_pass;
	SafeGPU::UniqueGPUBuffer m_light_ssbo;
	SafeGPU::UniqueGPUBuffer m_debug_buffer;

	// Render data
	RenderStage m_render_stage;
	std::vector<RenderLight> m_lights;
	std::vector<PositionColorVertex> m_debug_verts;
	bool m_wireframe_mode;
	SDL_FColor m_clear_color;
	DebugModeUniform m_debug_mode;

	constexpr static int MAX_LIGHTS = 16;

public:
	Renderer(std::shared_ptr<Context> context) noexcept;

	Renderer(
		std::shared_ptr<Context> context, 
		std::shared_ptr<Shader> shader
	) noexcept;

	~Renderer() noexcept = default;
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	void setLights(const std::vector<RenderLight>& lights) noexcept;

	bool& wireframeMode() noexcept;

	SDL_FColor& clearColor() noexcept;

	DebugModeUniform& debugMode() noexcept;

	std::unique_ptr<Shader> createShader(
		const ShaderDescription& vert_shader_desc,
		const ShaderDescription& frag_shader_desc
	) const noexcept;

	void draw(
		MeshComponent& mesh,
		MaterialComponent& material,
		std::weak_ptr<Camera> camera,
		const glm::mat4& model_matrix
	) noexcept;

	void drawLine(
		const glm::vec3& p0,
		const glm::vec3& p1,
		std::array<Uint8, 4> color,
		Camera *cam
	) noexcept;

private:
	void reset() noexcept;

	SafePipeline createPipeline(
		Shader *shader,
		SDL_GPUPrimitiveType primitive_type
	) const noexcept;

	void beginFrame() noexcept;

	void beginCopyPass() noexcept;

	void copyPass(MeshComponent& mesh, MaterialComponent& material) noexcept;

	void endCopyPass() noexcept;

	void bindFragmentSSBOs() noexcept;

	void beginRenderPass(bool b_clear, bool b_depth) noexcept;

	void renderPass(
		MeshComponent& mesh,
		MaterialComponent& material,
		std::weak_ptr<Camera> camera,
		const glm::mat4& model_matrix
	) noexcept;

	void renderDebug() noexcept;

	void endRenderPass() noexcept;

	void renderGUI() noexcept;

	void submitFrame() noexcept;

	void bindPipeline(SafePipeline *pipeline) noexcept;

	void createDepthTexture() noexcept;

	void createSampler() noexcept;

	SafeGPU::UniqueGPUGraphicsPipeline createPipeline(
		const SDL_GPUGraphicsPipelineCreateInfo& create_info
	) const noexcept;

	SafeGPU::UniqueGPUBuffer uploadBuffer(
		const std::vector<std::byte>& data,
		Uint32 usage
	) noexcept;

	void updateBuffer(
		SDL_GPUBuffer *buffer,
		const std::vector<std::byte>& data,
		Uint32 usage
	) noexcept;

	static SDL_GPUTextureFormat getTextureFormat(Image *image) noexcept;

	SafeGPU::UniqueGPUTexture createTexture(Image *image) noexcept;

	template <typename T>
	static std::vector<std::byte> vectorToRawBytes(
		const std::vector<T>& data
	) noexcept
	{
		// Copy vertex data as a vector of bytes
		const std::byte *raw_data = reinterpret_cast<const std::byte*>(data.data());
		size_t num_bytes = sizeof(T) * data.size();

		return std::vector<std::byte>(raw_data, raw_data + num_bytes);
	}
};

}; // end of namespace

