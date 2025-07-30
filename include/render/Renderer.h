#pragma once

#include "render/Context.h"
#include "render/Shader.h"

#include <functional>
#include <memory>
#include <vector>

namespace APE {
namespace Render {

struct PositionColorVertex {
	float x, y, z;
	Uint8 r, g, b, a;
};

static const ShaderDescription default_shader_desc {
	"res/shaders/PositionColor.vert.spv",
	"res/shaders/SolidColor.frag.spv",
	0, 
	0, 
	0, 
	0
};

struct Renderer {
	std::shared_ptr<Context> context;
	bool wireframe_mode;
	SDL_FColor clear_color;

private:
	SDL_GPUGraphicsPipeline *fill_pipeline;
	SDL_GPUGraphicsPipeline *line_pipeline;
	SDL_GPUBuffer *vertex_buffer;

public:
	// Special Member Functions
	Renderer(std::shared_ptr<Context> context);
	Renderer(std::shared_ptr<Context> context, Shader* shader);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	// API Functions
	std::unique_ptr<Shader> createShader(ShaderDescription shader_desc) const;

	void useShader(Shader* shader);

	void useVertexData(std::vector<PositionColorVertex> vertex_data);

	void draw(std::function<void(SDL_GPURenderPass*)> draw_scene);
};

}; // end of namespace Render
}; // end of namespace APE
