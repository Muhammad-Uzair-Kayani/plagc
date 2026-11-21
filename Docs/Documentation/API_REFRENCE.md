# plagc — Client API & Backend Translation Reference

**Scope:** APIs currently exposed to a Sandbox/game client  
**Codebase:** `Muhammad-Uzair-Kayani/plagc`, `main`, reviewed 21 November 2026  
**Rendering configuration examined:** Windows + GLFW + Ogre-Next 3.x + Vulkan  
**Status:** Source-level documentation; successful image output is **not** verified (the current Sandbox reports black cubes).

> **Purpose.** Explain (1) what a plagc client can call, (2) the equivalent vendor/API operation or internal mechanism, (3) ownership and lifetime, and (4) current implementation boundaries. This is a reference for what exists today, not a roadmap or a claim that every path has passed runtime testing.

## 1. System overview

```text
Sandbox / Client Application
    |
    +-- Application                  [owns lifecycle, EventBus, Window, Renderer, JobSystem]
    |     +-- Window                 [GLFW platform window]
    |     +-- Renderer               [Ogre Root, Vulkan plugin, native render output, workspace]
    |     +-- EventBus               [MPSC event queues + subscriptions]
    |     +-- JobSystem              [worker pool, MPMC queue, work stealing]
    |
    +-- ResourceManager::Get()       [singleton registry -> Ogre MeshPtr / TextureGpu*]
    +-- MaterialSystem               [instance -> Ogre HLMS PBS/Unlit datablocks]
    +-- SceneManager (many)          [one Ogre::SceneManager per plagc scene]
          +-- Camera (many)          [Ogre::Camera owned by scene]
          +-- SceneNodes             [hierarchy and transforms]
          +-- Items                  [Ogre::Item instances of shared v2 meshes]

Renderer::SetScene(scene, camera) connects a scene to an active compositor workspace.
```

**Public boundary:** Client code uses `plagc::Renderer`, `plagc::SceneManager`, `plagc::Camera`, `plagc::ResourceManager`, `plagc::MaterialSystem`, handles, and plain structs. Ogre pointers are resolved inside the Ogre backend. `Renderer` is **not** a plagc singleton; `ResourceManager` **is**. Ogre itself maintains a singleton `Ogre::Root`.

### Object creation and ownership

| Object | Created by client API | Who owns it? | Underlying object |
|---|---|---|---|
| Window | `WindowBuilder(...).Build()` | `Owner<Window>` | `GLFWwindow*` |
| Renderer | `RendererBuilder().WindowHandle(window).Build()` | `Owner<Renderer>` | `Ogre::Root`, Vulkan plugin, `Ogre::Window*`, workspace |
| ResourceManager | `ResourceManager::Get()` | Internal singleton | Resource registries and Ogre manager references |
| MaterialSystem | `CreateMaterialSystem(mediaRoot)` | `Owner<MaterialSystem>` | HLMS datablock registry; HLMS providers owned by Ogre |
| SceneManager | `CreateSceneManager(name, materials, workers)` | `Owner<SceneManager>` | One `Ogre::SceneManager*` |
| Camera | `scene->CreateCamera(name)` | SceneManager; client borrows pointer | `Ogre::Camera*` |
| Node | `scene->CreateNode(name, parent)` | SceneManager | `Ogre::SceneNode*` |
| Item | `scene->CreateItem(name, mesh, node)` | SceneManager | `Ogre::Item*` |
| Material | `materials->RegisterMaterial(type, name)` | MaterialSystem registry / HLMS | `Ogre::HlmsDatablock*` |
| Mesh / texture | `resources.LoadMesh/LoadTexture` | Resource registry + Ogre resource managers | `Ogre::MeshPtr` / `Ogre::TextureGpu*` |

`Owner<T>` is plagc's unique-ownership alias. The numeric handles are not raw Ogre pointers. Handles for nodes and items are scoped to their owning scene; they are not globally unique across all scenes.

## 2. Window — platform abstraction (GLFW)

**Header:** `Contracts/Platforms/Window.h`  
**Backend:** `Concrete/Platforms/Windows/WindowsWindow.cpp`

