#include "layers/game/GameLayer.h"

#include "core/Engine.h"

namespace APE::Game {

void GameLayer::setup() noexcept
{

}

void GameLayer::update() noexcept
{

}

void GameLayer::draw() noexcept
{
	auto& world = Engine::world();

	auto view = world.registry.view<
		Render::MeshComponent,
		Render::MaterialComponent,
		TransformComponent,
		HierarchyComponent>();

	for (auto [ent, mesh, material, transform, hierarchy] : view.each()) {
		glm::mat4 model_mat = world.getModelMatrix(ent);
		Engine::renderer()->draw(
			mesh,
			material,
			Engine::getCamera(),
			model_mat
		);
	}
}

void GameLayer::drawGUI() noexcept
{

}

};	// end of namespace

