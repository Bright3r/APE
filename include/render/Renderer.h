#pragma once

#include "render/Context.h"
#include "render/Shader.h"

#include <memory>

namespace APE {
namespace Render {

struct Renderer {
	std::shared_ptr<Context> context;
	std::shared_ptr<Shader> shader;
	bool wireframe_mode;
	SDL_FColor clear_color;

private:
	SDL_GPUGraphicsPipeline *fill_pipeline;
	SDL_GPUGraphicsPipeline *line_pipeline;

public:
	// Special Member Functions
	Renderer(std::shared_ptr<Context> context, std::shared_ptr<Shader> shader);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer& operator=(const Renderer& other) = delete;

	// API Functions
	void createPipelines(std::shared_ptr<Shader> new_shader);
	void draw();
};

}; // end of namespace Render
}; // end of namespace APE