### Client API

| Client call | Result | Under the hood / notes |
|---|---|---|
| `WindowBuilder().Width(uint16_t)` | Configure width | Stored as creation settings; used by `glfwCreateWindow`. |
| `.Height(uint16_t)` | Configure height | Stored as creation settings. |
| `.Name(string)` | Window title | Passed to GLFW at creation. |
| `.Vsync(bool)` | Store VSync preference | Current Windows backend stores the value; it does **not** configure Vulkan presentation through this setter. |
| `.FullScreen(bool)` | Fullscreen preference | Uses primary monitor/video mode when `true`. |
| `.CallBack(function<bool(Owner<Event>)>)` | Route platform events | GLFW callbacks pass owned plagc events to the callback. |
| `.Build()` | `Owner<Window>` | Initializes GLFW, specifies `GLFW_NO_API`, creates a `GLFWwindow*`. |
| `window->OnUpdate()` | Poll OS events | Calls `glfwPollEvents()`. |
| `GetWidth()`, `GetHeight()` | Stored dimensions | Returns cached `WindowData` fields, not necessarily the latest framebuffer size. |
| `GetVsync()` | Stored preference | Returns stored bool. |

**Actual event wiring:** The current Windows implementation publishes a `WindowCloseRequestEvent` and constructs `KeyPressedEvent` from the GLFW key callback. Several installed GLFW callbacks (resize, framebuffer resize, focus, mouse, scroll, file drop, etc.) currently contain no event-publishing logic. The key callback currently does not branch on GLFW press/release/repeat action; all actions are forwarded as `KeyPressedEvent`. Do not document those event types as actively emitted yet.

**Native window versus Ogre window:** `WindowsWindow` owns the OS window. `OgreRenderer` retrieves the Win32 `HWND` via `glfwGetWin32Window` and passes it as `externalWindowHandle` to `Ogre::Root::createRenderWindow`. There is no second plagc window class.

## 3. Renderer — rendering orchestration (Ogre-Next/Vulkan)

**Headers:** `Contracts/RenderSystem/Renderer.h`, `Concrete/RenderSystem/Ogre/OgreRenderer.cpp`

### Client API

| Client call | Meaning | Under the hood |
|---|---|---|
| `RendererBuilder().WindowHandle(Window*)` | Supply the already-created platform window | Stores non-owning pointer for construction. |
| `.Build()` | Create renderer | Selects backend with `GET_RENDERER()`; current implemented case is `Ogre_Next`. |
| `renderer->SetScene(scene, camera)` | Choose the visible scene/view | Checks concrete types and camera ownership; `CompositorManager2::addWorkspace` using the scene, output texture, camera and `plagc.BasicWorkspace` definition. |
| `renderer->ClearScene()` | Stop rendering the active scene | `CompositorManager2::removeWorkspace`; does not destroy scene/camera. |
| `renderer->OnWindowResize()` | Notify Ogre of an external window resize | `Ogre::Window::windowMovedOrResized()`. Must currently be invoked explicitly because GLFW resize callbacks do not forward it. |
| `renderer->OnUpdate()` | Render one frame | Calls `Ogre::Root::renderOneFrame()` **only if** a workspace is active; otherwise returns. |

### Initialization internals

1. Check Window pointer; reject a second existing `Ogre::Root`.
2. Create `Ogre::Root`, create/install Vulkan plugin, select `Vulkan Rendering Subsystem`.
3. Call `Root::initialise(false)` without Ogre creating its own platform window.
4. Extract GLFW's native Win32 handle and call `Root::createRenderWindow(...)` with `externalWindowHandle`.
5. Retrieve `Root::getCompositorManager2()` and call `createBasicWorkspaceDef("plagc.BasicWorkspace", clearColor, noShadowNode)`.
6. Wait for a client to provide a scene and camera using `SetScene` before creating the workspace instance.

`SetScene` creates the replacement workspace first and then removes the old one. The renderer stores **one active workspace pointer**; it does not publicly expose a multi-view system. It owns neither the selected SceneManager nor Camera. **Remove the workspace before destroying its scene, camera, or render target.**

