#include "editor/EditorApp.h"
#include "core/AppRunner.h"

int main() 
{
	auto editor = std::make_unique<APE::Editor::EditorApp>();
	AppRunner::init(std::move(editor), "APE Physics Engine", 1200, 800);
	AppRunner::run();

	return 0; 
}
