#include <plagc.h>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreLight.h>
#include <OgreColourValue.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <initializer_list>

class SandboxApp final : public plagc::Application
{
public:

	SandboxApp()
	{
		CLIENT_INFO("Sandbox initialized");

		const std::filesystem::path mediaPath =
			R"(C:\Personals\Game Engine Development\Development\plagc\third-party\ogre-next\Samples\Media)";

		// 1. Initialize materials and one scene.
		m_MaterialSystem = plagc::CreateMaterialSystem(mediaPath.string());
		m_Scenes.emplace_back(
			plagc::CreateSceneManager("Sandbox.Main", *m_MaterialSystem, 1u)
		);
		auto& scene = *m_Scenes.front();
		auto& materials = *m_MaterialSystem;

		// 2. Register resource directories and load a shared mesh plus two textures.
		auto& resources = plagc::ResourceManager::Get();
		resources.AddResourceLocation(
			(mediaPath / "packs" / "DebugPack").string(), "DemoMeshes");
		resources.AddResourceLocation(
			(mediaPath / "materials" / "textures").string(), "DemoTextures");

		const auto cubeMesh = resources.LoadMesh("Cube_d.mesh", "DemoMeshes");

		resources.LoadTexture("Panels_Diffuse.png", "DemoTextures", plagc::TextureUsage::ColorSRGB);
		resources.LoadTexture("Panels_Normal_Tangent.png", "DemoTextures", plagc::TextureUsage::Normal);

		// 3. Solid-colour PBS metal.
		const auto metalMaterial = materials.RegisterMaterial(plagc::MaterialType::PBS, "Sandbox.Metal");

		materials.SetColor(metalMaterial, { 0.85f, 0.72f, 0.42f, 1.0f });
		materials.SetMetalness(metalMaterial, 0.8f);
		materials.SetRoughness(metalMaterial, 0.2f);

		// 4. PBS material with diffuse and normal textures.
		const auto normalMaterial = materials.RegisterMaterial(plagc::MaterialType::PBS, "Sandbox.NormalMapped");

		materials.SetColor(normalMaterial, { 1.0f, 1.0f, 1.0f, 1.0f });
		materials.SetMetalness(normalMaterial, 0.05f);
		materials.SetRoughness(normalMaterial, 0.45f);
		materials.SetTexture(normalMaterial, plagc::MaterialTexture::BaseColor, "Panels_Diffuse.png");
		materials.SetTexture(normalMaterial, plagc::MaterialTexture::Normal, "Panels_Normal_Tangent.png");

		// 5. Two separate Items share the same mesh.
		m_MetalNode = scene.CreateNode("MetalNode");
		scene.SetNodePosition(m_MetalNode, -2.1f, 0.0f, 0.0f);
		const auto metalItem = scene.CreateItem("MetalCube", cubeMesh, m_MetalNode);
		scene.SetMaterial(metalItem, metalMaterial);

		m_NormalNode = scene.CreateNode("NormalNode");
		scene.SetNodePosition(m_NormalNode, 2.1f, 0.0f, 0.0f);
		const auto normalItem = scene.CreateItem("NormalCube", cubeMesh, m_NormalNode);
		scene.SetMaterial(normalItem, normalMaterial);

		// 6. Camera.
		auto* camera = scene.CreateCamera("MainCamera");
		camera->SetPosition(0.0f, 2.0f, 12.0f);
		camera->LookAt(0.0f, 0.0f, 0.0f);
		camera->SetPerspective(60.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
		camera->SetAutoAspectRatio(true);

		// 7. Directional light.
		auto* light = scene.CreateDirectionalLight("directional.light");
		light->SetDirection(-0.5f, -1.0f, -0.5f);
		light->SetDiffuseColor({ 1.0f, 1.0f, 1.0f });
		light->SetSpecularColor({ 1.0f, 1.0f, 1.0f });
		light->SetPowerScale(10.0f);
		light->SetCastShadows(true);

		// 8. Shadow configuration.
		plagc::WorkspaceConfig workspace;
		workspace.Shadows = plagc::ShadowSettings{};

		workspace.Shadows->DirectionalResolution = 512;
		workspace.Shadows->PssmSplits = 2;
		workspace.Shadows->Filter = plagc::ShadowFilter::PCF_2x2;
		workspace.Shadows->MaxSpotLights = 0;
		workspace.Shadows->MaxPointLights = 0;

		m_Renderer->SetScene(scene, *camera, workspace);

		m_LastUpdate = std::chrono::steady_clock::now();
	}


	~SandboxApp() override
	{
		// Workspace must be gone before the scene and camera are destroyed.
		m_Renderer->ClearScene();
		m_Scenes.clear();
		m_MaterialSystem.reset();
		plagc::ResourceManager::Shutdown(); // Before Ogre Root destruction.
	}

private:
	void OnUpdate() override
	{
		const auto now = std::chrono::steady_clock::now();
		const float dt = std::chrono::duration<float>(now - m_LastUpdate).count();
		CLIENT_INFO("FrameTime: {}", dt);
		m_LastUpdate = now;

		m_Elapsed += dt;
		m_Rotation = std::fmod(m_Rotation + 50.0f * dt, 360.0f);

		// Smoothly oscillates within 0.70 - 1.30 (never exceeds these bounds).
		const float scale = 1.0f + 0.30f * std::sin(m_Elapsed * 2.0f);

		auto& scene = *m_Scenes.front();
		for (const plagc::SceneNodeHandle node : {m_MetalNode, m_NormalNode})
		{
			scene.SetNodeRotation(node, m_Rotation, m_Rotation * 0.5f, 0.0f);
			scene.SetNodeScale(node, scale, scale, scale);
		}
	}

	plagc::SceneNodeHandle m_MetalNode = plagc::InvalidSceneNode;
	plagc::SceneNodeHandle m_NormalNode = plagc::InvalidSceneNode;
	std::chrono::steady_clock::time_point m_LastUpdate{};
	float m_Elapsed = 0.0f;
	float m_Rotation = 0.0f;
};

Owner<plagc::Application> plagc::CreateApplication()
{
	return MakeUnique<SandboxApp>();
}