### Renderer limitations now

- Vulkan/GLFW external-window integration is implemented for **Windows**; the non-Windows `InitWindow` branch throws.
- No independent public compositor/window/render-view abstractions.
- No lighting, shadow configuration, advanced post-processing, offscreen render-target API, or automatic resize hookup in this contract.
- `Renderer` is **not** a singleton; although `Ogre::Root` restricts an Ogre backend to one live Root, the public API uses `Owner<Renderer>`.

## 4. ResourceManager — resource factory and cache

**Headers:** `Contracts/RenderSystem/ResourceManager.h`, `Concrete/RenderSystem/Ogre/ResourceManager.cpp`

### Client API

| Client call | Meaning | Under the hood |
|---|---|---|
| `ResourceManager::Get()` | Access/create global resource registry | Singleton `Owner<ResourceManager>`; creates `OgreResourceManager` for Ogre backend. Requires initialized Root/RenderSystem. |
| `ResourceManager::Shutdown()` | Destroy registry before Ogre Root | Resets singleton; clears mesh strong references and handle maps. |
| `AddResourceLocation(path, group="General", recursive=false)` | Register filesystem search path | Resolves `std::filesystem::absolute`; checks directory; creates Ogre group if needed; `Ogre::ResourceGroupManager::addResourceLocation(..., "FileSystem", ...)`. |
| `LoadMesh(name, group="General")` | Load registered v2 mesh | Initializes resource group, checks existence; `Ogre::MeshManager::load`; holds `Ogre::MeshPtr` in registry; returns handle. |
| `LoadTexture(name, group="General", usage=ColorSRGB)` | Load 2D texture | `TextureGpuManager::createOrRetrieveTexture`, flags/filters, `scheduleTransitionTo(Resident)`, `waitForData()`. |
| `GetMesh(name, group)` | Find cached mesh handle | Looks up `(group, name)` in mesh map; returns `InvalidResource` if missing. |
| `GetTexture(name, group)` | Find cached texture handle | Looks up `(group, name)` in texture map; returns `InvalidResource` if missing. |
| `HasResource(handle)` | Check registration | Checks handle registry. |
| `ReleaseResource(handle)` | Remove registration | Erases registry and name mapping; **does not forcibly destroy** a native resource still held/managed elsewhere. |

### TextureUsage mapping

| `TextureUsage` | Ogre translation | Purpose |
|---|---|---|
| `ColorSRGB` | `TextureFlags::PrefersLoadingFromFileAsSRGB` | Color-space handling for albedo/base color. |
| `Linear` | No sRGB or normal-specific flag | Linear-value data (e.g., roughness/metalness). |
| `Normal` | `TextureFilter::TypePrepareForNormalMapping` | Normal-map preparation. |

All three paths currently request automatic batching and default mipmap filters. The current `LoadTexture()` waits for residency before returning; it is **not** a nonblocking streaming API.

### Resource contract details

- Repeated load of the same registered `(group, name)` returns the existing handle. Repeated texture load with a different `TextureUsage` throws.
- The v2 `MeshManager::load` accepts **Ogre v2** `.mesh` files, not legacy `[MeshSerializer_v1.8]`. Use a compatible v2 mesh such as `Cube_d.mesh`, or convert old assets with OgreMeshTool.
- The implementation restricts using the same texture filename in multiple resource groups because Ogre texture aliases are global.
- `LoadMesh` and `LoadTexture` are separate namespaces in the plagc registry; an invalid or wrong-kind handle throws when backend resolution is requested.
- Resource handles are invalidated on `ReleaseResource`; an already-created `Ogre::Item` can still hold its own mesh reference. Never assume release destroys a live scene object.
- **Important current mismatch:** MaterialSystem's `SetTexture()` accepts a *filename*, not the `ResourceHandle` returned by `LoadTexture()`. They are not yet a unified handle-based texture-binding API.

## 5. MaterialSystem — materials (Ogre HLMS)

