#include <plagc.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <filesystem>


#include "RenderingStressTest.h"



class SandboxApp : public plagc::Application
{
public:


	SandboxApp()
	{
		CLIENT_INFO("Sandbox Initialized");

		m_KeyEventHandle = m_EventBus->Subscribe<plagc::KeyPressedEvent>(
			std::bind(&SandboxApp::OnKeyPressed, this, std::placeholders::_1)
		);

		const std::filesystem::path mediaPath =
			R"(C:\Personals\Game Engine Development\Development\plagc\third-party\ogre-next\Samples\Media)";

		m_MaterialSystem = plagc::CreateMaterialSystem(mediaPath.string());

		m_Scenes.emplace_back(
			plagc::CreateSceneManager("Sandbox.Main", *m_MaterialSystem)
		);

		m_Scenes.emplace_back(
			plagc::CreateSceneManager("Sandbox.Secondary", *m_MaterialSystem)
		);

		sandbox::RenderingTestAssets assets;

		// Ogre v2 compatible mesh
		assets.MeshDirectory = (mediaPath / "packs" / "DebugPack").string();
		assets.MeshFile = "Cube_d.mesh";

		// Actual directory from materials.zip
		assets.TextureDirectory = (mediaPath / "materials" / "textures").string();

		// Existing Ogre sample textures
		assets.AlbedoFile = "Panels_Diffuse.png";
		assets.NormalFile = "Panels_Normal_Tangent.png";
		assets.RoughnessFile = "GlassRoughness.png";
		assets.MetalnessFile = "Panels_refmask.png";

		assets.InstanceCount = 256;

		m_RenderTest = std::make_unique<sandbox::RenderingStressTest>(
			*m_Renderer,
			*m_MaterialSystem,
			*m_Scenes[0],
			*m_Scenes[1]
		);

		m_RenderTest->Initialize(assets);
	}


	~SandboxApp()
	{
		m_RenderTest.reset();
	}

	void OnKeyPressed(plagc::Event& e)
	{
		if (e.GetType() == plagc::KeyPressedEvent::StaticType)
		{
			auto& keyEvent = static_cast<plagc::KeyPressedEvent&>(e);
			CLIENT_INFO("Key Board Button Pressed, Key Code: {}", keyEvent.ScanCode);
		}
	}

	void OnUpdate() override
	{
		if (m_RenderTest)
			m_RenderTest->OnUpdate();
	}

	void StressTest()
	{
		constexpr size_t count = 11'000;
		constexpr int iterations = 5'000;

		auto parallelResults =
			std::make_shared<std::vector<double>>(count);

		auto sequentialResults =
			std::make_shared<std::vector<double>>(count);

		plagc::JobGraph graph(m_JobSystem);

		auto parallelStart = std::chrono::steady_clock::now();

		// JOB A: Parallel execution
		auto jobA = graph.ParallelFor(
			0, count,
			[parallelResults](size_t i)
			{
				double value = static_cast<double>(i) + 1.0;

				for (int j = 0; j < iterations; ++j)
				{
					value = std::sqrt(
						value * 1.000001 + (j % 7) + 1.0
					);
				}

				(*parallelResults)[i] = value;
			}
		);

		// JOB B: Sequential execution
		auto jobB = graph.AddJob(
			[parallelResults, sequentialResults, count, parallelStart]()
			{
				auto parallelEnd = std::chrono::steady_clock::now();

				double parallelMs =
					std::chrono::duration<double, std::milli>(
						parallelEnd - parallelStart
					).count();

				auto sequentialStart = std::chrono::steady_clock::now();

				for (size_t i = 0; i < count; ++i)
				{
					double value = static_cast<double>(i) + 1.0;

					for (int j = 0; j < iterations; ++j)
					{
						value = std::sqrt(
							value * 1.000001 + (j % 7) + 1.0
						);
					}

					(*sequentialResults)[i] = value;
				}

				auto sequentialEnd = std::chrono::steady_clock::now();

				double sequentialMs =
					std::chrono::duration<double, std::milli>(
						sequentialEnd - sequentialStart
					).count();

				double parallelChecksum = 0.0;
				double sequentialChecksum = 0.0;

				for (size_t i = 0; i < count; ++i)
				{
					parallelChecksum += (*parallelResults)[i];
					sequentialChecksum += (*sequentialResults)[i];
				}

				CLIENT_INFO("=== Job System Benchmark ===");
				CLIENT_INFO("Parallel Time: {} ms", parallelMs);
				CLIENT_INFO("Sequential Time: {} ms", sequentialMs);
				CLIENT_INFO("Speedup: {}x", sequentialMs / parallelMs);
				CLIENT_INFO("Efficiency (10 workers): {}%",
					(sequentialMs / parallelMs) / 10.0 * 100.0);

				CLIENT_INFO("Parallel Checksum: {}", parallelChecksum);
				CLIENT_INFO("Sequential Checksum: {}", sequentialChecksum);
				CLIENT_INFO("Checksums Match: {}",
					parallelChecksum == sequentialChecksum);
			}
		);

		// Job B executes ONLY after Job A fully completes
		graph.AddDependency(jobB, jobA);

		graph.Submit();
	}


private:


private:

	plagc::JobAllocator m_Allocator;

	plagc::SubscriptionHandler<plagc::KeyPressedEvent> m_KeyEventHandle;

	std::unique_ptr<sandbox::RenderingStressTest> m_RenderTest;
};


Owner<plagc::Application> plagc::CreateApplication()
{
	return MakeUnique<SandboxApp>();
}