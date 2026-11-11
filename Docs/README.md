# plagc

**plagc** is a cross-platform, fast, and lightweight 3D game engine written in modern C++.

The project is currently under active development.

> **Status:** Early development

## Overview

plagc aims to provide a focused environment for developing 3D games without carrying unnecessary engine complexity.

The project prioritizes:

- Cross-platform development
- Performance
- Lightweight design
- Modern C++
- Clear and maintainable engine development

Initial development is focused on **Windows** and **Linux**. Additional platforms may be considered as the project matures.

## Technology

Current technologies used or planned for plagc include:

| Technology | Purpose |
|---|---|
| C++ | Engine development |
| Premake | Project generation |
| GLFW | Windowing and input |
| spdlog | Logging |
| EnTT | Entity Component System |
| Ogre / Ogre-Next | Rendering |
| GLM | Mathematics |
| Jolt Physics | Physics |
| Assimp | Asset importing |
| Dear ImGui | Editor interface |

The technology stack may evolve during development. Not every listed dependency is integrated into the current build.

## Getting Started

### Clone the repository

Clone with its Git submodules:

```bash
git clone --recurse-submodules https://github.com/Muhammad-Uzair-Kayani/plagc.git
cd plagc
```

If you already cloned without submodules, initialize them from the repository root:

```bash
git submodule update --init --recursive
```

The current build requires the GLFW and spdlog submodules under `third-party/`.

### Windows — Visual Studio 2022 (MSVC)

**Prerequisites:** Visual Studio 2022 with the **Desktop development with C++** workload, Windows SDK, Git, and the repository's bundled Windows Premake executable (`third-party/premake/premake5.exe`).

**Option A: Generate with the included batch script.** From the repository root, run in Command Prompt:

```bat
Scripts\vs22\GenerateProjects.bat
```

**Option B: Generate manually with Premake.** From the repository root:

```bat
third-party\premake\premake5.exe vs2022
```

Both methods generate the Visual Studio 2022 solution and project files. Open `plagc.sln` in Visual Studio 2022, select `Debug`, `Release`, or `Dist` and the `x64` platform, then build the `Sandbox` startup project (or build the solution).

Alternatively, from a **Developer Command Prompt for VS 2022** after generating the solution:

```bat
msbuild plagc.sln /m /p:Configuration=Debug /p:Platform=x64
```

To run the Debug executable from the repository root:

```bat
bin\Debug\Sandbox\Sandbox.exe
```

Replace `Debug` with `Release` or `Dist` when building or running another configuration. On Windows, `Dist` uses the GUI subsystem; `Debug` and `Release` use the console subsystem.

### Linux — GNU Make (`gmake2`) / VS Code

**Prerequisites:** Git, a modern C++ compiler (such as GCC or Clang), GNU Make, development tools, Linux X11 development libraries needed by GLFW, and a **Linux Premake 5 executable**. The checked-in `third-party/premake/premake5.exe` is a Windows executable and cannot run natively on Linux.

On Ubuntu/Debian, a starting point for required packages is:

```bash
sudo apt update
sudo apt install build-essential git pkg-config libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

Install Premake 5 separately and make `premake5` available on your `PATH`, or place a Linux Premake executable at `third-party/premake/premake5`. If your Premake executable is elsewhere, set `PREMAKE` when invoking the script.

**Option A: Generate with the Linux script.** From the repository root:

```bash
bash Scripts/gmake2/GenerateProjects.sh
```

**Option B: Generate manually with Premake.** From the repository root:

```bash
premake5 gmake2
```

If you placed the Linux executable in the repository instead, run:

```bash
./third-party/premake/premake5 gmake2
```

**Build** the generated GNU Make projects from the repository root:

```bash
make config=debug -j"$(nproc)"
```

Other configurations:

```bash
make config=release -j"$(nproc)"
make config=dist -j"$(nproc)"
```

**Run** the Debug build from the repository root:

```bash
./bin/Debug/Sandbox/Sandbox
```

Replace `Debug` with `Release` or `Dist` to run another configuration.

For **VS Code**, open the `plagc` repository folder and use its integrated terminal to generate and build with the same `gmake2` and `make` commands. VS Code is the editor; the actual build is performed by your C++ toolchain and GNU Make. The C/C++ extension is recommended for source navigation and debugging.

> **Linux status:** Premake includes Linux/X11 build rules, but Linux compilation and runtime compatibility have not been independently verified by this README update. Additional dependency or compiler adjustments may be required during early development. The current GLFW recipe targets X11, not a native Wayland backend.

### Build configurations and output

| Configuration | Purpose |
|---|---|
| `Debug` | Debug symbols; compiler optimizations disabled |
| `Release` | Optimized development build |
| `Dist` | Optimized distribution build |

Premake builds the **plagc** engine as a static library and **Sandbox** as an executable. Generated binaries are placed under `bin/<Configuration>/<Project>/`; intermediate artifacts are stored under `bin-int/`.

The included scripts **generate project files only**. They do not compile or launch the engine. Run the corresponding Visual Studio/MSBuild or GNU Make command afterward.

## Documentation

Detailed technical information is maintained separately from this README.

### Architecture

For engine structure, system responsibilities, dependency rules, runtime organization, editor design, and architectural decisions:

[Architecture.md](Architecture/Architecture.md)

### Licensing files

Full licensing terms: [LICENSE](Licenses/LICENSE)

Additional copyright and attribution information: [NOTICE](Licenses/NOTICE)

## Project Status

plagc is currently in its foundation and early development stage.

APIs, tooling, features, and internal implementation are expected to change while the engine matures. The repository should not yet be considered production-ready.

## Contributions

plagc is currently an **owner-maintained project**. External code contributions are not being accepted at this stage.

This policy may change in the future as the project and contribution workflow mature.

## License

plagc is licensed under the **Apache License, Version 2.0**.

Copyright © 2026–present  
**Muhammad Uzair Kayani / plagc**

See [LICENSE](Licenses/LICENSE) for the complete license terms.

Third-party libraries and dependencies remain subject to their respective licenses.

## Repository

For source code and the latest project updates:

https://github.com/Muhammad-Uzair-Kayani/plagc