**Headers:** `Contracts/RenderSystem/MaterialSystem.h`, `Concrete/RenderSystem/Ogre/MaterialSystem.cpp`

### Creation and shader sources

```cpp
auto materials = plagc::CreateMaterialSystem(mediaPath.string());
```

`mediaPath` must point to the directory **containing** Ogre's `Hlms` folder (for example `.../Samples/Media`). The Ogre backend loads the HLMS source directories and shared libraries discovered through `HlmsUnlit::getDefaultPaths()` and `HlmsPbs::getDefaultPaths()`, then registers `Ogre::HlmsUnlit` and `Ogre::HlmsPbs` if needed. The `MaterialSystem` object owns its **registered datablock handles**, not a new global Ogre Root.

### Client API

| Client call | Meaning | Ogre translation / constraints |
|---|---|---|
| `RegisterMaterial(MaterialType, name)` | Create named material | `Hlms::createDatablock` under Unlit/PBS provider; stores handle. Duplicate names rejected. |
| `GetMaterial(name)` | Lookup material | Returns handle or `InvalidMaterial`. |
| `DestroyMaterial(handle)` | Remove material | Checks it is not linked to renderables, then `Hlms::destroyDatablock`. |
| `SetColor(handle, MaterialColor)` | Set tint / diffuse color | Unlit: `HlmsUnlitDatablock::setUseColour(true)` + `setColour(RGBA)`; PBS: `HlmsPbsDatablock::setDiffuse(RGB)` (alpha is not applied in this branch). |
| `SetRoughness(handle, value)` | Set PBS roughness | `HlmsPbsDatablock::setRoughness`; allowed `(0, 1]`; Unlit rejected. |
| `SetMetalness(handle, value)` | Set PBS metalness | `HlmsPbsDatablock::setMetalness`; allowed `[0, 1]`; Unlit rejected. |
| `SetTexture(handle, slot, filename)` | Bind texture by filename | Unlit: only `BaseColor` -> `setTexture(0, filename)`; PBS maps slots to `PBSM_*` below. |

### Material enums and default properties

| plagc enum | Ogre concept |
|---|---|
| `MaterialType::Unlit` | HLMS Unlit: no physical lighting calculations |
| `MaterialType::PBS` | HLMS PBS: physically based shading |
| `MaterialTexture::BaseColor` | Unlit slot 0; PBS `PBSM_DIFFUSE` |
| `MaterialTexture::Normal` | PBS `PBSM_NORMAL` |
| `MaterialTexture::Roughness` | PBS `PBSM_ROUGHNESS` |
| `MaterialTexture::Metalness` | PBS `PBSM_METALLIC` |

New PBS materials use metallic workflow, roughness `0.5`, metalness `0.0`. New Unlit datablocks enable color and default to white. A PBS material without the needed illumination may render dark; do not interpret successful `SetTexture()` calls as proof of correct visible shading.

**Lifetime rule:** Destroy SceneManager Items using a material before calling `DestroyMaterial` or destroying the MaterialSystem. If a datablock remains linked to an Item, `DestroyMaterial` rejects the operation. The current implementation registers texture filenames directly with Ogre HLMS, not through plagc's resource handle registry.

## 6. SceneManager — isolated scenes, nodes and instances

**Headers:** `Contracts/RenderSystem/SceneManager.h`, `Concrete/RenderSystem/Ogre/SceneManager.cpp`

### Scene creation

```cpp
auto scene = plagc::CreateSceneManager("Level.Main", *materials);
auto other = plagc::CreateSceneManager("Level.Menu", *materials, 0);
```

Each call creates one named `Ogre::SceneManager` with `Ogre::Root::createSceneManager(Ogre::ST_GENERIC, workerThreads, name)`. Names must be nonempty and unique among Ogre scene managers. The optional `workerThreads` value is passed to Ogre; it is **not** an API to assign jobs to plagc's JobSystem.

### Camera management

| Client call | Backend translation | Notes |
|---|---|---|
| `CreateCamera(name)` | `Ogre::SceneManager::createCamera(name)` plus an `OgreCamera` wrapper | Returns borrowed `Camera*`; scene owns it. |
| `GetCamera(name)` | Lookup in scene camera registry | Returns `nullptr` when missing. |
| `DestroyCamera(name)` | `Ogre::SceneManager::destroyCamera(native)` | Remove any compositor workspace referring to it first. |

