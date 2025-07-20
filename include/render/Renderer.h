#pragma once

#include "render/Context.h"
#include "render/Shader.h"

#include <functional>
#include <memory>

namespace APE {
namespace Render {

static const ShaderDescription default_shader_desc {
	"res/shaders/RawTriangle.vert.spv",
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

	void draw(std::function<void(SDL_GPURenderPass*)> draw_scene);
};

}; // end of namespace Render
}; // end of namespace APE
