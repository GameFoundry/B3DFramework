# Compiling dependencies manually

Below you will find a list of dependencies that B3D Framework relies on, as well as links to their source code and/or binaries. If a dependency isn't provided in binary form you will have to manually compile it (this is the case for the large majority or them). Make sure to compile the exact version of the dependency listed below. Newer versions *might* work, but haven't been tested. 

Once you have the dependency development files (headers and binaries) you will need to make sure they placed in the appropriate folders so B3D Framework can find them during the build process. 

The dependencies are searched for in these locations:
- The `/Dependency` folder within framework source. See below for the exact breakdown of how this folder is supposed to look. Usually you want to put all your dependencies here.
- If dependency cannot be found in the `/Dependency` folder, its default install path is searched for instead. For example `usr/local` on Linux/macOS or default install path if the dependency comes with an installer. 

Note that on Windows most dependencies do not have default install paths and should therefore be placed in the `/Dependency` folder. In order to avoid problems with dependency versions this should be the preferred behaviour on Linux/macOS as well. 

`/Dependency` folder breakdown:
- Static & shared libraries (.lib, .a, .so): 
  - Pick one of:
    - (bsfSource)/Dependencies/(DepName)/lib
	- (bsfSource)/Dependencies/(DepName)/lib/(Platform)
	- (bsfSource)/Dependencies/(DepName)/lib/(Platform)/(Configuration)
	- (bsfSource)/Dependencies/(DepName)/lib/(Configuration)
- Dynamic libraries (.dll, .dylib)
  - Place in (bsfSource)/bin/(Platform)/(Configuration)
- Includes
  - Place in (bsfSource)/Dependencies/(DepName)/include
- Tools (executables)
  - Place in (bsfSource)/Dependencies/tools/(DepName)  
  
Legend:
- (bsfSource) - root directory of B3D Framework
- (DepName) - name of the dependency (title of each dependency shown below)
- (Platform) - x86 for 32-bit builds, x64 for 64-bit builds
- (Configuration) - Debug, RelWithDebInfo, MinSizeRel, or Release  
  
Each library is accompanied by a Find***.cmake CMake module that is responsible for finding the library. These modules are located under `Source/CMake/Modules`. They follow the rules described above, but if you are unsure where library outputs should be placed you can look at the source code for those modules to find their search paths.
   
Additionally, if the dependency structure still isn't clear, download one of the pre-compiled dependency packages to see an example.  

## Building dependencies from source

By default, pre-compiled binaries for all dependencies are bundled with the framework, so you do not need to build anything yourself to use B3D.

If you do want to rebuild a dependency from source (for example to tweak compile flags, try a new version, or build on an unsupported platform), each dependency listed below with a `Build script:` entry has a matching shell script under `Framework/Scripts/`. Run it from that directory:

```sh
cd Framework/Scripts
./B3DBuild<DepName>.sh
```

The script clones or updates the upstream source, configures CMake, builds, and installs the result into `Framework/Dependencies/<DepName>/` in the layout the framework expects.

### Overriding the CMake generator

The default generator is picked by `B3DBuildCommon.sh` (`Visual Studio 17 2022` on Windows, `Ninja Multi-Config` on macOS/Linux). To use a different generator, set the `B3D_CMAKE_GENERATOR` environment variable before running the script:

```sh
B3D_CMAKE_GENERATOR="Ninja" ./B3DBuildSnappy.sh
```
## Library list

**snappy**
- Snappy 1.2.2
- https://github.com/google/snappy
- Required by bsfUtility
- Compile as a static library
- Build script: `B3DBuildSnappy.sh`

**LibICU**
- http://site.icu-project.org/
- Only required for Linux builds
- Required by bsfCore
- Compile as a static library
 - Make sure to provide `-fPIC` as an explicit flag
 - You *can* use system version of the ICU library, but your binaries will then only be compatible with Linux distros using the exact same API version (which changes often). It's safest to link ICU statically.

**FBXSDK**
- FBX SDK 2016.1
- http://usa.autodesk.com/fbx
- Required by bsfFBXImporter
- No compilation required, libraries are provided pre-compiled

**freetype**
- Freetype 2.14.3
- https://gitlab.freedesktop.org/freetype/freetype
- Required by bsfFontImporter
- Compile as a static library
- Build script: `B3DBuildFreetype.sh`

**freeimg**
- FreeImage 3.18
- http://freeimage.sourceforge.net
- Required by bsfFreeImgImporter
- Compile as a dynamic library on Windows, static library on Linux/macOS
- Build script: `B3DBuildFreeImage.sh`

**PhysX**
- PhysX 3.4
- https://github.com/NVIDIAGameWorks/PhysX (branch *3.4*)
- Required by bsfPhysX
- Compile as a dynamic library on Windows, static library on Linux/macOS (default)
- Build script: `B3DBuildPhysX.sh`