### SceneNode management

| Client call | Backend translation | Notes |
|---|---|---|
| `CreateNode(name, parent=InvalidSceneNode)` | Root/parent `Ogre::SceneNode::createChildSceneNode()` | Returns `SceneNodeHandle`; parent is optional. |
| `GetNode(name)` | Name registry lookup | Returns `InvalidSceneNode` (`0`) if absent. |
| `DestroyNode(handle)` | `Ogre::SceneManager::destroySceneNode()` | Rejects node with children or attached objects. |
| `SetNodePosition(handle, x,y,z)` | `SceneNode::setPosition` | Position **relative to parent**. |
| `SetNodeScale(handle, x,y,z)` | `SceneNode::setScale` | Relative scale. |
| `SetNodeRotation(handle, yaw,pitch,roll)` | Constructs quaternion from Y yaw * X pitch * Z roll; `setOrientation` | Angles are **degrees**, combined Y-X-Z in that order. |
| `GetNodePosition(handle)` | `SceneNode::getPosition` | Returns `SceneVector3` (local position). |

### Mesh instances and materials

| Client call | Backend translation | Notes |
|---|---|---|
| `CreateItem(name, meshHandle, nodeHandle)` | Resolve v2 `MeshPtr`, `SceneManager::createItem(mesh)`, attach Item to node | Instantiates existing shared mesh data. |
| `GetItem(name)` | Scene-local name map | Returns `InvalidSceneItem` (`0`) if absent. |
| `DestroyItem(handle)` | `SceneManager::destroyItem` | Removes native instance; underlying mesh resource need not be deleted. |
| `SetMaterial(item, material)` | `Item::setDatablock(HlmsDatablock*)` | Applies to entire Item/all SubItems. |
| `SetSubMaterial(item, subItemIndex, material)` | `Item::getSubItem(index)->setDatablock(...)` | Per-submesh assignment; bounds checked. |

**Resource versus instance:** `LoadMesh` creates/locates shared mesh data; `CreateItem` creates a scene instance of that mesh. Many Items can share one mesh. The Item and its transform belong to a scene; the mesh remains managed by ResourceManager/Ogre. Multiple scenes can share MaterialSystem and ResourceManager data but have independent camera/node/item registries.

**No current client SceneManager calls** for lights, shadows, terrain, skeletal animation, physics, visibility layers, or automatic parent-node recursive deletion.

## 7. Camera — camera properties and movement

**Headers:** `Contracts/RenderSystem/Camera.h`, `Concrete/RenderSystem/Ogre/Camera.cpp`

| Client call | Ogre translation | Semantics |
|---|---|---|
| `SetPosition(x,y,z)` | `Camera::setPosition` | Camera-local position; parent SceneNode may affect final world view. |
| `GetPosition()` | `Camera::getPosition` | Returns `CameraVector3` (local position). |
| `Move(x,y,z)` | `Camera::move` | Translational offset. |
| `MoveRelative(x,y,z)` | `Camera::moveRelative` | Movement along the camera's local axes. |
| `LookAt(x,y,z)` | `Camera::lookAt` | Orient toward a world-space point. |
| `SetDirection(x,y,z)` | `Camera::setDirection` | Orient using direction vector. |
| `Yaw(degrees)` | `Camera::yaw(Radian(Degree(...)))` | Horizontal rotation in degrees; Ogre defines yaw axis behavior. |
| `Pitch(degrees)` | `Camera::pitch(Radian(Degree(...)))` | Vertical rotation in degrees. |
| `SetPerspective(fovYDegrees, aspect, near, far)` | Perspective projection + FOV/aspect/clips | Checks FOV `(0,179)`, positive aspect/near, far > near or `0`; disables auto aspect. |
| `SetOrthographic(width,height,near,far)` | Orthographic projection + ortho window/clips | Positive dimensions and near; far > near; disables auto aspect. |
| `SetAspectRatio(aspect)` | `Camera::setAspectRatio` | Positive aspect; disables auto aspect. |
| `SetAutoAspectRatio(bool)` | `Camera::setAutoAspectRatio` | Ogre adapts to viewport behavior. |

