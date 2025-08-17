#include "render/Shader.h"
#include "util/Logger.h"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>

namespace APE {
namespace Render {

Shader::Shader(const ShaderDescription& vert_desc, 
	       const ShaderDescription& frag_desc,
	       SDL_GPUDevice* device) noexcept
	: m_vert_shader(nullptr)
	, m_frag_shader(nullptr)
	, m_device(device)
	, m_vert_desc(vert_desc)
	, m_frag_desc(frag_desc)
{
	m_vert_shader = loadShader(vert_desc, SDL_GPU_SHADERSTAGE_VERTEX); 
	m_frag_shader = loadShader(frag_desc, SDL_GPU_SHADERSTAGE_FRAGMENT);
}

Shader::~Shader() noexcept
{
	SDL_ReleaseGPUShader(m_device, m_vert_shader);
	SDL_ReleaseGPUShader(m_device, m_frag_shader);
}

Shader::Shader(Shader&& other) noexcept
	: m_vert_shader(other.m_vert_shader)
	, m_frag_shader(other.m_frag_shader)
	, m_device(other.m_device)
	, m_vert_desc(other.m_vert_desc)
	, m_frag_desc(other.m_frag_desc)
{
	other.m_device = nullptr;
	other.m_vert_shader = nullptr;
	other.m_frag_shader = nullptr;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this != &other) {
		// Release our shaders
		if (m_vert_shader)
			SDL_ReleaseGPUShader(m_device, m_vert_shader);

		if (m_frag_shader)
			SDL_ReleaseGPUShader(m_device, m_frag_shader);

		// Move other's pointers into ours
		m_device = other.m_device;
		m_vert_shader = other.m_vert_shader;
		m_frag_shader = other.m_frag_shader;
		m_vert_desc = other.m_vert_desc;
		m_frag_desc = other.m_frag_desc;

		// Clear other
		other.m_device = nullptr;
		other.m_vert_shader = nullptr;
		other.m_frag_shader = nullptr;
	}

	return *this;
}

SDL_GPUShader* Shader::loadShader(
	const ShaderDescription& shader_desc, 
	SDL_GPUShaderStage stage) noexcept
{
	// Read shader code into buffer
	size_t code_size;
	void *code = SDL_LoadFile(shader_desc.filepath.c_str(), &code_size);
	if (!code) {
		APE_ERROR("Failed to load shader code from {} - {}", 
	    		shader_desc.filepath.c_str(), 
	    		SDL_GetError()
	   	);
		return nullptr;
	}

	// Detect shader format and corresponding shader entrypoint
	std::string entrypoint;
	SDL_GPUShaderFormat backend_formats = SDL_GetGPUShaderFormats(m_device);
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
		APE_ERROR(
			"Failed to detect shader format from {} - {}",
			shader_desc.filepath.c_str(),
			SDL_GetError()
		);

		return nullptr;
	}

	SDL_GPUShaderCreateInfo shaderInfo = {
		.code_size = code_size,
		.code = static_cast<Uint8*>(code),
		.entrypoint = entrypoint.c_str(),
		.format = format,
		.stage = stage,
		.num_samplers = shader_desc.num_samplers,
		.num_storage_textures = shader_desc.num_storage_textures,
		.num_storage_buffers = shader_desc.num_storage_buffers,
		.num_uniform_buffers = shader_desc.num_uniform_buffers
	};

	SDL_GPUShader *shader = SDL_CreateGPUShader(m_device, &shaderInfo);
	APE_CHECK((shader != nullptr),
		"SDL_CreateGPUShader Failed - {}",
		SDL_GetError()
	);

	SDL_free(code);
	return shader;
}


SDL_GPUShader* Shader::getVertexShader() const noexcept
{
	return m_vert_shader;
}

SDL_GPUShader* Shader::getFragmentShader() const noexcept
{
	return m_frag_shader;
}

SDL_GPUDevice* Shader::getDevice() const noexcept
{
	return m_device;
}

VertexFormat Shader::getVertexFormat() const noexcept
{
	return m_vert_desc.vertex_format;
}

};	// end of namespace Render
};	// end of namespace APE
