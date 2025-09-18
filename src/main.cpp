#include "editor/Editor.h"
#include "core/Engine.h"

int main() 
{
	APE::Engine::init("APE Physics Engine", 1200, 800);
	APE::Engine::pushLayer(std::make_unique<APE::Editor::EditorLayer>());
	// APE::Engine::pushLayer(std::make_unique<APE::Game::GameLayer>());
	APE::Engine::run();

	return 0; 
}
