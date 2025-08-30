#pragma once

#include "render/Camera.h"
#include "render/Context.h"
#include "render/ImGuiSession.h"
#include "render/SafeGPU.h"
#include "render/Shader.h"
#include "render/Mesh.h"
#include "render/Model.h"
#include "render/Image.h"

#include <SDL3/SDL_gpu.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstddef>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

namespace APE::Render {

struct ModelViewProjUniform {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

static const ShaderDescription default_vert_shader_desc {
	.filepath = "res/shaders/Default.vert.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 1, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};

static const ShaderDescription default_frag_shader_desc {
	.filepath = "res/shaders/Default.frag.spv",
	.num_samplers = 1, 
	.num_uniform_buffers = 1, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
	.vertex_format = Model::VertexType::getLayout(),
};


class Renderer {
private:
	std::shared_ptr<Context> m_context;
	bool m_wireframe_mode;
	SDL_FColor m_clear_color;

	std::shared_ptr<Shader> m_shader;
	SafeGPU::UniqueGPUGraphicsPipeline m_fill_pipeline;
	SafeGPU::UniqueGPUGraphicsPipeline m_line_pipeline;
	SDL_GPUTexture* m_swapchain_texture;
	SDL_GPURenderPass* m_render_pass;
	SDL_GPUCommandBuffer* m_cmd_buf;
	bool m_is_drawing;
	SafeGPU::UniqueGPUSampler m_sampler;
	SafeGPU::UniqueGPUTexture m_depth_texture;

	std::unique_ptr<ImGuiSession> m_imgui_session;

public:
	// Special Member Functions
	//
	Renderer(std::shared_ptr<Context> context) noexcept;
	Renderer(std::shared_ptr<Context> context, 
		std::shared_ptr<Shader> shader) noexcept;
	~Renderer() noexcept = default;
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	// API Functions
	//
	void reset() noexcept;

	[[nodiscard]] std::unique_ptr<Shader> createShader(
		const ShaderDescription& vert_shader_desc,
		const ShaderDescription& frag_shader_desc) const noexcept;

	void useShader(Shader* shader) noexcept;

	[[nodiscard]] float getAspectRatio() const noexcept;

	void beginRenderPass(bool b_clear, bool b_depth) noexcept;

	void beginDrawing() noexcept;

	void draw(Model* model, std::weak_ptr<Camera> camera) noexcept;

	void endDrawing() noexcept;

private:
	void draw(Model::ModelMesh& mesh,
		const glm::mat4& model_mat, 
		Camera* camera) noexcept;

	void createDepthTexture() noexcept;

	void createSampler() noexcept;

	[[nodiscard]] SafeGPU::UniqueGPUGraphicsPipeline createPipeline(
		const SDL_GPUGraphicsPipelineCreateInfo& create_info) const noexcept;

	[[nodiscard]] SafeGPU::UniqueGPUBuffer uploadBuffer(
		const std::vector<std::byte>& data,
		Uint32 usage) noexcept;

	[[nodiscard]] static SDL_GPUTextureFormat getTextureFormat(
		Image* image) noexcept;

	[[nodiscard]] SafeGPU::UniqueGPUTexture createTexture(Image* image) noexcept;

	template <typename T>
	[[nodiscard]] static std::vector<std::byte> vectorToRawBytes(
		const std::vector<T>& data) noexcept
	{
		// Copy vertex data as a vector of bytes
		const std::byte* raw_data = reinterpret_cast<const std::byte*>(data.data());
		size_t num_bytes = sizeof(T) * data.size();

		return std::vector<std::byte>(raw_data, raw_data + num_bytes);
	}
};

}; // end of namespace

