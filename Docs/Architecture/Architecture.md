# plagc Architecture

This document defines the high-level architecture, system boundaries, dependency direction, and design philosophy of **plagc**.

It is intended to act as the architectural baseline for future engine development.

Implementation details, APIs, data layouts, scheduling algorithms, renderer internals, and subsystem-specific decisions may evolve without changing the high-level structure defined here.

---

# 1. Architectural Goals

plagc is designed around three primary goals:

## Cross-platform

The engine should expose platform-independent interfaces while isolating platform-specific implementation details.

Initial development focuses on:

- Windows
- Linux

Additional platforms may be targeted later where technically practical.

---

## Fast

Performance-sensitive systems should be designed around:

- multithreading
- worker pools
- job stealing
- data-oriented organization
- cache locality
- explicit ownership
- controlled synchronization
- custom memory allocation strategies

Performance is treated as part of the engine architecture rather than as a later optimization phase.

---

## Lightweight

plagc should provide the systems required to build 3D games without attempting to support every possible engine feature.

The engine should prefer:

- focused systems
- explicit interfaces
- limited coupling
- understandable ownership
- minimal unnecessary abstraction
- replaceable subsystem implementations

---

# 2. High-Level Architecture

The engine is organized into four conceptual layers.

```text
                         ┌───────────────────────────────────────────┐
                         │                   CORE                    │
                         │                                           │
                         │  Window              Logging              │
                         │  Event / Input       Rendering            │
                         │  ECS                 Physics              │
                         │  Asset Manager       Animation            │
                         │                                           │
                         │          Application / Core API            │
                         └───────────────────────────────────────────┘
                                  ▲              ▲              ▲
                                  │              │              │
                              uses Core      uses Core      uses Core
                                  │              │              │

                    ┌─────────────┘              │              └─────────────┐
                    │                            │                            │
          ┌──────────────────┐       ┌──────────────────┐       ┌─────────────────────┐
          │  Editor Backend  │──────▶│ Gameplay Systems │       │ Sandbox / Test      │
          └────────┬─────────┘ uses  └─────────┬────────┘       │ Environment         │
                   │                           │                 └─────────────────────┘
                   │ manages                   │ used by
                   │                           │
                   │                   ┌───────▼────────┐
                   │                   │  Game Runtime  │
                   │                   └────────────────┘
                   │
                   │
          ┌────────▼─────────┐
          │ Editor UI /      │
          │ Frontend         │
          └──────────────────┘
                   ▲
                   │
               User uses

                         User also uses Game Runtime
```

The exact rendering of the diagram may change, but the dependency direction defined by it is architectural.

---

# 3. Dependency Structure

The architecture follows these primary relationships.

## Direct Core clients

Three systems directly depend on the Core:

- Editor Backend
- Gameplay Systems
- Sandbox / Test Environment

Each consumes engine functionality through defined Core APIs.

They should not bypass those interfaces to depend directly on unrelated subsystem implementation details.

---

## Editor Backend -> Gameplay Systems

The Editor Backend may use Gameplay Systems.

This allows editor functionality to:

- inspect gameplay state
- edit gameplay-related data
- interact with gameplay entities
- control gameplay-related runtime behavior
- expose gameplay information to editor tools

Gameplay Systems do not exist inside the editor.

They remain a separate architectural layer that the editor may consume.

---

## Editor UI / Frontend -> Editor Backend

The Editor UI depends strictly on the Editor Backend.

The frontend is responsible for presentation and user interaction.

It should not directly manipulate engine internals.

The expected interaction is:

```text
Editor UI
    ↓
Editor Backend
    ↓
Engine / Gameplay functionality
```

This separation keeps rendering of the editor interface independent from editor logic and engine orchestration.

---

## Game Runtime -> Gameplay Systems

The Game Runtime consumes Gameplay Systems.

Gameplay Systems contain the game-facing functionality required to execute the game.

The runtime is therefore responsible for running the game rather than defining engine infrastructure.

---

## Editor Backend -> Game Runtime

The Editor Backend manages Game Runtime during editor-driven workflows.

Examples include:

- start
- stop
- pause
- restart
- inspect
- control runtime state
- enter or exit play mode

This relationship represents runtime orchestration.

It does not mean that Game Runtime is implemented as part of the Editor Backend.

---

## User relationships

The user interacts with two primary environments:

### Editor UI

Used when developing and editing a game.

### Game Runtime

Used when running or playing a built game.

The editor is therefore not required for a distributed game runtime.

---

# 4. Core

The Core contains reusable engine functionality.

It should remain independent from:

