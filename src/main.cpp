#include "layers/game/GameLayer.h"
#include "layers/editor/EditorLayer.h"
#include "core/Engine.h"

int main() 
{
	APE::Engine::init("APE Physics Engine", 1200, 800);

	APE::Engine::pushLayer(std::make_unique<APE::Game::GameLayer>());
	APE::Engine::pushLayer(std::make_unique<APE::Editor::EditorLayer>());
	APE::Engine::run();

	return 0; 
}
