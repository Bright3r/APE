#pragma once

#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace APE {
namespace Render {

struct Shader {
	SDL_GPUShader *vert_shader;
	SDL_GPUShader *frag_shader;
	SDL_GPUDevice *device;

	Shader(const std::filesystem::path& vert_filepath,
			  const std::filesystem::path& frag_filepath,
			  SDL_GPUDevice *device,
			  Uint32 num_samplers,
			  Uint32 num_uniform_buffers,
			  Uint32 num_storage_buffers,
			  Uint32 num_storage_textures);

	~Shader();

	Shader(const Shader& other) = delete;
	Shader& operator=(const Shader& other) = delete;
	Shader(Shader&& other);
	Shader& operator=(Shader&& other);

	SDL_GPUShader* loadShader(const std::filesystem::path& filepath,
			  SDL_GPUShaderStage stage,
			  Uint32 num_samplers,
			  Uint32 num_uniform_buffers,
			  Uint32 num_storage_buffers,
			  Uint32 num_storage_textures);
};

};	// end of namespace Render
};	// end of namespace APE
