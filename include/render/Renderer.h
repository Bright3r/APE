#pragma once

#include "render/Camera.h"
#include "render/Context.h"
#include "render/SafeGPU.h"
#include "render/Shader.h"
#include "render/Mesh.h"
#include "render/Model.h"
#include "render/Vertex.h"

#include <SDL3/SDL_gpu.h>
#include <cstring>
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


class Renderer {
private:
	std::shared_ptr<Context> m_context;
	bool m_wireframe_mode;
	SDL_FColor m_clear_color;

	SafeGPU::UniqueGPUGraphicsPipeline m_fill_pipeline;
	SafeGPU::UniqueGPUGraphicsPipeline m_line_pipeline;

	Camera* m_cam;
	SDL_GPURenderPass* m_render_pass;
	SDL_GPUCommandBuffer* m_cmd_buf;
	bool m_is_drawing;


public:
	// Special Member Functions
	//
	Renderer(std::shared_ptr<Context> context, Camera *cam);
	Renderer(std::shared_ptr<Context> context, Camera *cam, Shader* shader);
	~Renderer() = default;
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	// API Functions
	//
	std::unique_ptr<Shader> createShader(ShaderDescription shader_desc) const;

	void useShader(Shader* shader);

	float getAspectRatio() const;

	void beginDrawing();

	void endDrawing();

	void draw(Model& model);

	void draw(Mesh& mesh, const glm::mat4& model_mat);

private:
	SafeGPU::UniqueGPUGraphicsPipeline createPipeline(
		const SDL_GPUGraphicsPipelineCreateInfo& create_info) const;

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

		// Execute copy pass
		SDL_EndGPUCopyPass(copy_pass);
		SDL_SubmitGPUCommandBuffer(cmd_buffer);

		// Cleanup resources
		SDL_ReleaseGPUTransferBuffer(m_context->device, transfer_buffer);

		return buffer;
	}
};

}; // end of namespace Render
}; // end of namespace APE
