#include "layers/game/GameLayer.h"

#include "core/Engine.h"
#include "core/components/Render.h"

namespace APE::Game 
{

void GameLayer::setup() noexcept
{

}

void GameLayer::update() noexcept
{

}

void GameLayer::draw() noexcept
{
	auto& world = Engine::world();

	// Get lights
	std::vector<Render::RenderLight> lights;
	for (auto& [ent, light_comp] : world.registry.view<Render::LightComponent>().each())
	{
		auto world_transform = world.getWorldTransform(ent);
		Render::RenderLight light(
			glm::vec4(world_transform.position, 0),
			light_comp.attenuation,
			light_comp.ambient_color,
			light_comp.diffuse_color,
			light_comp.specular_color,
			light_comp.type,
			world_transform.getLookatDirection()
		);
		lights.emplace_back(light);
	}
	Engine::renderer()->setLights(lights);

	// Draw each model in scene
	auto view = world.registry.view<
		Render::MeshComponent,
		Render::MaterialComponent,
		TransformComponent,
		HierarchyComponent>();
	for (auto [ent, mesh, material, transform, hierarchy] : view.each()) 
	{
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