Camera does **not** create its own scene, render a frame, allocate mesh data, or own a workspace. A camera created in one scene cannot be given to `Renderer::SetScene()` with a different scene; renderer checks `nativeCamera->getSceneManager()`.

## 8. EventBus — event publication and subscriptions

**Headers:** `EventSystem/EventBus.h`, `EventSystem/Event.h`  
**Backend mechanism:** plagc MPSC queues and observer registry; platform-origin events are produced by GLFW callbacks.

| Client call | Behavior | Implementation details |
|---|---|---|
| `EventBus(capacity)` | Construct event bus | Creates main MPSC queue and smaller critical-event wait buffer. |
| `Subscribe<T>(callback)` | Register typed event observer | Returns move-only `SubscriptionHandler<T>` RAII handle. |
| `SubscriptionHandler<T>::Reset()` | Unsubscribe early | Calls `EventBus::UnSubscribe<T>`; destructor also unsubscribes. |
| `PushEvent(Owner<Event>&&)` | Enqueue owned event | Main MPSC queue, fallback wait buffer if critical; returns success flag. |
| `OnUpdate()` | Dispatch pending events | Drains queues, invokes observers in subscription order, stops dispatch for an event if status is `Consume`. |
| `Event::GetType/GetStatus/SetStatus/GetCategory/SetCategory` | Inspect/control event | `EventStatus::Continue` or `Consume`; category Normal/Critical. |

Typical client subscription:

```cpp
m_KeyEventHandle = m_EventBus->Subscribe<plagc::KeyPressedEvent>(
    [this](plagc::Event& e) { OnKeyPressed(e); }
);
```

The current dispatcher holds its observer mutex while delivering callbacks. **Do not call `Subscribe` / `Reset` / `UnSubscribe` from inside an event callback** without revisiting that locking design; it can deadlock. Handler lifetime must not exceed the EventBus it references.

**Event types versus emitted events:** Many window/mouse/keyboard event types are *declared* in headers. The current WindowsWindow implementation forwards close-request events and key-callback events; the other GLFW callbacks are present but do not yet emit events. Treat additional event emission as unfinished.

## 9. JobSystem and JobGraph — CPU task scheduling

**Headers:** `JobSystem/JobSystem.h`, `JobSystem/Job.h`, `JobSystem/JobAllocator.h`  
**Implementation:** plagc concurrency primitives, not an Ogre API translation.

### Client-oriented JobSystem API

| API | Purpose | Under the hood / caveat |
|---|---|---|
| `JobSystem()` | Create worker pool | Starts about `hardware_concurrency - 2` workers (fallback 4), global MPMC queue, per-worker work-stealing deques. |
| `Submit(fn)` | Submit indivisible callable | Creates an allocated Job and queues it. |
| `ParallelFor(start,end,fn)` | Divide range into tasks | Root job recursively subdivides work and creates child jobs; asynchronous. |
| `GetPoolSize()` | Inspect worker count | Number of worker threads. |
| `OutstandingJobs()` | Inspect in-flight job count | Atomic count; not a wait/join primitive. |
| `IsStopping()` | Inspect shutdown state | Atomic shutdown flag. |
| `ShutDown(true)` | Stop accepting work and join workers | Waits for outstanding work/cancellation to finish; do not invoke from worker. |

### Dependency-oriented JobGraph API

| API | Purpose | Under the hood |
|---|---|---|
| `JobGraph(jobSystem)` | Create a graph of CPU jobs | Holds graph-construction state until submission. |
| `AddJob(fn)` | Add one job | Allocates a Job, returns temporary `Job*` for dependency wiring. |
| `ParallelFor(start,end,fn)` | Add parallel root | Adds a range-dividing root to graph; does **not** submit yet. |
| `AddDependency(dependent, independent)` | Make dependent wait for independent | Registers continuation and increases dependency count. |
| `Submit()` | Validate DAG and schedule ready jobs | Detects cycles/external dependencies, then publishes zero-indegree roots. |

