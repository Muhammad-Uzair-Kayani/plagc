# plagc

**plagc** is a cross-platform, fast, and lightweight 3D game engine written in modern C++.

The project is focused on building a practical 3D engine with explicit system boundaries, multithreaded execution, data-oriented design, controlled memory management, and a clean separation between engine, gameplay, runtime, and editor systems.

> **Status:** Early development / architecture phase

Repository:  
https://github.com/Muhammad-Uzair-Kayani/plagc

---

## What plagc aims to be

plagc is designed around three primary goals:

- **Cross-platform**
- **Fast**
- **Lightweight**

The engine is intended to provide the systems required for building 3D games without growing into an unnecessarily large or loosely connected feature set.

Initial development is focused on:

- Windows
- Linux

Additional platforms may be considered later where technically practical.

---

## Main Features

The planned engine foundation includes:

- Cross-platform window management
- Logging
- Event and input handling
- 3D rendering
- Entity Component System
- 3D physics
- Asset management
- Animation
- Gameplay systems
- Game runtime
- Sandbox / test environment
- Visual editor

The exact feature set will evolve as development progresses.

---

## Performance Direction

plagc is being designed with performance as an architectural concern from the beginning.

Major areas include:

- multithreading
- worker thread pools
- job stealing
- data-oriented design
- cache-friendly data organization
- custom allocators
- pool allocators
- arena allocators
- explicit resource ownership
- controlled synchronization

---

## Technology

The current technology direction includes:

| Technology | Purpose |
|---|---|
| C++17 / C++20 | Engine development |
| Premake | Project generation |
| GLFW | Windowing and platform input |
| spdlog | Logging |
| EnTT | Entity Component System |
| Ogre / Ogre-Next | Rendering |
| GLM | Mathematics |
| Jolt Physics | 3D physics |
| Assimp | Asset importing |
| Dear ImGui | Editor UI |

The dependency set may change as plagc develops.

---

## Architecture

plagc uses a layered engine architecture built around a reusable **Core**.

The main architectural areas are:

- Core
- Editor Backend
- Gameplay Systems
- Sandbox / Test Environment
- Editor UI / Frontend
- Game Runtime
