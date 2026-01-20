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
#include <array>
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

// Uniform Buffer Formats
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

struct LightShadowMapUniform
{
	glm::mat4 light_space_matrix;
	glm::mat4 model;
};


// Shader Descriptions
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
	.num_samplers = 2, 
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

static const ShaderDescription shadow_map_vert_shader_desc {
	.filepath = "res/shaders/ShadowMapping.vert.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 1, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};
static const ShaderDescription shadow_map_frag_shader_desc {
	.filepath = "res/shaders/ShadowMapping.frag.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 0, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};

static const ShaderDescription quad_vert_shader_desc {
	.filepath = "res/shaders/quad.vert.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 0, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Position2DVertex::getLayout(),
};
static const ShaderDescription quad_frag_shader_desc {
	.filepath = "res/shaders/quad.frag.spv",
	.num_samplers = 1, 
	.num_uniform_buffers = 0, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Position2DVertex::getLayout(),
};


enum class RenderStage
{
	FrameFinished,
	FrameStarted,
	CopyPass,
	ShadowPass,
	RenderPass,
	RenderGUI,
	FrameReady,
};

struct Quad
{
	std::vector<Position2DVertex> vertices = {
		Position2DVertex{{-1.f, -1.f}, {0.f, 0.f}},
		Position2DVertex{{ 1.f, -1.f}, {1.f, 0.f}},
		Position2DVertex{{ 1.f,  1.f}, {1.f, 1.f}},
		Position2DVertex{{-1.f,  1.f}, {0.f, 1.f}}
	};

	std::vector<Uint32> indices = {
		0, 1, 2,
		2, 3, 0
	};

	SafeGPU::UniqueGPUBuffer vertex_buffer = nullptr;
	SafeGPU::UniqueGPUBuffer index_buffer = nullptr;
};


class Renderer 
{
	friend class APE::Engine;

	constexpr static int MAX_LIGHTS = 16;
	constexpr static int SHADOW_MAP_RES = 1024;

	// Static resources
	std::shared_ptr<Context> m_context;
	std::shared_ptr<Shader> m_shader;
	std::shared_ptr<Shader> m_debug_shader;
	std::shared_ptr<Shader> m_shadow_shader;
	SafeGPU::SafePipeline m_pipeline;
	SafeGPU::SafePipeline m_debug_pipeline;
	SafeGPU::SafePipeline m_shadow_pipeline;
	SafeGPU::UniqueGPUSampler m_sampler;
	SafeGPU::UniqueGPUSampler m_shadow_sampler;
	SafeGPU::UniqueGPUTexture m_depth_texture;
	std::array<SafeGPU::UniqueGPUTexture, MAX_LIGHTS> m_shadow_maps;
	std::unique_ptr<ImGuiSession> m_imgui_session;

	// Per-frame resources
	SDL_GPUCommandBuffer *m_cmd_buf;
	SDL_GPUTexture *m_swapchain_texture;
	SDL_GPUCopyPass *m_copy_pass;
	SDL_GPURenderPass *m_render_pass;
	SafeGPU::UniqueGPUBuffer m_light_ssbo;
	SafeGPU::UniqueGPUBuffer m_debug_buffer;
	RenderLight *m_shadow_pass_light;

	// Render data
	RenderStage m_render_stage;
	std::vector<RenderLight> m_lights;
	std::vector<PositionColorVertex> m_debug_verts;
	bool m_wireframe_mode;
	SDL_FColor m_clear_color;
	DebugModeUniform m_debug_mode;

	std::shared_ptr<Shader> m_quad_shader;
	SafeGPU::SafePipeline m_quad_pipeline;
	SafeGPU::UniqueGPUSampler m_quad_sampler;
	
	Quad m_quad;

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

	std::shared_ptr<Shader> createShader(
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

	SafeGPU::SafePipeline createPipeline(
		Shader *shader,
		SDL_GPUPrimitiveType primitive_type,
		bool render_to_swapchain
	) const noexcept;

	void beginFrame() noexcept;

	void beginCopyPass() noexcept;

	void copyPass(MeshComponent& mesh, MaterialComponent& material) noexcept;

	void endCopyPass() noexcept;

	void bindFragmentSSBOs() noexcept;

	void beginShadowPass(SDL_GPUTexture *shadow_map, RenderLight *light) noexcept;

	void shadowPass(MeshComponent& mesh, const glm::mat4& model_matrix) noexcept;

	void endShadowPass() noexcept;

	void beginRenderPass(
		const SafeGPU::SafePipeline& pipeline,
		SDL_GPUTexture *target_texture,
		bool b_clear,
		bool b_depth = false,
		SDL_GPUTexture *depth_texture = nullptr
	) noexcept;

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

	void bindPipeline(const SafeGPU::SafePipeline& pipeline) noexcept;

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

	SafeGPU::UniqueGPUTexture createTexture(
		const SDL_GPUTextureCreateInfo& texture_desc
	) const noexcept;

	SafeGPU::UniqueGPUResource<SDL_GPUSampler> createSampler(
		const SDL_GPUSamplerCreateInfo& sampler_desc
	) const noexcept;

	SDL_GPUTextureCreateInfo defaultDepthTextureDesc() const noexcept;

	SDL_GPUTextureCreateInfo shadowMapTextureDesc() const noexcept;

	SDL_GPUSamplerCreateInfo defaultSamplerDesc() const noexcept;

	SDL_GPUSamplerCreateInfo shadowMapSamplerDesc() const noexcept;

	SDL_GPUSamplerCreateInfo quadSamplerDesc() const noexcept;

	void copyQuad() noexcept;
	void renderQuad(SDL_GPUTexture *texture) noexcept;


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