The API also exposes lower-level `CreateJob`, `PushJob`, `parallel_for`, `DivideJob`, `OnJobDestroyed`, and `JobAllocator::Allocate/DeAllocate`. They are **implementation plumbing**, not recommended entry points for ordinary game clients.

**Crucial lifetime rule:** `Job*` returned by asynchronous calls may be destroyed automatically on completion. Use it only while constructing a graph; never store and poll/dereference it after submission. A `JobGraph` that has **not** been submitted frees its allocated jobs on destruction; a submitted graph transfers completion ownership to JobSystem.

**Threading boundary:** The JobSystem does not automatically make Ogre SceneManager, rendering, or material APIs thread-safe. Keep backend scene mutations on the appropriate rendering/engine thread unless you deliberately design synchronization and cross-thread ownership.

## 10. Application — orchestration and startup/shutdown

**Headers:** `Core/Application.h`, `Core/Application.cpp`.

The client subclasses `Application` and overrides its protected virtual `OnUpdate()`. The base constructor initializes EventBus, Window, and Renderer; the client constructor can then initialize the MaterialSystem and scenes. The `Application` owns `m_Renderer`, `m_JobSystem`, `m_MaterialSystem`, and `m_Scenes` (among others).

### Actual frame loop today

```text
Application::Run()
    1. client OnUpdate()          // Sandbox updates stress-test state
    2. Window::OnUpdate()         // glfwPollEvents
    3. EventBus::OnUpdate()       // dispatch queued events
    4. Renderer::OnUpdate()       // Ogre Root::renderOneFrame if workspace exists
```

Call `Renderer::OnUpdate()` **only once** per frame. The Sandbox's `OnUpdate()` should change state; base `Application::Run()` submits the frame afterward.

### Recommended integration sequence

```cpp
// Application already created m_Window and m_Renderer before Sandbox constructor.
const std::filesystem::path media = R"(C:\path\to\ogre-next\Samples\Media)"; // replace this path
m_MaterialSystem = plagc::CreateMaterialSystem(media.string());

auto& resources = plagc::ResourceManager::Get();
resources.AddResourceLocation((media / "packs" / "DebugPack").string(), "Models");
const auto mesh = resources.LoadMesh("Cube_d.mesh", "Models");

m_Scenes.emplace_back(plagc::CreateSceneManager("Sandbox.Main", *m_MaterialSystem));
auto& scene = *m_Scenes.back();
const auto node = scene.CreateNode("Cube.Node");
const auto item = scene.CreateItem("Cube.Item", mesh, node);

const auto unlit = m_MaterialSystem->RegisterMaterial(plagc::MaterialType::Unlit, "Cube.Unlit");
m_MaterialSystem->SetColor(unlit, {1.0f, 1.0f, 1.0f, 1.0f});
scene.SetMaterial(item, unlit);

auto* camera = scene.CreateCamera("MainCamera");
camera->SetPosition(0.0f, 2.0f, 10.0f);
camera->LookAt(0.0f, 0.0f, 0.0f);
camera->SetPerspective(60.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
m_Renderer->SetScene(scene, *camera);
```

This illustrates the public call pattern. It is not evidence that the current render output is visually correct; black-cube rendering remains a separate diagnostic issue.

### Explicit lifetime order

```text
CREATE:     Window -> Renderer/Root -> MaterialSystem/ResourceManager -> Scenes
            -> Cameras/Nodes/Items -> Renderer::SetScene

TEARDOWN:   Renderer::ClearScene -> destroy Items/Scenes -> destroy Materials
            -> ResourceManager::Shutdown -> destroy Renderer/Root -> destroy Window
```

**Current Application gap:** `Application::Shutdown()` currently stops JobSystem and clears `m_Running`, but does not itself call `ResourceManager::Shutdown()`. Because ResourceManager is a function-local static singleton, explicitly shut it down before Ogre Root is destroyed; otherwise its registry may retain Ogre resource references until process/static destruction. Also ensure the Sandbox stress-test helper is destroyed while its scenes and renderer are still valid.

