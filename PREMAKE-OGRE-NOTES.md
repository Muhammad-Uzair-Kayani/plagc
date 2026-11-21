# Premake Ogre Vulkan: missing SPIRV-Reflect symbols

Only replacement: `Scripts/premake/OGRE.lua` (extract at repository root).

## Diagnosed error

`OgreVulkanProgram.cpp` calls `spvReflectCreateShaderModule`, `spvReflectDestroyShaderModule`,
`spvReflectEnumerateDescriptorSets`, and `spvReflectEnumerateInputVariables`. They are
implemented in the C source `spirv_reflect.c`, which was not selected by the previous
Premake recipe because the Vulkan source list was restricted to `**.cpp` files.

The patch adds the **vendor's own** `spirv_reflect.c` into the existing
`RenderSystem_Vulkan` static library and supplies its header directories.
It finds the source under `third-party/ogre-next/RenderSystems/Vulkan/` and
fails **during Premake generation** if the source isn't available (instead
of allowing a long build to fail during linking).

## Apply without rebuilding all of Ogre

1. Extract to your plagc root. Replace only `Scripts/premake/OGRE.lua`.
2. Run `premake5 vs2022` and reload the VS solution when prompted.
3. With Release | x64 selected, **Build** (not Clean/Rebuild) the `RenderSystem_Vulkan`
   project, then **Build** the `Sandbox` project. Existing OgreMain/HLMS
   objects and libraries should remain intact as their compiler settings
   and source files have not changed.
4. If Premake cannot find `spirv_reflect.c`, check that the corresponding
   Ogre-Next vendor source exists; do not substitute a random SDK version.

Do not delete `bin-int`, and do not clean the solution. Debug and Dist link
through the same Vulkan source project when built in those configurations.

## Validation limits

The patch passed Lua grammar and diff checks, but the Ogre source tree and
MSVC toolchain on your machine are not available here. Full compilation,
linking and runtime shader behavior are therefore not verified. This
fix addresses link symbols only, not the earlier glslang runtime crash.
