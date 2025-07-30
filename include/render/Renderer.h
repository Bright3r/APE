#pragma once

#include "render/Camera.h"
#include "render/Context.h"
#include "render/Shader.h"

#include <SDL3/SDL_gpu.h>
#include <cstring>
#include <functional>
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

namespace APE {
namespace Render {

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

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
	Camera* m_cam;
	SDL_GPUBuffer* m_vertex_buffer;

	SDL_GPUGraphicsPipeline* m_fill_pipeline;
	SDL_GPUGraphicsPipeline* m_line_pipeline;

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

	void useVertexData(std::vector<PositionColorVertex> vertex_data);

	float getAspectRatio() const;

	void draw(std::function<void(SDL_GPURenderPass*)> draw_scene);

private:
	// std::vector<PositionColorVertex> vertex_data = {
	// 	{    -1,    -1, 0, 255,   0,   0, 255 },
	// 	{     1,    -1, 0,   0, 255,   0, 255 },
	// 	{     0,     1, 0,   0,   0, 255, 255 }
	// };

	std::vector<PositionColorVertex> vertex_data = {
		// Front face (z = +0.5)
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-left
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-right
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-right

		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-left
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-right
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-left

		// Back face (z = -0.5)
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-right
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-left
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-left

		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-right
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-left
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-right

		// Left face (x = -0.5)
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-back
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Bottom-front
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-front

		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Bottom-back
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Top-front
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Top-back

		// Right face (x = +0.5)
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-front
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Bottom-back
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-back

		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Bottom-front
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Top-back
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Top-front

		// Top face (y = +0.5)
		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Front-left
		{  0.5,  0.5,  0.5,   0,   0, 255, 255 }, // Front-right
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Back-right

		{ -0.5,  0.5,  0.5, 255, 255,   0, 255 }, // Front-left
		{  0.5,  0.5, -0.5,   0,   0,   0, 255 }, // Back-right
		{ -0.5,  0.5, -0.5, 255, 255, 255, 255 }, // Back-left

		// Bottom face (y = -0.5)
		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Back-left
		{  0.5, -0.5, -0.5, 255,   0, 255, 255 }, // Back-right
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Front-right

		{ -0.5, -0.5, -0.5,   0, 255, 255, 255 }, // Back-left
		{  0.5, -0.5,  0.5,   0, 255,   0, 255 }, // Front-right
		{ -0.5, -0.5,  0.5, 255,   0,   0, 255 }, // Front-left
	};

	template <typename T>
	SDL_GPUBuffer* uploadBuffer(const std::vector<T>& data, Uint32 usage) {
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