## 11. Quick capability and limitation matrix

| Capability | Current state | Scope / nuance |
|---|---|---|
| Windows GLFW window + native Vulkan surface integration | Implemented path | Not a second plagc window; runtime correctness still under test. |
| Render one bound scene using one workspace | Implemented | `SetScene`/`ClearScene` and one active compositor workspace. |
| Multiple SceneManagers | Implemented | Each wraps its own native Ogre scene. |
| Multiple cameras per scene | Implemented | Scene-owned objects with perspective/orthographic settings. |
| Parent/child transforms | Implemented | Create nodes, local transforms, explicit deletion. |
| Shared mesh resource with many Items | Implemented | Ogre v2 meshes only. |
| 2D texture loading/caching | Implemented | Synchronous residency wait; `TextureUsage` flags. |
| Unlit / PBS materials | Implemented | HLMS-backed handles and basic properties. |
| Texture-by-handle material binding | **Not implemented** | `SetTexture` currently takes filenames. |
| Scene lighting, shadows | **Not exposed** | No current plagc API for them. |
| Assimp, Ozz, OpenAL, ECS, physics, editor | **Not part of current rendering contracts** | Do not include in this API as implemented features. |
| Resize propagation to Ogre | Partially wired | Renderer has `OnWindowResize`, but GLFW callback is empty. |
| GLFW mouse and rich window event emission | Partially wired | Event classes exist; many native callbacks are stubs. |
| JobSystem / JobGraph | Implemented API | Dedicated plagc CPU scheduler, separate from Ogre renderer workers. |

## 12. File and API source index

The following paths were examined in `Muhammad-Uzair-Kayani/plagc` on `main`:

- [`plagc/src/Contracts/RenderSystem/Renderer.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/RenderSystem/Renderer.h)
- [`plagc/src/Concrete/RenderSystem/Ogre/OgreRenderer.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/RenderSystem/Ogre/OgreRenderer.cpp)
- [`plagc/src/Contracts/RenderSystem/SceneManager.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/RenderSystem/SceneManager.h)
- [`plagc/src/Concrete/RenderSystem/Ogre/SceneManager.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/RenderSystem/Ogre/SceneManager.cpp)
- [`plagc/src/Contracts/RenderSystem/Camera.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/RenderSystem/Camera.h)
- [`plagc/src/Concrete/RenderSystem/Ogre/Camera.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/RenderSystem/Ogre/Camera.cpp)
- [`plagc/src/Contracts/RenderSystem/ResourceManager.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/RenderSystem/ResourceManager.h)
- [`plagc/src/Concrete/RenderSystem/Ogre/ResourceManager.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/RenderSystem/Ogre/ResourceManager.cpp)
- [`plagc/src/Contracts/RenderSystem/MaterialSystem.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/RenderSystem/MaterialSystem.h)
- [`plagc/src/Concrete/RenderSystem/Ogre/MaterialSystem.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/RenderSystem/Ogre/MaterialSystem.cpp)
- [`plagc/src/Contracts/Platforms/Window.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Contracts/Platforms/Window.h)
- [`plagc/src/Concrete/Platforms/Windows/WindowsWindow.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Concrete/Platforms/Windows/WindowsWindow.cpp)
- [`plagc/src/EventSystem/EventBus.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/EventSystem/EventBus.h)
- [`plagc/src/EventSystem/EventBus.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/EventSystem/EventBus.cpp)
- [`plagc/src/JobSystem/JobSystem.h`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/JobSystem/JobSystem.h)
- [`plagc/src/JobSystem/JobSystem.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/JobSystem/JobSystem.cpp)
- [`plagc/src/Core/Application.cpp`](https://github.com/Muhammad-Uzair-Kayani/plagc/blob/main/plagc/src/Core/Application.cpp)

**Documentation policy:** Update this reference alongside changes to the contract headers and recheck the Ogre implementation before claiming new capabilities. Separate visual/runtime validation from source-level API coverage.
