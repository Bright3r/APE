#pragma once

#include "render/Camera.h"
#include "render/Context.h"
#include "render/Shader.h"
#include "render/Mesh.h"

#include <SDL3/SDL_gpu.h>
#include <cstring>
#include <functional>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

namespace APE {
namespace Render {

struct ModelViewProjUniform {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

static const ShaderDescription default_shader_desc {
	.vert_shader_filepath = "res/shaders/PositionColorUniform.vert.spv",
	.frag_shader_filepath = "res/shaders/SolidColor.frag.spv",
	.num_samplers = 0, 
	.num_uniform_buffers = 1, 
	.num_storage_buffers = 0, 
	.num_storage_textures = 0,
};

class Renderer {
private:
	std::shared_ptr<Context> m_context;
	bool m_wireframe_mode;
	SDL_FColor m_clear_color;

	SDL_GPUGraphicsPipeline* m_fill_pipeline;
	SDL_GPUGraphicsPipeline* m_line_pipeline;

	Camera* m_cam;
	std::vector<std::weak_ptr<Mesh>> m_scene;

public:
	// Special Member Functions
	//
	Renderer(std::shared_ptr<Context> context, Camera *cam);
	Renderer(std::shared_ptr<Context> context, Camera *cam, Shader* shader);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	// API Functions
	//
	std::unique_ptr<Shader> createShader(ShaderDescription shader_desc) const;

	void useShader(Shader* shader);

	float getAspectRatio() const;

	void drawMesh(Mesh* mesh, SDL_GPURenderPass* render_pass);

	void draw(std::function<void(SDL_GPURenderPass*)> draw_scene);

private:
	template <typename T>
	SDL_GPUBuffer* uploadBuffer(const std::vector<T>& data, Uint32 usage)
	{
		// Create GPU buffer
		Uint32 buffer_size = sizeof(T) * data.size();
		SDL_GPUBufferCreateInfo buffer_info = {
			.usage = usage,
			.size = buffer_size,
		};

		SDL_GPUBuffer *buffer = SDL_CreateGPUBuffer(
			m_context->device,
			&buffer_info
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
		T *mapped = static_cast<T*>(
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

		// Cleanup resources
		SDL_EndGPUCopyPass(copy_pass);
		SDL_SubmitGPUCommandBuffer(cmd_buffer);
		SDL_ReleaseGPUTransferBuffer(m_context->device, transfer_buffer);

		return buffer;
	}
};

}; // end of namespace Render
}; // end of namespace APE
