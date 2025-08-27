#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_stdinc.h>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <vector>

namespace APE::Render {

struct VertexFormat {
	std::vector<SDL_GPUVertexAttribute> attributes;
	std::vector<SDL_GPUVertexBufferDescription> buffer_descs;

	VertexFormat(const std::vector<SDL_GPUVertexAttribute>& attributes,
		const std::vector<SDL_GPUVertexBufferDescription>& buffer_descs)
		noexcept
		: attributes(attributes)
		, buffer_descs(buffer_descs)
	{

	}

	// Only allow to be called as an lvalue
	[[nodiscard]] SDL_GPUVertexInputState getInputState() const & noexcept
	{
		SDL_GPUVertexInputState input_state = {
			.vertex_buffer_descriptions =
				buffer_descs.data(),

			.num_vertex_buffers =
				static_cast<Uint32>(buffer_descs.size()),

			.vertex_attributes =
				attributes.data(),

			.num_vertex_attributes =
				static_cast<Uint32>(attributes.size()),
		};
		return input_state;
	}

	// Don't allow temporaries, destruction invalidates internal pointers
	SDL_GPUVertexInputState getInputState() const && = delete;
};

struct PositionColorVertex {
	glm::vec3 pos;
	Uint8 r, g, b, a;

	[[nodiscard]] static VertexFormat getLayout() noexcept
	{
		std::vector<SDL_GPUVertexBufferDescription> buffer_desc = {{
			.slot = 0,
			.pitch = sizeof(PositionColorVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0,
		}};

		std::vector<SDL_GPUVertexAttribute> attributes = {{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0,
		}, {
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
			.offset = sizeof(glm::vec3),
		}};

		return VertexFormat(attributes, buffer_desc);
	}
};

struct TextureVertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;

	[[nodiscard]] static VertexFormat getLayout() noexcept
	{
		std::vector<SDL_GPUVertexBufferDescription> buffer_desc = {{
			.slot = 0,
			.pitch = sizeof(TextureVertex),
			.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
			.instance_step_rate = 0,
		}};

		std::vector<SDL_GPUVertexAttribute> attributes = {{
			.location = 0,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = 0,
		}, {
			.location = 1,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
			.offset = sizeof(glm::vec3),
		}, {
			.location = 2,
			.buffer_slot = 0,
			.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
			.offset = sizeof(glm::vec3) * 2,
		}};

		return VertexFormat(attributes, buffer_desc);
	}
};

};	// end of namespace