- Editor UI
- editor-specific workflows
- individual games
- project-specific gameplay logic

Higher layers consume the Core through defined interfaces.

---

# 5. Window System

The Window system owns platform window creation and management.

Responsibilities include:

- native window creation
- lifecycle management
- resize handling
- platform-specific window configuration
- integration with rendering and input

Platform-specific code should remain behind the window abstraction.

---

# 6. Logging

The Logging system provides engine-wide diagnostic output.

It should support:

- informational messages
- warnings
- errors
- debugging output
- subsystem identification

Logging should remain available across engine development environments.

---

# 7. Event and Input System

The Core event system provides engine-level event communication.

Examples include:

- keyboard events
- mouse events
- window events
- resize events
- application events

Input polling may also be exposed for higher-level systems.

Gameplay input interpretation belongs to Gameplay Systems rather than the low-level event layer.

---

# 8. Rendering

The rendering system owns 3D rendering infrastructure.

The current high-level direction uses:

- Ogre / Ogre-Next
- forward-clustered rendering

The renderer should expose structured engine interfaces rather than allowing higher systems to depend directly on backend-specific implementation details.

This allows the rendering implementation to remain isolated from:

- gameplay
- editor logic
- application code

---

# 9. Entity Component System

The ECS provides the primary entity/component organization for engine and game-world data.

The system should align with plagc's data-oriented design goals.

Expected properties include:

- component-based entities
- efficient iteration
- cache-conscious data access
- clear system ownership
- compatibility with multithreaded processing

EnTT is currently used as the ECS technology.

---

# 10. Physics

The Physics system provides 3D world simulation.

The current technology direction uses Jolt Physics.

Physics should remain exposed through plagc-owned interfaces rather than leaking backend-specific details throughout the engine.

Possible responsibilities include:

- rigid bodies
- collision
- simulation
- queries
- transform synchronization
- world lifecycle

---

# 11. Asset Manager

The Asset Manager controls how resources enter and exist inside the engine.

Responsibilities may include:

- importing
- loading
- caching
- lookup
- ownership
- lifetime
- unloading
- runtime resource access

External asset formats should be transformed into engine-owned representations where appropriate.

Assimp is currently part of the asset-importing technology direction.

---

# 12. Animation

Animation infrastructure belongs to the engine foundation, while gameplay interpretation of animation belongs at a higher level.

Planned capabilities include:

- animation playback
- animation states
- state transitions
- blending
- gameplay-controlled animation
- animation state machines

Gameplay Systems should decide *why* an animation state changes.

The animation system should decide *how* that animation state is evaluated and played.

---

# 13. Gameplay Systems

Gameplay Systems form the game-facing layer above the engine Core.

They should contain behavior that describes game execution rather than general engine infrastructure.

Possible responsibilities include:

- player behavior
- NPC behavior
- gameplay state
- game rules
- gameplay input mapping
- character state
- gameplay animation control
- weapons
- interaction systems
- game-specific entity behavior

Gameplay Systems use Core functionality.

They should not become part of the Core because they describe how a game behaves rather than how the engine operates.

---

# 14. Game Runtime

Game Runtime is the execution environment for a plagc game.

Its responsibilities include:

- starting the game
- maintaining game execution
- running Gameplay Systems
- coordinating runtime state
- interacting with required engine services
- producing the standalone playable application

The runtime should remain capable of operating without the editor.

A distributed game should not require editor components.

---

# 15. Editor Backend

The Editor Backend owns editor-specific logic and engine orchestration.

It acts as the bridge between editor tools and engine functionality.

Potential responsibilities include:

- scene editing
- entity manipulation
- inspector operations
- editor commands
- serialization
- project operations
- runtime management
- gameplay inspection
- engine state inspection
- undo/redo infrastructure
- editor-specific state

The backend may use:

- Core APIs
- Gameplay Systems
- Game Runtime management interfaces

The backend should not contain UI rendering code.

---

# 16. Editor UI / Frontend

The Editor UI is responsible for presenting editor functionality to the user.

Dear ImGui is currently planned as the editor UI technology.

Possible UI areas include:

- dockspace
- hierarchy
- inspector
- console
- asset browser
- scene controls
- runtime controls
- statistics
- project menus

The frontend communicates with the Editor Backend.

It should not directly reach into:

- renderer internals
- physics internals
- ECS internals
- runtime implementation
- asset implementation

unless exposed through an appropriate backend interface.

---

# 17. Sandbox / Test Environment

The Sandbox is a dedicated engine client used for development and validation.

It directly uses Core APIs.

Its role is to test engine functionality without requiring:

- the editor
- a complete game
- production gameplay code