**OpenAL**
- OpenAL Soft 1.17.2
- https://github.com/kcat/openal-soft
- Required by bsfOpenAudio
- **Linux only**
  - Make sure to get audio backend libraries before compiling: PulseAudio, OSS, ALSA and JACK
  - On Debian/Ubuntu run: `apt-get install libpulse libasound2-dev libjack-dev`
- Compile as a dynamic library on Windows/Linux (default), static library on macOS
- Build script: `B3DBuildOpenAL.sh`

**libogg**
- libogg v1.3.5
- https://github.com/xiph/ogg
- Required by bsfOpenAudio and bsfFMOD
- Compile as a static library
  - Required when compiling libvorbis and libFLAC below.
- Build script: `B3DBuildOgg.sh`

**libvorbis**
- libvorbis v1.3.7
- https://github.com/xiph/vorbis
- Required by bsfOpenAudio and bsfFMOD
- Compile as a static library
  - Requires libogg.
- Build script: `B3DBuildVorbis.sh`

**libFLAC**
- libFLAC 1.5.0
- https://github.com/xiph/flac
- Required by bsfOpenAudio
- Compile as a static library
  - Requires libogg.
- Build script: `B3DBuildFLAC.sh`

**glslang**
- glslang version: 15.4.0 (matches Vulkan SDK 1.4.321)
- SPIRV-Tools / SPIRV-Headers: pinned transitively via glslang's `known_good.json` (fetched by `update_glslang_sources.py`)
- https://github.com/KhronosGroup/glslang
- Required by bsfVulkanGpuBackend
- Compile as a static library
- Build script: `B3DBuildGlslang.sh`

**SPIRV-Cross**
- SPIRV-Cross version: vulkan-sdk-1.4.321.0 (matches Vulkan SDK 1.4.321, same as glslang above)
- https://github.com/KhronosGroup/SPIRV-Cross
- Required by bsfShaderBackendVKSL and bsfMetalGpuBackend
- Compile as a static library
- Build script: `B3DBuildSPIRVCross.sh`

**MoltenVK** (macOS only)
- Commit ID: a684b47baab834e12da2af9f5997c867c4265b46
- https://github.com/KhronosGroup/MoltenVK
- Required by bsfVulkanGpuBackend
- Compile and install, then copy contents of `macOS/static/` folder into `lib` sub-folder

**XShaderCompiler**
- https://github.com/BearishSun/XShaderCompiler (branch *banshee*)
- Required by bsfSL
- Compile as a static library
- Build script: `B3DBuildShaderCompiler.sh`

**Slang**
- Slang v2025.21.0
- https://github.com/shader-slang/slang
- Required by bsfSL
- Compile as a static library
- Build script: `B3DBuildSlang.sh`

**mono** (via .NET runtime)
- .NET 9.0.11
- Only required if B3D_SCRIPT_API=C# option is specified during the build (i.e. C# scripting is enabled)
- https://github.com/dotnet/runtime
- Required by bsfMono
- Compile as a dynamic library
- Build script: `B3DBuildMono.sh`

**METIS**
- METIS v5.2.1
- https://github.com/KarypisLab/METIS
- Required by bsfCore
- Compile as a static library
- Build script: `B3DBuildMetis.sh`

**protobuf**
- Protocol Buffers v21.12
- https://github.com/protocolbuffers/protobuf
- Required by bsfCore (BansheeForge integration)
- Compile as a static library
- Build script: `B3DBuildProtobuf.sh`

**GameNetworkingSockets**
- GameNetworkingSockets v1.4.1
- https://github.com/ValveSoftware/GameNetworkingSockets
- Required by bsfCore (networking)
- Compile as a static library
- Build script: `B3DBuildGameNetworkingSockets.sh`

**LLVM**
- LLVM llvmorg-20.1.7
- https://github.com/llvm/llvm-project
- Only required for B3DCodeGen (the Clang-based code generator)
- Build script: `B3DBuildLLVM.sh`

**bison**
- Bison 3.0.4
- Only required if B3D_BUILD_BSL_PARSER option is specified during the build (off by default)
- **Windows**
  - http://sourceforge.net/projects/winflexbison/files/
- **Linux**
  - Debian/Ubuntu: `apt-get install bison`
  - Or equivalent package for your distribution
- **macOS**
  - `brew install bison`
  - Make sure old version of Bison that comes with Xcode is overriden:
    - Add this to $HOME/.bash_profile: `export PATH="/usr/local/opt/bison/bin:$PATH"`
    - `mv /usr/bin/bison /usr/bin/bison-2.3`
- Required by bsfSL
- Executable (tool)
 
**flex**
- Flex 2.6.1
- Only required if B3D_BUILD_BSL_PARSER option is specified during the build (off by default)
- **Windows**
  - http://sourceforge.net/projects/winflexbison/files/
- **Linux**
  - Debian/Ubuntu: `apt-get install flex`
  - Or equivalent package for your distribution
- **macOS**
  - `brew install flex`
- Required by bsfSL
- Executable (tool)
