#pragma once

#include "render/Camera.h"
#include "render/Context.h"
#include "render/SafeGPU.h"
#include "render/Shader.h"
#include "render/Mesh.h"
#include "render/Model.h"
#include "render/Image.h"

#include <SDL3/SDL_gpu.h>
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
	.vert_shader_filepath = "res/shaders/Default.vert.spv",
	.frag_shader_filepath = "res/shaders/Default.frag.spv",
	.num_samplers = 0, 
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

	Camera* m_cam;
	SDL_GPURenderPass* m_render_pass;
	SDL_GPUCommandBuffer* m_cmd_buf;
	bool m_is_drawing;

	SafeGPU::UniqueGPUSampler m_sampler;


public:
	// Special Member Functions
	//
	Renderer(std::shared_ptr<Context> context, Camera *cam);
	Renderer(std::shared_ptr<Context> context, 
	  Camera *cam, 
	  std::shared_ptr<Shader> shader);
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

	void draw(Model::MeshType& mesh, const glm::mat4& model_mat);

private:
	SafeGPU::UniqueGPUGraphicsPipeline createPipeline(
		const SDL_GPUGraphicsPipelineCreateInfo& create_info) const;

	SDL_GPUBuffer* uploadBuffer(const std::vector<Uint8>& data, Uint32 usage);

	SafeGPU::UniqueGPUTexture createTexture(Image* image);

	SafeGPU::UniqueGPUSampler createSampler();

	template <typename T>
	static std::vector<Uint8> vectorToRawBytes(const std::vector<T>& data)
	{
		// Copy vertex data as a vector of bytes
		const Uint8* raw_data = reinterpret_cast<const Uint8*>(data.data());
		size_t num_bytes = sizeof(T) * data.size();

		return std::vector<Uint8>(raw_data, raw_data + num_bytes);
	}
};

}; // end of namespace Render
}; // end of namespace APE
