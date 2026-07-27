# Vulkan Setup

SHIBA renders with Vulkan. Building requires the **LunarG Vulkan SDK**
(≥ **1.4.350**). CMake finds it via `find_package(Vulkan)`, which reads the
`VULKAN_SDK` env var set by the SDK.

VMA is vendored in `thirdparty/`, no install needed.

Builds are driven by `CMakePresets.json`, so you never pass `-S`/`-B` by hand.
List everything available with `cmake --list-presets`.

## Windows

Run the installer from <https://vulkan.lunarg.com/sdk/home#windows> (it sets
`VULKAN_SDK` automatically), then open a **new** terminal, and pick a preset
(`msvc` or `clang-cl`):

```sh
cmake --preset msvc
cmake --build --preset msvc-debug     # or msvc-dev for RelWithDebInfo
```

## Linux

Download the tarball from <https://vulkan.lunarg.com/sdk/home#linux>, extract it,
and source the setup script (add it to your `~/.bashrc`):

```sh
source ~/vulkan/<version>/setup-env.sh
cmake --preset gcc                    # or clang
cmake --build --preset gcc-debug      # or gcc-dev for RelWithDebInfo
```

Distro packages (`libvulkan-dev` etc.) work too, but may lag the 1.4.350 floor;
use the tarball if the version check fails.

## First configure / after installing the SDK

`find_package` caches its result in `CMakeCache.txt`. If CMake ever configured
*before* the SDK was installed, it cached "not found" and won't re-detect on a
plain reload. Make sure `VULKAN_SDK` is set in the shell/IDE launching CMake
(open a **fresh** one after installing), then wipe the preset's build tree and
reconfigure:

```sh
rm -rf build/<preset>                 # e.g. build/msvc
cmake --preset <preset>
```

The configured summary prints the detected device name when it worked.

## Troubleshooting

- **`Could NOT find Vulkan`**: SDK missing, or `VULKAN_SDK` unset in this shell.
  New terminal (Windows) / re-source `setup-env.sh` (Linux). If the var *is* set,
  wipe `build/<preset>` and reconfigure; a stale "not found" may be cached.
- **IDE still can't find it**: the IDE inherited the old environment. Restart it
  after installing the SDK so it picks up `VULKAN_SDK`.
- **Version too old**: update the SDK or switch from distro packages to the tarball.
- **`vulkaninfo` shows no devices**: GPU driver issue, not the SDK. Update drivers.

## Licenses

| Component                                   | Where                        | License    |
|---------------------------------------------|------------------------------|------------|
| Vulkan headers / loader / validation layers | Installed via SDK            | Apache-2.0 |
| VMA (Vulkan Memory Allocator)               | Vendored in `thirdparty/vma` | MIT        |

VMA is redistributed in this repo, so its `LICENSE` file is kept alongside the
vendored copy. The Vulkan SDK is installed per-machine (not redistributed), so
its licenses apply to LunarG's distribution, not to SHIBA. See
<https://vulkan.lunarg.com/license/> for the full per-component breakdown of the
bundled tools.