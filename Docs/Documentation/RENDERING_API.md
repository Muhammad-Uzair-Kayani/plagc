# plagc Rendering System — API Reference

**Version:** Phase 3 (lights, ambient illumination, and shadows)  
**Namespace:** `plagc`  
**Primary language:** C++20  
**Backend in the supplied implementation:** Ogre-Next 3.0 / Vulkan (Windows external-window integration)

> **Source scope.** This document is based on the supplied `plagc_lighting_phase3.zip` (the cumulative Phase 1–3 update) and the original `RenderSystem (2).zip` for APIs that were not changed in Phase 3. It documents the code as supplied, **not** a separately verified build or the latest live GitHub revision. Public methods and internal implementation details are deliberately separated.

## Contents

1. [System structure and ownership](#1-system-structure-and-ownership)
2. [Renderer and workspace APIs](#2-renderer-and-workspace-apis)
3. [SceneManager API](#3-scenemanager-api)
4. [Camera API](#4-camera-api)
5. [Light APIs](#5-light-apis)
6. [Ambient illumination API](#6-ambient-illumination-api)
7. [Shadow configuration API](#7-shadow-configuration-api)
8. [MaterialSystem API](#8-materialsystem-api)
9. [ResourceManager API](#9-resourcemanager-api)
10. [Concrete Ogre classes (internal/backend APIs)](#10-concrete-ogre-classes-internalbackend-apis)
11. [Typical usage sequences](#11-typical-usage-sequences)
12. [Lifecycle, errors and limits](#12-lifecycle-errors-and-limits)
13. [Header and source index](#13-header-and-source-index)

---

## 1. System structure and ownership

### 1.1 Public contracts versus concrete backends

| Public contract / type | Concrete implementation | Role |
|---|---|---|
| `Renderer` | `OgreRenderer` | Root rendering initialization, scene binding, compositor workspace, frame rendering |
| `SceneManager` | `OgreSceneManager` | Scene cameras, light wrappers, node hierarchy and mesh items |
| `Camera` | `OgreCamera` | Camera movement, orientation and projection |
| `Light` / its subtypes | `OgreDirectionalLight`, `OgrePointLight`, `OgreSpotLight` | Light properties and native Ogre light operations |
| `MaterialSystem` | `OgreMaterialSystem` | PBS/Unlit datablocks, material handles and backend shadow sampling settings |
| `ResourceManager` | `OgreResourceManager` | Mesh/texture resource handles, caching and Ogre resource lookup |
| `ShadowSettings` | Used by `OgreRenderer` and `OgreMaterialSystem` | Plain workspace-level configuration value; **not** a polymorphic object |
| `WorkspaceConfig` | Read by `OgreRenderer` | Optional shadow configuration passed when selecting a scene |

### 1.2 Ownership rules

| Object | Who owns it? | How client code receives it | Destruction |
|---|---|---|---|
| Renderer | Client `Owner<Renderer>` | `RendererBuilder().WindowHandle(window).Build()` | Release owner after renderer-dependent resources have been destroyed |
| Scene | Client `Owner<SceneManager>` | `CreateSceneManager(...)` | Destroy scene after clearing its compositor workspace |
| Camera | `SceneManager` | Borrowed `Camera*` from `CreateCamera`/`GetCamera` | `DestroyCamera(name)` or scene destruction |
| Light | `SceneManager` | Borrowed typed pointer from `Create*Light`/`Get*Light` | `DestroyLight(name)` or scene destruction |
| Scene node | `SceneManager` | `SceneNodeHandle` (`uint64_t`) | `DestroyNode(handle)` after dependents are gone |
| Mesh item | `SceneManager` | `SceneItemHandle` (`uint64_t`) | `DestroyItem(handle)` or scene destruction |
| Material | `MaterialSystem` | `MaterialHandle` (`uint64_t`) | `DestroyMaterial(handle)` after no renderables use it |
| Mesh/texture registration | `ResourceManager` singleton | `ResourceHandle` (`uint64_t`) | `ReleaseResource(handle)`; not necessarily native GPU eviction |
| Native Ogre light / camera / node | Native Ogre scene | Non-owning pointer retained by wrappers/records | Managed through corresponding `Ogre::SceneManager` |
| Compositor workspace | `OgreRenderer` | No public handle | `ClearScene()` / renderer shutdown |

**Important:** A pointer returned by `CreateCamera`, `GetCamera`, `Create*Light` or `Get*Light` is **borrowed**. Never `delete` it or store it after its scene has been destroyed. `Owner<T>` is the project's owning pointer abstraction (as used in the supplied headers); it is not a separate rendering handle type.

---

## 2. Renderer and workspace APIs

**Public headers:** `Contracts/RenderSystem/Renderer.h`, `Contracts/RenderSystem/Shadow.h`

### 2.1 `RendererBuilder`

| Public method | Returns | What it does |
|---|---|---|
| `RendererBuilder& WindowHandle(Window* window)` | Builder reference | Supplies an existing application window; supports chaining. Does **not** take ownership of that window. |
| `Owner<Renderer> Build()` | Owning renderer | Validates a non-null window and creates the selected backend implementation. |

**Internal:** `BuildRenderer()` chooses the backend from `GET_RENDERER()`. In the supplied code, `RenderSystem::Ogre_Next` constructs `OgreRenderer`; other paths throw.

```cpp
Owner<Renderer> renderer = RendererBuilder()
    .WindowHandle(window)
    .Build();
```

### 2.2 `Renderer`

| Public method | What it does | Notes |
|---|---|---|
| `void SetScene(SceneManager& scene, Camera& camera)` | Sets the current scene/camera using the **basic workspace without shadows**. | Kept for compatibility. Calls configured overload with empty `WorkspaceConfig`. |
| `void SetScene(SceneManager& scene, Camera& camera, const WorkspaceConfig& config)` | Creates and selects a workspace for this scene/camera. | Uses `config.Shadows` to decide whether to create a shadow-enabled workspace. |
| `void ClearScene()` | Removes the active compositor workspace. | Also removes any generated workspace/shadow-node definitions owned by that active workspace. Does not destroy the scene, camera, lights or items. |
| `void OnWindowResize()` | Notifies the Ogre render window of external-window size changes. | Call from the application resize pathway. |
| `void OnUpdate()` | Renders one frame, **if** a workspace is active. | Returns without rendering if none is bound; throws if Ogre reports frame rendering failure. |

### 2.3 `WorkspaceConfig`

```cpp
struct WorkspaceConfig
{
    std::optional<ShadowSettings> Shadows;
};
```

| Field | Default | Meaning |
|---|---|---|
| `Shadows` | `std::nullopt` | No shadow compositor node. If populated, the renderer creates a shadow-enabled workspace using its settings. |

There is no public `Shadow` class, `CreateShadow()` factory, shadow object handle, or separate shadow manager in Phase 3.

### 2.4 Workspace replacement behavior

1. Validates that scene and camera are Ogre-backed and that the camera belongs to the supplied scene.
2. If shadows are requested, validates shadow settings, creates a uniquely named compositor shadow-node definition, and creates a workspace definition using it.
3. Creates the replacement runtime workspace **before** removing the old runtime workspace.
4. Coordinates PBS/Unlit shadow-filter settings via the scene's `OgreMaterialSystem`.
5. Calls `ClearScene()` for the old workspace and installs the newly created workspace.
6. Removes any newly created definitions if creation fails. A failed creation is intended to leave the previous active workspace in place.

**Scope:** The renderer supports **one active compositor workspace** in the supplied version. Passing a new `ShadowSettings` structure does not dynamically modify an existing workspace; call `SetScene()` again to rebuild/select a replacement.

---

## 3. SceneManager API

**Header:** `Contracts/RenderSystem/SceneManager.h`  
**Creation function:**

```cpp
Owner<SceneManager> CreateSceneManager(
    const std::string& name,
    MaterialSystem& materials,
    std::size_t workerThreads = 0
);
```

Creates a named scene using the currently selected rendering backend. The `MaterialSystem&` is **non-owning**; the material system must outlive the scene. Empty names or duplicate Ogre scene names are rejected.

### 3.1 Camera management

| Method | Returns | Behavior |
|---|---|---|
| `Camera* CreateCamera(const std::string& name)` | Borrowed camera pointer | Creates and registers a native Ogre camera plus its public wrapper. Throws on empty/duplicate name. |
| `Camera* GetCamera(const std::string& name) const` | Pointer or `nullptr` | Finds a camera by scene-local name. |
| `void DestroyCamera(const std::string& name)` | — | Destroys the native camera and wrapper. Clear any renderer workspace using it first. |

### 3.2 Light creation and lookup

| Method | Returns | Behavior |
|---|---|---|
| `DirectionalLight* CreateDirectionalLight(const std::string& name, SceneNodeHandle parent = InvalidSceneNode)` | Borrowed directional-light pointer | Creates a directional light with its own transform child node. |
| `PointLight* CreatePointLight(const std::string& name, SceneNodeHandle parent = InvalidSceneNode)` | Borrowed point-light pointer | Creates a point light with its own transform child node. |
| `SpotLight* CreateSpotLight(const std::string& name, SceneNodeHandle parent = InvalidSceneNode)` | Borrowed spotlight pointer | Creates a spotlight with its own transform child node. |
| `Light* GetLight(const std::string& name) const` | Base pointer or `nullptr` | Type-neutral lookup. |
| `DirectionalLight* GetDirectionalLight(const std::string& name) const` | Typed pointer or `nullptr` | Returns null for missing name or wrong type. |
| `PointLight* GetPointLight(const std::string& name) const` | Typed pointer or `nullptr` | Returns null for missing name or wrong type. |
| `SpotLight* GetSpotLight(const std::string& name) const` | Typed pointer or `nullptr` | Returns null for missing name or wrong type. |
| `void DestroyLight(const std::string& name)` | — | Destroys native light and its private transform node; removes the wrapper record. Throws for unknown name. |

**Hierarchy:** `parent == InvalidSceneNode` means the native root node; otherwise the named light receives a **dedicated child node** of that parent. Moving/rotating a parent consequently affects the light's transform. The dedicated light node is **not** given a public `SceneNodeHandle`.

### 3.3 Ambient illumination

| Method | Returns | Behavior |
|---|---|---|
| `void SetAmbientLight(const AmbientLightProperties& properties)` | — | Applies hemisphere upper/lower ambient colors, direction, and environment scale to the native scene. |
| `AmbientLightProperties GetAmbientLight() const` | Value | Reads current native hemisphere ambient properties. |

See [Section 6](#6-ambient-illumination-api) for fields and constraints.

### 3.4 Scene node management

**Types:** `using SceneNodeHandle = std::uint64_t;` and `constexpr SceneNodeHandle InvalidSceneNode = 0;`

| Method | Returns | Behavior |
|---|---|---|
| `SceneNodeHandle CreateNode(const std::string& name, SceneNodeHandle parent = InvalidSceneNode)` | Handle | Creates a named node below parent or root; validates name and parent. |
| `SceneNodeHandle GetNode(const std::string& name) const` | Handle | Name lookup; returns `InvalidSceneNode` when missing. |
| `void DestroyNode(SceneNodeHandle node)` | — | Destroys a node **only if it has no native children and no attached objects**. |
| `void SetNodePosition(SceneNodeHandle node, float x, float y, float z)` | — | Sets local position. |
| `void SetNodeScale(SceneNodeHandle node, float x, float y, float z)` | — | Sets local scale. |
| `void SetNodeRotation(SceneNodeHandle node, float yaw, float pitch, float roll)` | — | Replaces node orientation using angles in **degrees**, composed as Y-yaw × X-pitch × Z-roll. |
| `SceneVector3 GetNodePosition(SceneNodeHandle node) const` | Value | Reads the node's local position. |

`SceneVector3` contains `float X`, `Y`, `Z` (all default 0). Missing/invalid handles throw for mutation or retrieval methods that require a node. A node containing a privately owned light transform node cannot be destroyed until the associated light is destroyed.

### 3.5 Mesh item management and shadow casting

**Types:** `using SceneItemHandle = std::uint64_t;` and `constexpr SceneItemHandle InvalidSceneItem = 0;`

| Method | Returns | Behavior |
|---|---|---|
| `SceneItemHandle CreateItem(const std::string& name, ResourceHandle mesh, SceneNodeHandle node)` | Handle | Builds an Ogre mesh item using a loaded mesh handle; attaches it to an existing node. |
| `SceneItemHandle GetItem(const std::string& name) const` | Handle | Returns registered item handle or `InvalidSceneItem`. |
| `void DestroyItem(SceneItemHandle item)` | — | Destroys the native item and clears its registration. |
| `void SetMaterial(SceneItemHandle item, MaterialHandle material)` | — | Applies a datablock to the whole item. |
| `void SetSubMaterial(SceneItemHandle item, std::size_t subItemIndex, MaterialHandle material)` | — | Applies a datablock to a specified subitem; checks index bounds. |
| `void SetItemCastShadows(SceneItemHandle item, bool enabled)` | — | Enables/disables this item's participation as a shadow caster. |
| `bool GetItemCastShadows(SceneItemHandle item) const` | `bool` | Reads native item shadow-casting flag. |

`SetItemCastShadows` controls **casting**; it does not control whether a PBS material **receives** shadows (see `MaterialSystem::SetReceiveShadows`).

---

## 4. Camera API

**Header:** `Contracts/RenderSystem/Camera.h`  
**Creation/ownership:** `SceneManager::CreateCamera`, not an independent camera factory.

**Return type:** `CameraVector3` has `float X`, `Y`, and `Z`.

### 4.1 Transform and orientation

| Method | What it does |
|---|---|
| `void SetPosition(float x, float y, float z)` | Sets camera position. |
| `CameraVector3 GetPosition() const` | Returns camera position. |
| `void Move(float x, float y, float z)` | Moves camera by a vector passed to native `Ogre::Camera::move`. |
| `void MoveRelative(float x, float y, float z)` | Moves in the camera's local orientation frame. |
| `void LookAt(float x, float y, float z)` | Points camera toward a world target. |
| `void SetDirection(float x, float y, float z)` | Sets its viewing direction. |
| `void Yaw(float degrees)` | Rotates around the native camera yaw axis, in degrees. |
| `void Pitch(float degrees)` | Pitches camera in degrees. |

### 4.2 Projection

| Method | What it does | Validation in supplied Ogre implementation |
|---|---|---|
| `void SetPerspective(float fovYDegrees, float aspectRatio, float nearClip, float farClip)` | Selects perspective and sets vertical FOV, aspect and near/far clipping. | `0 < FOV < 179`; aspect > 0; near > 0; far == 0 or far > near. |
| `void SetOrthographic(float width, float height, float nearClip, float farClip)` | Selects orthographic projection with requested view dimensions. | Width/height > 0; near > 0; far > near. |
| `void SetAspectRatio(float aspectRatio)` | Disables auto aspect and sets manual ratio. | Aspect > 0. |
| `void SetAutoAspectRatio(bool enabled)` | Toggles Ogre's automatic aspect handling. | No extra validation. |

---

## 5. Light APIs

**Header:** `Contracts/RenderSystem/Light.h`  
**Inheritance:** `Light` → `DirectionalLight`; `Light` → `PositionalLight` → (`PointLight`, `SpotLight`).

### 5.1 Light value types

| Type | Fields / cases | Meaning |
|---|---|---|
| `enum class LightType` | `Directional`, `Point`, `Spot` | Identifies a public light subtype. |
| `struct LightColor` | `float R = 1`, `G = 1`, `B = 1` | RGB light color, no alpha. |
| `struct LightVector3` | `float X = 0`, `Y = 0`, `Z = 0` | Position/direction vector. |
| `struct AttenuationProperties` | `Range = 100`, `Constant = 1`, `Linear = 0`, `Quadratic = 0` | Distance attenuation values (the struct's defaults, not a promise about native light initial state). |

### 5.2 `Light` — common contract

All directional, point and spot lights expose these methods:

| Method | Return | Function |
|---|---|---|
| `LightType GetType() const` | Type | Reports `Directional`, `Point`, or `Spot`. |
| `void SetDiffuseColor(const LightColor& color)` | — | Sets the native diffuse contribution of the light. |
| `LightColor GetDiffuseColor() const` | Color | Reads native diffuse color. |
| `void SetSpecularColor(const LightColor& color)` | — | Sets the native specular contribution. |
| `LightColor GetSpecularColor() const` | Color | Reads native specular color. |
| `void SetPowerScale(float scale)` | — | Sets nonnegative light intensity multiplier. Throws if negative/NaN. |
| `float GetPowerScale() const` | `float` | Reads intensity multiplier. |
| `void SetCastShadows(bool enabled)` | — | Marks this light eligible to cast shadows when a suitable shadow workspace is active. |
| `bool GetCastShadows() const` | `bool` | Reads native casting flag. |
| `void SetVisible(bool visible)` | — | Sets native light visibility. |
| `bool IsVisible() const` | `bool` | Reads native visibility. |

**Note:** Turning on `SetCastShadows(true)` alone does **not** create shadow maps. A shadow-enabled `WorkspaceConfig` must also be bound and suitable shadow slots must be configured.

### 5.3 `DirectionalLight` — sunlight / parallel light

Inherits all `Light` operations plus:

| Method | What it does |
|---|---|
| `void SetDirection(float x, float y, float z)` | Sets light direction. Rejects zero-length direction; supplied Ogre wrapper normalizes the input. |
| `LightVector3 GetDirection() const` | Reads the native light direction. |

A directional light does **not** expose position or attenuation properties. Its own transform node is used to represent orientation, and an optional parent can organize its hierarchy.

### 5.4 `PositionalLight` — shared abstract point/spot contract

Inherits `Light` and adds:

| Method | What it does | Constraints |
|---|---|---|
| `void SetPosition(float x, float y, float z)` | Sets the dedicated transform node's **local** position. | No extra numeric validation in wrapper. |
| `LightVector3 GetPosition() const` | Reads local position from that transform node. | — |
| `void SetAttenuation(float range, float constant, float linear, float quadratic)` | Explicitly sets attenuation range and three coefficients. | Range > 0; coefficients >= 0; at least one coefficient > 0. |
| `void SetAttenuationByRadius(float radius, float threshold)` | Requests Ogre's radius-based attenuation configuration. | Radius > 0; 0 < threshold < 1. |
| `AttenuationProperties GetAttenuation() const` | Reads native range and three attenuation coefficients. | — |

### 5.5 `PointLight` — omnidirectional positional light

`PointLight` is an empty specialization of `PositionalLight`: it has **all common Light methods** plus **all PositionalLight methods**, with no extra public methods. Its native Ogre light type is `LT_POINT`.

### 5.6 `SpotLight` — cone-shaped positional light

Inherits all `Light` and `PositionalLight` methods, plus:

| Method | What it does | Constraints |
|---|---|---|
| `void SetDirection(float x, float y, float z)` | Sets spotlight direction via native Ogre light. | Zero vector rejected; normalized by wrapper. |
| `LightVector3 GetDirection() const` | Reads native direction. | — |
| `void SetSpotlightRange(float innerAngleDegrees, float outerAngleDegrees, float falloff = 1.0f)` | Sets inner cone, outer cone and transition falloff. | Inner >= 0; outer >= inner; outer < 180 degrees; falloff > 0. |
| `float GetSpotlightInnerAngle() const` | Reads inner cone angle in degrees. | — |
| `float GetSpotlightOuterAngle() const` | Reads outer cone angle in degrees. | — |
| `float GetSpotlightFalloff() const` | Reads falloff value. | — |

**Direction coordinate caution:** `GetDirection()`/`SetDirection()` are direct wrappers of Ogre light direction behavior; unlike `SetPosition()`, the public contract does not provide a separate world/local direction selector. Ogre may use the attached transform node's orientation. Test nested-node direction behavior in the Sandbox if the distinction matters to gameplay.

---

## 6. Ambient illumination API

**Type:** `AmbientLightProperties` (declared in `SceneManager.h`).

```cpp
struct AmbientLightProperties
{
    LightColor UpperColor{ 0.0f, 0.0f, 0.0f };
    LightColor LowerColor{ 0.0f, 0.0f, 0.0f };
    LightVector3 HemisphereDirection{ 0.0f, 1.0f, 0.0f };
    float EnvironmentScale = 1.0f;
};
```

| Property | Role |
|---|---|
| `UpperColor` | Ambient color of the upper hemisphere. |
| `LowerColor` | Ambient color of the lower hemisphere. |
| `HemisphereDirection` | Direction defining the upper/lower hemispheres. Must be nonzero when setting. |
| `EnvironmentScale` | Ambient environment scale; must be nonnegative. |

**Public operations:** `SceneManager::SetAmbientLight(properties)` and `GetAmbientLight()`.

The Ogre backend forwards this information to native `Ogre::SceneManager::setAmbientLight()` and its matching native getters. This is **hemisphere ambient lighting**, not a global-illumination solver, sky environment texture system, or a separate `IlluminationManager`.

---

## 7. Shadow configuration API

**Header:** `Contracts/RenderSystem/Shadow.h`.

### 7.1 `ShadowFilter`

```cpp
enum class ShadowFilter
{
    PCF_2x2, PCF_3x3, PCF_4x4, PCF_5x5, PCF_6x6, ESM
};
```

PCF options select a percentage-closer filtering variant for PBS shadow sampling; `ESM` requests exponential shadow maps and enables the `EsmK` setting. Filter selection is coordinated with the Ogre HLMS PBS/Unlit providers, rather than configured per material.

### 7.2 `ShadowSettings` fields and defaults

| Field (`type`) | Default | What it configures |
|---|---:|---|
| `DirectionalResolution` (`uint32_t`) | `2048` | Shadow-map resolution per directional PSSM split. `0` disables directional shadows. |
| `SpotResolution` (`uint32_t`) | `1024` | Resolution of each configured spot shadow-map slot. |
| `PointResolution` (`uint32_t`) | `512` | Resolution of each configured point shadow-map slot / point-light helper input. |
| `PssmSplits` (`uint8_t`) | `3` | Directional PSSM cascade count; must be 2–4 when directional maps are enabled. |
| `MaxSpotLights` (`uint8_t`) | `2` | Number of spot-shadow slots; 0 disables spot shadows. |
| `MaxPointLights` (`uint8_t`) | `1` | Number of point-shadow slots; 0 disables point shadows. |
| `PssmLambda` (`float`) | `0.95` | Distribution parameter for directional cascade splits (0–1). |
| `SplitPadding` (`float`) | `1.0` | Extra padding for split calculations; >= 0. |
| `SplitBlend` (`float`) | `0.125` | Cascade blend setting (0–1). |
| `SplitFade` (`float`) | `0.313` | Cascade fade setting (0–1). |
| `Filter` (`ShadowFilter`) | `PCF_3x3` | PBS shadow filter mode. |
| `EsmK` (`uint16_t`) | `600` | Exponential-shadow-map tuning value; must be > 0 for ESM. |

### 7.3 Validation performed by `OgreRenderer`

- If an enabled category requires a resolution, it must be in `1..8192`.
- If directional shadow mapping is enabled, `PssmSplits` must be in `2..4`.
- `MaxSpotLights + MaxPointLights <= 16`.
- The configuration must request **at least one** shadow-map category (directional, spot, or point).
- `PssmLambda`, `SplitBlend`, `SplitFade` must each be `0..1`; `SplitPadding >= 0`.
- When `Filter == ShadowFilter::ESM`, `EsmK` must be positive.

### 7.4 What the backend creates

- A PSSM directional shadow parameter group if `DirectionalResolution > 0`.
- One focused shadow parameter per requested spot-shadow slot.
- One focused shadow parameter per requested point-shadow slot.
- A compositor shadow-node definition through Ogre's `ShadowNodeHelper`.
- A shadow-aware compositor workspace definition referencing that shadow node.
- Matching HLMS PBS/Unlit filtering configuration via `OgreMaterialSystem::ApplyShadowSettings`.

**Not an individual-light API:** `ShadowSettings` describes workspace shadow capacity and sampling, not a specific light's shadow object. Which actual scene lights cast shadows is controlled separately with `Light::SetCastShadows` and Ogre's runtime light selection.

**Verification boundary:** The supplied Phase 3 implementation was not verified against a real Ogre SDK build or Vulkan runtime, so configured point/spot/directional shadow output must be visually tested. This reference describes what the code is written to request, not guaranteed successful runtime rendering.

---

## 8. MaterialSystem API

**Header:** `Contracts/RenderSystem/MaterialSystem.h`  
**Creation:** `Owner<MaterialSystem> CreateMaterialSystem(const std::string& hlmsPath);`

`hlmsPath` points to the Ogre media root containing the `Hlms` folder; the backend registers/reuses Ogre Unlit and PBS HLMS providers.

### 8.1 Types

| Type | Members |
|---|---|
| `enum class MaterialType` | `Unlit`, `PBS` |
| `enum class MaterialTexture` | `BaseColor`, `Normal`, `Roughness`, `Metalness` |
| `struct MaterialColor` | `float R=1`, `G=1`, `B=1`, `A=1` |
| `using MaterialHandle` | `std::uint64_t` |
| `InvalidMaterial` | `0` |

### 8.2 Registration, lookup, destruction

| Method | What it does |
|---|---|
| `MaterialHandle RegisterMaterial(MaterialType type, const std::string& name)` | Creates a named Unlit or PBS native datablock; returns handle. Rejects empty or duplicate name. PBS starts with metallic workflow, roughness 0.5 and metalness 0. |
| `MaterialHandle GetMaterial(const std::string& name) const` | Returns handle or `InvalidMaterial` if not found. |
| `void DestroyMaterial(MaterialHandle material)` | Destroys datablock/registration; refuses when linked renderables still use it. |

### 8.3 Surface properties and textures

| Method | What it does | Applicability / constraints |
|---|---|---|
| `void SetColor(MaterialHandle material, const MaterialColor& color)` | Sets Unlit color or PBS diffuse RGB. | For PBS, native call uses RGB; alpha is not forwarded in the supplied setter. |
| `void SetRoughness(MaterialHandle material, float roughness)` | Sets PBS surface roughness. | PBS only; **(0, 1]**. |
| `void SetMetalness(MaterialHandle material, float metalness)` | Sets PBS surface metalness. | PBS only; **[0, 1]**. |
| `void SetTexture(MaterialHandle material, MaterialTexture slot, const std::string& filename)` | Binds named Ogre texture to selected slot. | Unlit supports `BaseColor` only. PBS supports all four declared slots. |
| `void SetReceiveShadows(MaterialHandle material, bool enabled)` | Enables/disables shadow reception on a PBS datablock. | PBS only; **no public getter** exists in supplied API. |

The public material texture API takes a **filename**, not a `ResourceHandle`. The `ResourceManager` separately returns handles for loaded textures; this distinction exists in the supplied code and is not silently normalized here.

### 8.4 Material vs. light vs. item shadow controls

| API | Controls |
|---|---|
| `Light::SetCastShadows(true)` | Whether the light can contribute shadow maps. |
| `SceneManager::SetItemCastShadows(item, true)` | Whether a mesh item casts shadows. |
| `MaterialSystem::SetReceiveShadows(material, true)` | Whether a PBS surface receives shadows. |
| `WorkspaceConfig::Shadows` | Whether the active compositor workspace has a shadow-rendering pipeline and its capacity/filtering. |

All four concepts are different; enabling one does not substitute for enabling the others.

---

## 9. ResourceManager API

**Header:** `Contracts/RenderSystem/ResourceManager.h`  
**Access:** `ResourceManager::Get()` singleton; `ResourceManager::Shutdown()` destroys its singleton instance.

### 9.1 Types

| Type | Definition |
|---|---|
| `using ResourceHandle` | `std::uint64_t` |
| `InvalidResource` | `0` |
| `enum class TextureUsage` | `ColorSRGB`, `Linear`, `Normal` |

### 9.2 Resource group and loading methods

| Method | What it does |
|---|---|
| `static ResourceManager& Get()` | Lazily constructs/returns current resource manager, requiring an initialized Ogre render system. |
| `static void Shutdown()` | Releases the singleton wrapper/registry. It is not an automatic deep eviction of all Ogre native resources. |
| `void AddResourceLocation(const std::string& path, const std::string& group = "General", bool recursive = false)` | Adds an existing filesystem folder to an Ogre resource group; optionally recurses. |
| `ResourceHandle LoadMesh(const std::string& name, const std::string& group = "General")` | Looks up or loads a mesh, then returns its registered handle. |
| `ResourceHandle LoadTexture(const std::string& name, const std::string& group = "General", TextureUsage usage = TextureUsage::ColorSRGB)` | Looks up or loads a texture, applying usage-specific flags; waits for texture data to be ready in the supplied backend. |
| `ResourceHandle GetMesh(const std::string& name, const std::string& group = "General") const` | Looks up registered mesh handle or returns `InvalidResource`. |
| `ResourceHandle GetTexture(const std::string& name, const std::string& group = "General") const` | Looks up registered texture handle or returns `InvalidResource`. |
| `bool HasResource(ResourceHandle handle) const` | Checks whether the handle is still registered. |
| `void ReleaseResource(ResourceHandle handle)` | Removes a registration. The backend intentionally does not forcibly destroy native resources still referenced by other objects. |

### 9.3 Resource behavior

- Handles are assigned locally in the manager and cached by `(group, name)`.
- Loading the same registered resource returns the existing handle.
- Re-loading a texture with a different `TextureUsage` throws.
- The supplied Ogre implementation rejects ambiguous reuse of the same texture filename across different groups.
- `LoadTexture()` currently waits for texture residency/data; it is **synchronous** from the caller's point of view.
- Releasing a registration is not the same operation as guaranteed GPU-memory eviction.

---

## 10. Concrete Ogre classes (internal/backend APIs)

This section is for engine contributors. Most client code should use the `Contracts/RenderSystem/...` headers instead of including `Concrete/.../Ogre` headers.

### 10.1 `OgreRenderer final : Renderer`

**Files:** `Concrete/RenderSystem/Ogre/OgreRenderer.h/.cpp`

**Public overrides:** `SetScene(scene, camera)`, `SetScene(scene, camera, config)`, `ClearScene`, `OnWindowResize`, `OnUpdate`, destructor. These implement the matching public contract (Section 2).

**Private methods:**

| Method | Role |
|---|---|
| `explicit OgreRenderer(Window* window)` | Initializes Ogre Root, installs Vulkan plugin, chooses render system, creates external window, initializes compositor. Constructed through `RendererBuilder`. |
| `void InitWindow(Window* window)` | Obtains native Win32 GLFW window handle and creates Ogre external render window. |
| `void CreateShadowNode(const std::string& name, const ShadowSettings& settings)` | Validates settings and translates them to Ogre `ShadowNodeHelper` parameters. |
| `void Shutdown()` | Removes active workspace, destroys render window, releases Ogre Root and Vulkan plugin. |

**Held state:** `Owner<Ogre::VulkanPlugin> m_VulkanPlugin`, `Owner<Ogre::Root> m_Root`, native render window pointer, compositor pointer, workspace pointer, generated workspace/shadow-node names, monotonically increasing workspace id.

**Backend limits:** The supplied `InitWindow()` currently supports the Windows GLFW native handle branch; the selected rendering API is Vulkan. Basic workspace background color is hard-coded in the implementation. One compositor workspace is active at a time.

### 10.2 `OgreSceneManager final : SceneManager`

**Files:** `Concrete/RenderSystem/Ogre/SceneManager.h/.cpp`

**Public overrides:** all camera, light, ambient, node, item and shadow-casting methods from Section 3.

**Backend-only getters:**

| Method | Role |
|---|---|
| `Ogre::SceneManager* GetOgreSceneManager() const` | Lets renderer access native Ogre scene during workspace creation. |
| `OgreMaterialSystem* GetOgreMaterialSystem() const` | Lets renderer coordinate PBS/Unlit shadow settings with material backend. |

**Private helpers:** `Light* CreateLight(name, parent, type)` (shared creation routine, **not a factory object**); `Ogre::SceneNode* RequireNode(handle) const`; `Ogre::Item* RequireItem(handle) const`.

**Record types maintained internally:**

- `CameraRecord`: `Owner<Camera> Wrapper`, `Ogre::Camera* Native`.
- `LightRecord`: `Owner<Light> Wrapper`, `Ogre::Light* Native`, `Ogre::SceneNode* TransformNode`.
- `NodeRecord`: node name and `Ogre::SceneNode* Native`.
- `ItemRecord`: item name and `Ogre::Item* Native`.

**Registries:** cameras/lights by name, nodes/items by numeric handle, plus node/item name→handle maps. Node/item counters start at 1. Native scene and material-system pointers are non-owning in this class. The destructor clears records and destroys native Ogre scene; its comment requires compositor workspaces using the scene to be removed first.

### 10.3 `OgreCamera final : Camera`

**Files:** Original `RenderSystem/Ogre/Camera.h/.cpp`.

Implements every `Camera` method in Section 4 by forwarding to a **non-owning** `Ogre::Camera*`. A backend-only `Ogre::Camera* GetOgreCamera() const` lets `OgreRenderer` obtain the native camera for workspace binding. The constructor validates non-null native camera pointer.

### 10.4 `OgreLightCommon` and final light wrappers

**Files:** `Concrete/RenderSystem/Ogre/Light.h/.cpp`.

`OgreLightCommon` is a **shared implementation helper**, not a public polymorphic `Light` base and not a native-object owner. It stores non-owning `Ogre::Light* m_Light` and `Ogre::SceneNode* m_Node`.

**Helper methods:** `SetDiffuseColor`, `GetDiffuseColor`, `SetSpecularColor`, `GetSpecularColor`, `SetPowerScale`, `GetPowerScale`, `SetCastShadows`, `GetCastShadows`, `SetVisible`, `IsVisible`, `SetPosition`, `GetPosition`, `SetDirection`, `GetDirection`, `SetAttenuation`, `SetAttenuationByRadius`, `GetAttenuation`, `SetSpotlightRange`, `GetSpotlightInnerAngle`, `GetSpotlightOuterAngle`, `GetSpotlightFalloff`.

| Concrete class | Public contract | Delegates |
|---|---|---|
| `OgreDirectionalLight final` | `DirectionalLight` | All common properties + direction to `m_Common`. `GetType()` returns `Directional`. |
| `OgrePointLight final` | `PointLight` | All common properties + position and attenuation. `GetType()` returns `Point`. |
| `OgreSpotLight final` | `SpotLight` | All common properties + position, attenuation, direction and cone settings. `GetType()` returns `Spot`. |

The native light type is assigned in `OgreSceneManager::CreateLight` as `LT_DIRECTIONAL`, `LT_POINT`, or `LT_SPOTLIGHT`. The scene manager owns the native light and its dedicated child transform node.

### 10.5 `OgreMaterialSystem : MaterialSystem`

**Files:** `Concrete/RenderSystem/Ogre/MaterialSystem.h/.cpp`.

**Public overrides:** all methods in Section 8. Also exposes backend-only methods:

| Method | Role |
|---|---|
| `void ApplyShadowSettings(const ShadowSettings& settings)` | Maps `ShadowFilter` to Ogre PBS filter, configures Unlit ESM toggle and ESM K as applicable. Renderer calls it while selecting the workspace; client code does not need to. |
| `Ogre::HlmsDatablock* GetOgreDatablock(MaterialHandle material) const` | Gives OgreSceneManager the native datablock used when applying materials to renderables. |

**Private helpers:** `MaterialRecord& Require(handle)` and its const overload validate handles. `MaterialRecord` stores material type, name, and native datablock pointer.

**Held state:** `Ogre::HlmsManager*`, `Ogre::HlmsPbs*`, `Ogre::HlmsUnlit*`, handle→material records, name→handle map, next handle counter.

**Shared-state note:** PBS/Unlit filter settings apply to their **HLMS instances**, not independently per workspace/material. Avoid expecting two active workspaces to use incompatible filter settings through the same shared provider.

### 10.6 `OgreResourceManager final : ResourceManager`

**Files:** Original `RenderSystem/Ogre/ResourceManager.h/.cpp`.

**Public overrides:** all methods in Section 9. Backend-only native access:

| Method | Role |
|---|---|
| `Ogre::MeshPtr GetOgreMesh(ResourceHandle handle) const` | Validates mesh handle and returns native mesh used by `OgreSceneManager::CreateItem`. |
| `Ogre::TextureGpu* GetOgreTexture(ResourceHandle handle) const` | Validates texture handle and returns native GPU texture pointer. |

**Private helpers:** `MakeKey(name, group)`, `PrepareGroup(group)`, `Require(handle, Kind type)`. `ResourceRecord` tracks mesh or texture kind, filename, group, texture usage and native resource.

**Held state:** native resource-group manager, native texture manager, handle→resource map, resource-name caches and next handle counter.

### 10.7 Public construction vs. backend construction

| Public entry point | Internal construction target |
|---|---|
| `RendererBuilder::Build()` | `OgreRenderer` through `BuildRenderer` |
| `CreateSceneManager(...)` | `OgreSceneManager` |
| `CreateMaterialSystem(hlmsPath)` | `OgreMaterialSystem` |
| `ResourceManager::Get()` | Lazily constructed `OgreResourceManager` |
| `SceneManager::CreateCamera(name)` | `OgreCamera` wrapper + native Ogre camera |
| `SceneManager::Create*Light(name, parent)` | Typed Ogre light wrapper + native light and private node |

No independent camera/light/shadow factory object is required by the supplied API.

---

## 11. Typical usage sequences

The examples below use engine objects (`window`, `renderer`, `materials`) whose creation/dependencies are intentionally shown only where relevant. Names and property signatures match the supplied contracts.

### 11.1 Create scene, camera, geometry and PBS material

```cpp
// Renderer is created from an existing plagc Window*.
auto renderer = RendererBuilder().WindowHandle(window).Build();
auto materials = CreateMaterialSystem(hlmsMediaRoot);
auto scene = CreateSceneManager("Demo", *materials);

ResourceManager& resources = ResourceManager::Get();
resources.AddResourceLocation(assetDirectory);
ResourceHandle mesh = resources.LoadMesh("Cube.mesh");

SceneNodeHandle node = scene->CreateNode("CubeNode");
SceneItemHandle item = scene->CreateItem("Cube", mesh, node);
MaterialHandle stone = materials->RegisterMaterial(MaterialType::PBS, "Stone");
materials->SetColor(stone, { 0.8f, 0.7f, 0.6f, 1.0f });
materials->SetRoughness(stone, 0.65f);
materials->SetMetalness(stone, 0.0f);
scene->SetMaterial(item, stone);

Camera* camera = scene->CreateCamera("MainCamera");
camera->SetPosition(0.0f, 2.0f, 8.0f);
camera->LookAt(0.0f, 0.0f, 0.0f);
camera->SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
renderer->SetScene(*scene, *camera);
```

*Asset filenames, mesh format and media folder layout must match the actual project. Do not treat the example path names as verified available assets.*

### 11.2 Add all three light types

```cpp
SceneNodeHandle room = scene->CreateNode("Room");

auto* sun = scene->CreateDirectionalLight("Sun");
sun->SetDirection(-0.5f, -1.0f, -0.5f);
sun->SetDiffuseColor({ 1.0f, 0.94f, 0.85f });
sun->SetPowerScale(2.0f);
sun->SetCastShadows(true);

auto* lamp = scene->CreatePointLight("Lamp", room);
lamp->SetPosition(0.0f, 3.0f, 0.0f);
lamp->SetAttenuationByRadius(10.0f, 0.01f);
lamp->SetDiffuseColor({ 1.0f, 0.72f, 0.45f });

auto* torch = scene->CreateSpotLight("Torch", room);
torch->SetPosition(2.0f, 3.0f, 0.0f);
torch->SetDirection(0.0f, -1.0f, 0.0f);
torch->SetSpotlightRange(20.0f, 35.0f, 1.0f);
```

### 11.3 Configure ambient hemisphere light

```cpp
AmbientLightProperties ambient;
ambient.UpperColor = { 0.4f, 0.5f, 0.65f };
ambient.LowerColor = { 0.1f, 0.08f, 0.06f };
ambient.HemisphereDirection = { 0.0f, 1.0f, 0.0f };
ambient.EnvironmentScale = 1.0f;
scene->SetAmbientLight(ambient);
```

### 11.4 Enable shadows for the current workspace

```cpp
scene->SetItemCastShadows(item, true);
materials->SetReceiveShadows(stone, true);
sun->SetCastShadows(true);

WorkspaceConfig config;
config.Shadows = ShadowSettings{};
config.Shadows->DirectionalResolution = 2048;
config.Shadows->PssmSplits = 3;
config.Shadows->Filter = ShadowFilter::PCF_3x3;

renderer->SetScene(*scene, *camera, config);
```

### 11.5 Change or disable shadows

```cpp
// Changes to structural shadow settings require a new workspace.
config.Shadows->DirectionalResolution = 4096;
renderer->SetScene(*scene, *camera, config);

// Revert to a basic no-shadow workspace without removing scene contents.
renderer->SetScene(*scene, *camera);
```

### 11.6 Basic rendering and cleanup pattern

```cpp
// In your application's frame loop:
renderer->OnUpdate();

// Once rendering is finished, destroy scene dependencies in safe order:
renderer->ClearScene();
scene.reset();
materials.reset();
ResourceManager::Shutdown();
renderer.reset();
```

This cleanup sketch assumes no other scenes/material users remain and that renderer owns the active Ogre Root. Destroying materials before all linked items are gone is invalid; destroying the renderer/root before scenes and material/resource managers is unsafe.

---

## 12. Lifecycle, errors and limits

### 12.1 Return conventions

| API family | Missing-object behavior | Invalid-operation behavior |
|---|---|---|
| `GetCamera` / `GetLight` / typed `Get*Light` | `nullptr` | Typed getters also return `nullptr` on type mismatch. |
| `GetNode` / `GetItem` | `InvalidSceneNode` / `InvalidSceneItem` | — |
| `GetMaterial` / `GetMesh` / `GetTexture` | `InvalidMaterial` / `InvalidResource` | — |
| `CreateCamera`, `CreateLight`, `CreateNode`, `CreateItem`, `RegisterMaterial` | — | Throws for invalid/duplicate names or invalid dependencies. |
| `RequireNode`, `RequireItem`, material/resource handle operations | — | Throws for invalid/unknown handles. |
| Camera/light property setters | — | Selected setters validate input; consult Sections 4–7. |
| `Renderer::SetScene` | — | Throws for incompatible Ogre objects, cross-scene camera, bad shadow config or backend creation failure. |

### 12.2 Lifetimes and ordering

1. Create a `Renderer` (Ogre Root/render system) before Ogre-dependent material, scene or resource manager instances.
2. Keep the `MaterialSystem` alive for as long as scenes created with its reference exist.
3. Keep a camera and scene alive while the renderer's active workspace uses them.
4. Destroy child items/lights before destroying parent scene nodes.
5. Release the compositor workspace with `Renderer::ClearScene()` before destroying its scene/camera.
6. Destroy scenes/items before materials they reference.
7. Call `ResourceManager::Shutdown()` before destroying the Ogre Root/renderer, after resource consumers are finished.
8. Do not manually delete borrowed camera/light pointers or backend-owned native objects.

### 12.3 What is and is not implemented in these files

**Implemented in the supplied API:** directional, point and spot light controls; scene-owned lights; ambient hemisphere color/direction; PBS and Unlit materials; item/material shadow flags; compositor shadow-node/workspace creation; PSSM directional split configuration; configured spot/point shadow slots; PCF and ESM filter selection; handle-based meshes/items/materials/resources; workspace swapping.

**Not provided by the supplied public API:** independent shadow-object factory; dedicated `LightingManager`; per-light public shadow-map allocation/priority control; light handles (`Light*` pointers are used); public getters for material shadow reception; public setter for workspace background color; world-space variants of light transform setters; per-workspace independent HLMS filter state; image-based lighting/GI integration; general HDR/bloom/SSAO workspace API.

### 12.4 Verification status

The source package notes report C++20 contract and stub-based syntax checks; they do **not** demonstrate a full build against the installed Ogre SDK or actual Vulkan rendering. Therefore, test especially: native shadow-node API compatibility, visible directional/spot/point shadows, texture/material reception, workspace switching, resizing, and lifetime shutdown paths.

---

## 13. Header and source index

### Public contract headers

| Header | Defines |
|---|---|
| `Contracts/RenderSystem/Renderer.h` | `Renderer`, `RendererBuilder`, `WorkspaceConfig` |
| `Contracts/RenderSystem/SceneManager.h` | `SceneManager`, scene handles, `SceneVector3`, `AmbientLightProperties`, `CreateSceneManager` |
| `Contracts/RenderSystem/Camera.h` | `Camera`, `CameraVector3` |
| `Contracts/RenderSystem/Light.h` | Light hierarchy, colors/vectors, attenuation, type enumeration |
| `Contracts/RenderSystem/Shadow.h` | `ShadowSettings`, `ShadowFilter` |
| `Contracts/RenderSystem/MaterialSystem.h` | `MaterialSystem`, types, handle and `CreateMaterialSystem` |
| `Contracts/RenderSystem/ResourceManager.h` | `ResourceManager`, types, handle and singleton operations |

### Backend headers and sources

| Implementation files | Implements |
|---|---|
| `Concrete/RenderSystem/Ogre/OgreRenderer.h/.cpp` | Ogre root, Vulkan plugin, external window, compositor/workspaces, shadow-node definitions |
| `Concrete/RenderSystem/Ogre/SceneManager.h/.cpp` | Scene-owned cameras, lights, nodes, items, ambient light, shadow casting flags |
| `Concrete/RenderSystem/Ogre/Light.h/.cpp` | `OgreLightCommon`, final directional/point/spot light wrappers |
| `Concrete/RenderSystem/Ogre/MaterialSystem.h/.cpp` | PBS/Unlit material datablocks, shadow filtering/reception |
| `Concrete/RenderSystem/Ogre/Camera.h/.cpp` | Camera wrapper and native camera bridge (from original supplied renderer files) |
| `Concrete/RenderSystem/Ogre/ResourceManager.h/.cpp` | Mesh/texture resource cache and native bridges (from original supplied renderer files) |

**Revision note:** This documentation intentionally preserves current names, method signatures, default values, missing getters, and limitations. It is a reference for the supplied code—not a proposal to redesign the API.
