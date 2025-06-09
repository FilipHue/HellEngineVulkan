#include "src/editor.h"

int main(int argc, char** argv)
{
	ApplicationConfiguration config("HellEditor", 1600, 900, false, argv[0]);

	Application* editor = new Editor(&config);

	Engine::GetInstance().Init();

	Engine::GetInstance().Run(*editor);

	Engine::GetInstance().Shutdown();
}