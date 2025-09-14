#pragma once

#include "core/render/Vertex.h"

#include <SDL3/SDL_gpu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>

namespace APE::Render {

struct ShaderDescription {
	std::filesystem::path filepath;
	Uint32 num_samplers;
	Uint32 num_uniform_buffers;
	Uint32 num_storage_buffers;
	Uint32 num_storage_textures;
	VertexFormat vertex_format;
};

class Shader {
private:
	SDL_GPUShader* m_vert_shader;
	SDL_GPUShader* m_frag_shader;
	SDL_GPUDevice* m_device;
	ShaderDescription m_vert_desc;
	ShaderDescription m_frag_desc;

public:
	Shader(
		const ShaderDescription& vert_shader_desc, 
		const ShaderDescription& frag_shader_desc,
		SDL_GPUDevice *device) noexcept;

	~Shader() noexcept;
	Shader(const Shader& other) noexcept = delete;
	Shader& operator=(const Shader& other) noexcept = delete;
	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	[[nodiscard]] SDL_GPUShader* loadShader(
		const ShaderDescription& shader_desc, 
		SDL_GPUShaderStage stage) noexcept;

	[[nodiscard]] SDL_GPUShader* getVertexShader() const noexcept;

	[[nodiscard]] SDL_GPUShader* getFragmentShader() const noexcept;

	[[nodiscard]] SDL_GPUDevice* getDevice() const noexcept;

	[[nodiscard]] VertexFormat getVertexFormat() const noexcept;
};

};	// end of namespace

