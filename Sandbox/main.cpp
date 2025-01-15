#include "test_bed/tests.h"
#include "sandbox.h"

int main(int argc, char** argv)
{
	ApplicationConfiguration config("Sandbox", 1600, 900, false, argv[0]);

	std::vector<Application*> apps{
		new SandboxApplication(&config),
		new TestDescriptorSet(&config),
		new TestDynamicBuffer(&config)
	};

	Engine::GetInstance().Init();

	Engine::GetInstance().Run(*apps[0]);

	Engine::GetInstance().Shutdown();
}