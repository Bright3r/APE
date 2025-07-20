#pragma once

#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace APE {
namespace Render {

struct ShaderDescription {
	const std::filesystem::path& vert_shader_filepath;
	const std::filesystem::path& frag_shader_filepath;
	Uint32 num_samplers;
	Uint32 num_uniform_buffers;
	Uint32 num_storage_buffers;
	Uint32 num_storage_textures;
};

struct Shader {
	SDL_GPUShader *vert_shader;
	SDL_GPUShader *frag_shader;
	SDL_GPUDevice *device;

	Shader(const ShaderDescription& shader_desc, SDL_GPUDevice *device);
	~Shader();
	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;
	Shader(Shader&& other);
	Shader& operator=(Shader&& other);

	SDL_GPUShader* loadShader(const ShaderDescription& shader_desc, 
			   SDL_GPUShaderStage stage);
};

};	// end of namespace Render
};	// end of namespace APE
