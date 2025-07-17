#include "render/Shader.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <filesystem>
#include <iostream>

namespace APE {
namespace Render {

Shader::Shader(const std::filesystem::path& vert_filepath,
	  const std::filesystem::path& frag_filepath,
	  SDL_GPUDevice *device,
	  Uint32 num_samplers,
	  Uint32 num_uniform_buffers,
	  Uint32 num_storage_buffers,
	  Uint32 num_storage_textures)
	: device(device)
{
	vert_shader = loadShader(vert_filepath, SDL_GPU_SHADERSTAGE_VERTEX, num_samplers, 
			 num_uniform_buffers, num_storage_buffers, num_storage_textures);

	frag_shader = loadShader(frag_filepath, SDL_GPU_SHADERSTAGE_FRAGMENT, num_samplers, 
			 num_uniform_buffers, num_storage_buffers, num_storage_textures);
}

Shader::~Shader()
{
	SDL_ReleaseGPUShader(device, vert_shader);
	SDL_ReleaseGPUShader(device, frag_shader);
}

Shader::Shader(Shader&& other)
	: device(other.device), vert_shader(other.vert_shader), frag_shader(other.frag_shader)
{
	other.device = nullptr;
	other.vert_shader = nullptr;
	other.frag_shader = nullptr;
}

Shader& Shader::operator=(Shader&& other)
{
	if (this != &other) {
		// Release our shaders
		if (vert_shader)
			SDL_ReleaseGPUShader(device, vert_shader);

		if (frag_shader)
			SDL_ReleaseGPUShader(device, frag_shader);

		// Move other's pointers into ours
		vert_shader = other.vert_shader;
		frag_shader = other.frag_shader;

		// Clear other
		other.vert_shader = nullptr;
		other.frag_shader = nullptr;
	}

	return *this;
}

SDL_GPUShader* Shader::loadShader(const std::filesystem::path& filepath,
			  SDL_GPUShaderStage stage,
			  Uint32 num_samplers,
			  Uint32 num_uniform_buffers,
			  Uint32 num_storage_buffers,
			  Uint32 num_storage_textures)
{
	// Read shader code into buffer
	size_t code_size;
	void *code = SDL_LoadFile(filepath.c_str(), &code_size);
	if (!code) {
		std::cerr << "Failed to load shader code from " << filepath << ": "
			<< SDL_GetError() << "\n";
		return nullptr;
	}

	// Detect shader format and corresponding shader entrypoint
	std::string entrypoint;
	SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;

	if (backend_formats & SDL_GPU_SHADERFORMAT_SPIRV) {
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	}
	else if (backend_formats & SDL_GPU_SHADERFORMAT_MSL) {
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	}
	else if (backend_formats & SDL_GPU_SHADERFORMAT_DXIL) {
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	}
	else {
		std::cerr << "Failed to detect shader format in " << filepath << " : " 
			<< SDL_GetError() << "\n";

		return nullptr;
	}

	SDL_GPUShaderCreateInfo shaderInfo = {
		.code_size = code_size,
		.code = static_cast<Uint8*>(code),
		.entrypoint = entrypoint.c_str(),
		.format = format,
		.stage = stage,
		.num_samplers = num_samplers,
		.num_storage_textures = num_storage_textures,
		.num_storage_buffers = num_storage_buffers,
		.num_uniform_buffers = num_uniform_buffers
	};

	SDL_GPUShader *shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (!shader) {
		std::cerr << "SDL_CreateGPUShader Failed: " << SDL_GetError() << "\n";
		SDL_free(code);
		return nullptr;
	}

	SDL_free(code);
	return shader;
}

};	// end of namespace Render
};	// end of namespace APE
