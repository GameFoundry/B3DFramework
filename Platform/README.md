# Framework Platform Overlays

Platform-specific code, one sub-folder per platform. Each platform is an **overlay** of
`Framework/Source/` — its layout mirrors the framework's, and CMake merges it into the
build at configure time, so `#include` paths and targets are unchanged. This lets a
proprietary platform ship as an independent git submodule.

```
Framework/Platform/
├── Win32/                       # Built-in host platform
│   ├── Platform.cmake           # Discovery metadata (declares GPU backends)
│   └── Source/
│       ├── CMakeLists.txt       # Build entry (OS libs, overlay plugins)
│       └── Engine/{Core,Utility}/Private/Win32/...
├── Linux/, MacOS/               # Built-in host platforms (same layout)
├── Unix/                        # Shared by Linux + macOS; sources only, no CMake files
│   └── Source/Engine/Utility/Private/Unix/...
└── <Custom>/                    # Proprietary platform (submodule), e.g. PS5
    ├── Platform.cmake
    ├── Source/{CMakeLists.txt, Engine/.../Private/<Custom>/, Plugins/bsf<Custom>GpuBackend/}
    └── Dependencies/<Package>/  # optional
```

## Platforms

The build target is chosen with the **`B3D_PLATFORM`** cache drop-down (one configuration =
one platform). Its choices are the host plus any present custom platform — e.g. `Win32` on
Windows, or `Win32;PS5` once a PS5 submodule is mounted; it defaults to the host. Selecting a
platform whose folder is absent is a hard error pointing at the submodule init command.

- **Built-in** (`Win32`, `MacOS`, `Linux`): the host is always offered. The shared `Unix`
  overlay is not a stand-alone target (it rides along on Linux/macOS).
- **Custom**: a proprietary platform shipped as a submodule appears in the drop-down once its
  folder is present under `Framework/Platform/`.

## CMake files per platform

- **`Platform.cmake`** — discovery metadata, `include()`d before the GPU options. When this
  platform is the build target it declares its GPU backends via `B3D_GPU_BACKEND_CHOICES` and
  `B3D_GPU_BACKEND_DEFAULT`; each backend's own `CMakeLists.txt` registers its plugin target
  and shading language through `B3DRegisterGpuBackend()`.
- **`Source/CMakeLists.txt`** — build entry, added after `bsf` exists. Links the platform's
  OS libraries and registers overlay plugins. Must self-guard (e.g. `if(NOT WIN32) return()`).

`Unix` needs neither (sources picked up by the glob). Engine sources under
`Source/Engine/{Core,Utility}` merge into `bsf`; `Source/Plugins/<name>` are platform-only
plugins. Optional `Dependencies/<Package>/` mirrors `Framework/Dependencies/<Package>/` and
is fetched with `B3DCheckAndUpdatePrebuiltDependency(<Package> TRUE)`.