Engine systems should be validated through Sandbox tests as they are developed.

Examples include:

- renderer tests
- ECS stress tests
- physics tests
- asset tests
- animation tests
- threading tests
- performance tests

---

# 18. Scene and World Data

The editor is planned to support visual editing of scenes and worlds.

Scene/world data is intended to be serializable.

The current high-level direction uses JSON-based serialization.

Serialization should represent engine/game state rather than UI state wherever possible.

The editor should be able to load, manipulate, and save this data through Editor Backend services.

---

# 19. Threading Model

plagc is intended to use multithreading as a foundational engine capability.

The high-level threading direction includes:

- worker thread pools
- job queues
- work stealing
- subsystem-level parallelism
- minimizing unnecessary blocking
- explicit synchronization

The job system should eventually allow systems such as:

- rendering preparation
- physics
- animation
- asset processing
- AI
- gameplay
- background loading

to execute independent work concurrently where dependencies permit.

---

# 20. Memory Management

Performance-sensitive systems may use specialized memory allocation strategies.

Potential allocator types include:

- arena allocators
- pool allocators
- frame allocators
- subsystem-specific allocators

General principles include:

- minimize unnecessary heap allocations
- make object lifetime explicit
- group objects with similar lifetime
- preserve data locality
- avoid hidden allocation in hot paths

Memory architecture should be designed alongside subsystem architecture rather than added afterward.

---

# 21. Data-Oriented Design

plagc should organize frequently processed data according to how systems consume it.

The purpose is to improve:

- cache utilization
- sequential access
- iteration performance
- batching
- vectorization potential
- parallel processing

Object-oriented design may still be used where appropriate.

Data-oriented design is primarily intended for performance-sensitive engine data and processing paths.

---

# 22. Build Architecture

Premake is the current project-generation system.

Its role is to support a common project definition while generating development projects for supported environments.

Current primary development compiler:

- MSVC

The architecture should not assume MSVC-specific behavior outside platform/compiler-specific boundaries.

---

# 23. Third-Party Dependencies

Current technology direction includes:

| Library | Responsibility |
|---|---|
| GLFW | Windowing / platform input |
| spdlog | Logging |
| EnTT | ECS |
| Ogre / Ogre-Next | Rendering |
| GLM | Mathematics |
| Jolt Physics | Physics |
| Assimp | Asset importing |
| Dear ImGui | Editor UI |

Third-party libraries should generally remain behind plagc-owned interfaces where practical.

This reduces dependency leakage and allows implementation choices to evolve later.

---

# 24. Architectural Rules

The following rules define the intended dependency discipline of plagc.

1. Core must not depend on the Editor.
2. Core must not depend on Gameplay Systems.
3. Core must not depend on Sandbox.
4. Gameplay Systems may depend on Core.
5. Editor Backend may depend on Core.
6. Editor Backend may use Gameplay Systems.
7. Sandbox may depend on Core.
8. Editor UI depends on Editor Backend.
9. Editor UI should not bypass the backend.
10. Game Runtime uses Gameplay Systems.
11. Editor Backend may manage Game Runtime.
12. Distributed Game Runtime must not require Editor UI.
13. Third-party implementations should be isolated behind plagc-owned boundaries where practical.
14. Platform-specific code should remain isolated.
15. Ownership and lifetime should remain explicit.
16. Performance-sensitive systems should minimize unnecessary allocations and synchronization.

---

# 25. Development Rule

A feature should belong to the lowest layer that actually owns its responsibility.

A useful distinction is:

```text
Core
    provides engine capability

Gameplay Systems
    decide game behavior

Game Runtime
    executes the game

Editor Backend
    controls and edits the project

Editor UI
    presents editor controls

Sandbox
    validates engine capability
```

This separation should be preserved as plagc grows.

---

# 26. Architecture Evolution

This document defines a high-level baseline rather than a frozen implementation.

Subsystem internals may evolve significantly.

Future design documents may separately define:

- Job System
- Renderer
- ECS
- Memory System
- Asset Pipeline
- Physics Integration
- Animation System
- Scene System
- Serialization
- Gameplay Framework
- Game Runtime
- Editor Backend
- Editor UI
- Platform Layer

Changes that alter the dependency direction or responsibility boundaries defined here should be treated as architectural changes rather than ordinary implementation changes.

---

# License

plagc is licensed under the Apache License, Version 2.0.

Copyright © 2026–present  
**Muhammad Uzair Kayani / plagc**

See:

- [LICENSE](LICENSE)
- [NOTICE](NOTICE)

For current project updates:

https://github.com/Muhammad-Uzair-Kayani/plagc