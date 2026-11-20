#include <plagc.h>
#include "RenderingStressTest.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace sandbox
{
    namespace
    {
        void Check(bool condition, const char* label)
        {
            if (!condition) throw std::runtime_error(std::string("Rendering integration test failed: ") + label);
            CLIENT_INFO("[PASS] {}", label);
        }
    }

    RenderingStressTest::RenderingStressTest(plagc::Renderer& renderer, plagc::MaterialSystem& materials,
                                             plagc::SceneManager& mainScene, plagc::SceneManager& secondaryScene)
        : m_Renderer(renderer), m_Materials(materials), m_MainScene(mainScene), m_SecondaryScene(secondaryScene) {}

    RenderingStressTest::~RenderingStressTest()
    {
        try { Shutdown(); }
        catch (const std::exception& e) { CLIENT_ERROR("Rendering test cleanup failed: {}", e.what()); }
    }

    void RenderingStressTest::Initialize(const RenderingTestAssets& assets)
    {
        using namespace plagc;
        if (m_Initialized) throw std::logic_error("RenderingStressTest is already initialized");
        if (!assets.InstanceCount) throw std::invalid_argument("InstanceCount must be nonzero");

        ResourceManager& resources = ResourceManager::Get();
        resources.AddResourceLocation(assets.MeshDirectory, "StressMeshes");
        resources.AddResourceLocation(assets.TextureDirectory, "StressTextures");

        m_Mesh = resources.LoadMesh(assets.MeshFile, "StressMeshes");
        m_Albedo = resources.LoadTexture(assets.AlbedoFile, "StressTextures", TextureUsage::ColorSRGB);
        m_Normal = resources.LoadTexture(assets.NormalFile, "StressTextures", TextureUsage::Normal);
        m_Roughness = resources.LoadTexture(assets.RoughnessFile, "StressTextures", TextureUsage::Linear);
        m_Metalness = resources.LoadTexture(assets.MetalnessFile, "StressTextures", TextureUsage::Linear);

        Check(resources.LoadMesh(assets.MeshFile, "StressMeshes") == m_Mesh, "mesh caching");
        Check(resources.LoadTexture(assets.AlbedoFile, "StressTextures", TextureUsage::ColorSRGB) == m_Albedo, "texture caching");
        Check(resources.GetMesh(assets.MeshFile, "StressMeshes") == m_Mesh, "GetMesh");
        Check(resources.GetTexture(assets.AlbedoFile, "StressTextures") == m_Albedo, "GetTexture");
        Check(resources.HasResource(m_Mesh) && resources.HasResource(m_Normal), "HasResource");
        Check(resources.GetMesh("missing.mesh", "StressMeshes") == InvalidResource, "missing mesh lookup");
        bool rejectedUsageChange = false;
        try { resources.LoadTexture(assets.AlbedoFile, "StressTextures", TextureUsage::Normal); }
        catch (const std::exception&) { rejectedUsageChange = true; }
        Check(rejectedUsageChange, "texture usage mismatch is rejected");

        m_Unlit = m_Materials.RegisterMaterial(MaterialType::Unlit, "Sandbox.Test.Unlit");
        m_Pbs = m_Materials.RegisterMaterial(MaterialType::PBS, "Sandbox.Test.PBS");
        Check(m_Materials.GetMaterial("Sandbox.Test.Unlit") == m_Unlit, "GetMaterial Unlit");
        Check(m_Materials.GetMaterial("Sandbox.Test.PBS") == m_Pbs, "GetMaterial PBS");
        bool rejectedDuplicateMaterial = false;
        try { m_Materials.RegisterMaterial(MaterialType::Unlit, "Sandbox.Test.Unlit"); }
        catch (const std::exception&) { rejectedDuplicateMaterial = true; }
        Check(rejectedDuplicateMaterial, "duplicate material name is rejected");

        m_Materials.SetColor(m_Unlit, { 0.25f, 0.8f, 1.0f, 1.0f });
        m_Materials.SetColor(m_Pbs, { 0.9f, 0.7f, 0.3f, 1.0f });
        m_Materials.SetRoughness(m_Pbs, 0.32f);
        m_Materials.SetMetalness(m_Pbs, 0.8f);
        m_Materials.SetTexture(m_Unlit, MaterialTexture::BaseColor, assets.AlbedoFile);
        m_Materials.SetTexture(m_Pbs, MaterialTexture::BaseColor, assets.AlbedoFile);
        m_Materials.SetTexture(m_Pbs, MaterialTexture::Normal, assets.NormalFile);
        m_Materials.SetTexture(m_Pbs, MaterialTexture::Roughness, assets.RoughnessFile);
        m_Materials.SetTexture(m_Pbs, MaterialTexture::Metalness, assets.MetalnessFile);
        CLIENT_INFO("[PASS] PBS/Unlit color, roughness, metalness and texture slots");

        const MaterialHandle disposable = m_Materials.RegisterMaterial(MaterialType::Unlit, "Sandbox.Test.Disposable");
        m_Materials.DestroyMaterial(disposable);
        Check(m_Materials.GetMaterial("Sandbox.Test.Disposable") == InvalidMaterial, "DestroyMaterial");

        m_MainCamera = m_MainScene.CreateCamera("Sandbox.MainCamera");
        Check(m_MainScene.GetCamera("Sandbox.MainCamera") == m_MainCamera, "CreateCamera/GetCamera");
        m_MainCamera->SetPosition(0.0f, 8.0f, 35.0f);
        const CameraVector3 position = m_MainCamera->GetPosition();
        Check(std::abs(position.Z - 35.0f) < 0.001f, "Camera::GetPosition");
        m_MainCamera->Move(0.0f, 1.0f, 0.0f);
        m_MainCamera->MoveRelative(0.0f, 0.0f, -1.0f);
        m_MainCamera->SetDirection(0.0f, -0.15f, -1.0f);
        m_MainCamera->Yaw(5.0f);
        m_MainCamera->Pitch(-3.0f);
        m_MainCamera->SetOrthographic(30.0f, 17.0f, 0.1f, 1000.0f);
        m_MainCamera->SetAspectRatio(1280.0f / 720.0f);
        m_MainCamera->SetAutoAspectRatio(true);
        m_MainCamera->SetPerspective(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        m_MainCamera->LookAt(0.0f, 0.0f, -14.0f);
        CLIENT_INFO("[PASS] all camera movement and projection methods");

        m_MainScene.CreateCamera("Sandbox.TemporaryCamera");
        m_MainScene.DestroyCamera("Sandbox.TemporaryCamera");
        Check(m_MainScene.GetCamera("Sandbox.TemporaryCamera") == nullptr, "DestroyCamera");

        m_Parent = m_MainScene.CreateNode("Sandbox.Parent");
        m_Child = m_MainScene.CreateNode("Sandbox.Child", m_Parent);
        Check(m_MainScene.GetNode("Sandbox.Child") == m_Child, "CreateNode/GetNode/parenting");
        m_MainScene.SetNodePosition(m_Parent, 0.0f, 0.0f, -14.0f);
        m_MainScene.SetNodePosition(m_Child, 0.0f, 0.0f, 0.0f);
        m_MainScene.SetNodeScale(m_Child, 0.8f, 0.8f, 0.8f);
        m_MainScene.SetNodeRotation(m_Child, 30.0f, 15.0f, 0.0f);
        const SceneVector3 nodePos = m_MainScene.GetNodePosition(m_Parent);
        Check(std::abs(nodePos.Z + 14.0f) < 0.001f, "GetNodePosition");

        bool rejectedParentDeletion = false;
        try { m_MainScene.DestroyNode(m_Parent); }
        catch (const std::exception&) { rejectedParentDeletion = true; }
        Check(rejectedParentDeletion, "parent node cannot be destroyed with children");

        m_Sample = m_MainScene.CreateItem("Sandbox.Sample", m_Mesh, m_Child);
        Check(m_MainScene.GetItem("Sandbox.Sample") == m_Sample, "CreateItem/GetItem");
        m_MainScene.SetMaterial(m_Sample, m_Unlit);
        m_MainScene.SetSubMaterial(m_Sample, 0, m_Pbs);
        m_MainScene.SetMaterial(m_Sample, m_Unlit); // Keep scene visible without lights.
        CLIENT_INFO("[PASS] SetMaterial/SetSubMaterial");
        bool rejectedInUseMaterial = false;
        try { m_Materials.DestroyMaterial(m_Unlit); }
        catch (const std::exception&) { rejectedInUseMaterial = true; }
        Check(rejectedInUseMaterial, "in-use material cannot be destroyed");

        const SceneNodeHandle disposableNode = m_MainScene.CreateNode("Sandbox.DisposableNode");
        const SceneItemHandle disposableItem = m_MainScene.CreateItem("Sandbox.DisposableItem", m_Mesh, disposableNode);
        m_MainScene.DestroyItem(disposableItem);
        Check(m_MainScene.GetItem("Sandbox.DisposableItem") == InvalidSceneItem, "DestroyItem");
        m_MainScene.DestroyNode(disposableNode);
        Check(m_MainScene.GetNode("Sandbox.DisposableNode") == InvalidSceneNode, "DestroyNode");

        const auto creationStart = std::chrono::steady_clock::now();
        const std::size_t columns = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(assets.InstanceCount))));
        m_StressNodes.reserve(assets.InstanceCount);
        m_StressItems.reserve(assets.InstanceCount);
        for (std::size_t i = 0; i < assets.InstanceCount; ++i)
        {
            const std::string suffix = std::to_string(i);
            const SceneNodeHandle node = m_MainScene.CreateNode("Stress.Node." + suffix, m_Parent);
            m_StressNodes.push_back(node);
            const float x = (static_cast<float>(i % columns) - static_cast<float>(columns) * 0.5f) * 2.0f;
            const float y = (static_cast<float>(i / columns) - static_cast<float>(columns) * 0.5f) * 2.0f;
            m_MainScene.SetNodePosition(node, x, y, 0.0f);
            m_MainScene.SetNodeScale(node, 0.65f, 0.65f, 0.65f);
            const SceneItemHandle item = m_MainScene.CreateItem("Stress.Item." + suffix, m_Mesh, node);
            m_StressItems.push_back(item);
            m_MainScene.SetMaterial(item, m_Unlit);
        }
        const auto creationMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - creationStart).count();
        CLIENT_INFO("[STRESS] created {} mesh instances in {} ms", assets.InstanceCount, creationMs);

        m_SecondaryCamera = m_SecondaryScene.CreateCamera("Sandbox.SecondaryCamera");
        m_SecondaryCamera->SetPosition(0.0f, 1.0f, 8.0f);
        m_SecondaryCamera->LookAt(0.0f, 0.0f, 0.0f);
        m_SecondaryCamera->SetPerspective(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
        m_SecondaryNode = m_SecondaryScene.CreateNode("Sandbox.SecondaryNode");
        m_SecondaryItem = m_SecondaryScene.CreateItem("Sandbox.SecondaryItem", m_Mesh, m_SecondaryNode);
        m_SecondaryScene.SetMaterial(m_SecondaryItem, m_Unlit);
        Check(m_MainScene.GetNode("Sandbox.SecondaryNode") == InvalidSceneNode, "independent SceneManagers");

        m_Renderer.SetScene(m_MainScene, *m_MainCamera);
        m_Renderer.OnWindowResize(); // Invokes the existing external-window resize notification API.
        m_Renderer.ClearScene();
        m_Renderer.SetScene(m_MainScene, *m_MainCamera);
        CLIENT_INFO("[PASS] SetScene/ClearScene/OnWindowResize");
        m_Frame = 0;
        m_LastFpsLog = std::chrono::steady_clock::now();
        m_Initialized = true;
        CLIENT_INFO("=== Rendering integration stress test initialized ===");
    }

    void RenderingStressTest::OnUpdate()
    {
        if (!m_Initialized) return;
        ++m_Frame;
        if (m_Frame == 120) m_Renderer.SetScene(m_SecondaryScene, *m_SecondaryCamera);
        if (m_Frame == 180) m_Renderer.SetScene(m_MainScene, *m_MainCamera);

        const std::size_t count = std::min<std::size_t>(m_StressNodes.size(), 64);
        for (std::size_t i = 0; i < count; ++i)
            m_MainScene.SetNodeRotation(m_StressNodes[i], static_cast<float>(m_Frame) * 0.25f + static_cast<float>(i), 0.0f, 0.0f);

        if (m_Frame % 120 == 0)
        {
            const auto now = std::chrono::steady_clock::now();
            const double seconds = std::chrono::duration<double>(now - m_LastFpsLog).count();
            if (seconds > 0.0) CLIENT_INFO("[STRESS] observed loop FPS over 120 frames: {}", 120.0 / seconds);
            m_LastFpsLog = now;
        }
    }

    void RenderingStressTest::Shutdown()
    {
        using namespace plagc;
        m_Renderer.ClearScene(); // Workspace must release references to cameras/scenes first.

        for (SceneItemHandle item : m_StressItems) m_MainScene.DestroyItem(item);
        m_StressItems.clear();
        for (SceneNodeHandle node : m_StressNodes) m_MainScene.DestroyNode(node);
        m_StressNodes.clear();

        if (m_Sample != InvalidSceneItem) { m_MainScene.DestroyItem(m_Sample); m_Sample = InvalidSceneItem; }
        if (m_SecondaryItem != InvalidSceneItem) { m_SecondaryScene.DestroyItem(m_SecondaryItem); m_SecondaryItem = InvalidSceneItem; }
        if (m_Child != InvalidSceneNode) { m_MainScene.DestroyNode(m_Child); m_Child = InvalidSceneNode; }
        if (m_Parent != InvalidSceneNode) { m_MainScene.DestroyNode(m_Parent); m_Parent = InvalidSceneNode; }
        if (m_SecondaryNode != InvalidSceneNode) { m_SecondaryScene.DestroyNode(m_SecondaryNode); m_SecondaryNode = InvalidSceneNode; }
        if (m_MainCamera) { m_MainScene.DestroyCamera("Sandbox.MainCamera"); m_MainCamera = nullptr; }
        if (m_SecondaryCamera) { m_SecondaryScene.DestroyCamera("Sandbox.SecondaryCamera"); m_SecondaryCamera = nullptr; }

        if (m_Unlit != InvalidMaterial) { m_Materials.DestroyMaterial(m_Unlit); m_Unlit = InvalidMaterial; }
        if (m_Pbs != InvalidMaterial) { m_Materials.DestroyMaterial(m_Pbs); m_Pbs = InvalidMaterial; }

        ResourceManager& resources = ResourceManager::Get();
        std::unordered_set<ResourceHandle> released;
        auto release = [&](ResourceHandle& handle)
        {
            if (handle == InvalidResource) return;
            if (released.insert(handle).second)
            {
                resources.ReleaseResource(handle);
                Check(!resources.HasResource(handle), "ReleaseResource/HasResource after release");
            }
            handle = InvalidResource;
        };
        release(m_Mesh);
        release(m_Albedo);
        release(m_Normal);
        release(m_Roughness);
        release(m_Metalness);
        m_Initialized = false;
        CLIENT_INFO("[PASS] released Items, Nodes, Cameras, Materials and Resource handles");
    }
}
