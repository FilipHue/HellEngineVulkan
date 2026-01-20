#include "test_bed/tests.h"
#include "sandbox.h"

int main(int argc, char** argv)
{
	ApplicationConfiguration config("Sandbox", 1600, 900, false, argv[0]);

	std::vector<Application*> apps{
		new SandboxApplication(&config),			// 0
#if _TEST_BED_ENABLE
		new TestCpuParticles(&config),				// 1
		new TestDescriptorSet(&config),				// 2
		new TestDynamicAttachment(&config),			// 3
		new TestDynamicUniformBuffer(&config),		// 4
		new TestGBuffer(&config),					// 5
		new TestGltfLoading(&config),				// 6
		new TestOffscreen(&config),					// 7
		new TestPipelines(&config),					// 8
		new TestPushConstants(&config),				// 9
		new TestSpecializationConstants(&config),	// 10
		new TestStencil(&config),					// 11
		new TestTexture(&config),					// 12
		new TestTexture3D(&config),					// 13
		new TestTextureArray(&config),				// 14
		new TestTextureCubemap(&config),			// 15
		new TestTextureCubemapArray(&config),		// 16
		new TestTriangle(&config)					// 17
#endif // _TEST_BED_ENABLE
	};

	Engine::GetInstance().Init();

	Engine::GetInstance().Run(*apps[0]);

	Engine::GetInstance().Shutdown();
}