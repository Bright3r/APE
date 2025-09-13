#include "render/Model.h"

#include <SDL3/SDL_stdinc.h>

#include <assimp/material.h>
#include <assimp/postprocess.h>

namespace APE::Render {

Model::Model(std::filesystem::path model_path) noexcept
	: model_path(model_path)
{

}


};	// end of namespace

