# Compiling

Banshee is available on two branches:
 - **preview** - This branch always contains latest "stable" pre-release version. This might be a few months behind master.
 - **master** - This branch contains the latest changes, might be quite unstable and might not even compile. Use this if you absolutely need the bleeding edge changes.
 
Banshee can be compiled in two modes:
 - **Runtime** (default) - This will compile the entire engine, scripting system and the editor. This is intended for those that plan on using the editor as a runtime to develop and build games.
 - **Framework** - This will compile only the C++ engine framework without the editor or the scripting API. This is intended for those that prefer to create applications more directly with less high level abstractions.
 
Supported platforms:
 - Windows
 - (Mac & Linux coming soon)

Supported compilers:
 - MSVC++ 14.0 (Visual Studio 2015)
 - (Clang/GCC coming soon)
 
## Compile steps (Simple)

We provide two sets of compile intructions depending on what you need:
 - Simple: Straightforward setup for those that don't need to perform any special configuration on Banshee's build and just want to compile with default settings
 - Advanced: Slightly more complicated setup, but allows you to configure exactly how to build Banshee and with which tools

### Windows

To create a Visual Studio 2015 solution with the complete runtime, follow these steps:
 1. Download source code
 2. Run setup.bat in the source code root folder
 3. Access Visual Studio solution in /Build/VS2015/Banshee.sln
 4. If compilation fails with errors related to DirectX, you need to install relevant DirectX dependencies as described [here](#otherDeps) 

## Compile steps (Advanced)

### Windows

For those that wish to customize their build, or compile using something other than Visual Studio, follow the steps below:
 1. Download source code
 2. Set up [third party dependencies](#dependencies)
 3. Generate build files using CMake:
  1. Install CMake 3.6.1 or higher for Windows
  2. Run CMake GUI
  3. Point CMake to the /Source sub-folder in Banshee's source code (Browse Source... button in GUI)
  4. Choose an output folder (Browse Build... button in GUI)
  5. Hit Configure and choose the toolset to generate the files for (e.g. Visual Studio 2015 64-bit)
  6. (Optionally) Customize your build by setting any of the properties that have appeared. You can choose whether to build the complete runtime or just the framework (BUILD_EDITOR property), as well as choose render API (Vulkan, DirectX, OpenGL), audio module (FMOD, OpenAudio) among other options.
  7. Hit the Generate button
 4. Once CMake is done generating you can use the toolset you chose (e.g. Visual Studio) to open the generated files (e.g. open the .sln file provided in the Build folder in the case of Visual Studio)

### <a name="dependencies"></a>Third party dependencies
Banshee relies on a variety of third party dependencies. We provide a set of pre-compiled dependencies needed for the latest version of Banshee using Visual Studio 2015, to save you the hassle of compiling them yourself. If you are not using VS2015 or want to compile the dependencies yourself for some other reason, a guide is provided below.

 * [Download dependencies (VS2015)] (http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_Master.zip)
 * [Compiling dependencies manually](dependencies.md)
 
For older versions of pre-compiled dependencies check the git release tag descriptions for links.

## <a name="otherDeps"></a>Other dependencies
The following dependencies will need to be installed manually regardless if you used pre-compiled dependency package or have compiled them yourself.

**DirectX SDK** (Optional if not using DirectX)
 - Only needed if on Windows 7 or earlier and using DirectX 11 render API
 - Set up DXSDK_DIR environment variable pointing to the DirectX instalation
 
**Windows SDK** (Optional if not using DirectX)
 - Only needed if on Windows 8 or later and using DirectX 11 render API
 
**DirectX Debug Layer** (Optional if not using DirectX)
 - Only needed if on Windows 10 and using DirectX 11 render API
 - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"
 
**Vulkan SDK** (Optional if not using Vulkan) 
 - Only needed if you selected the Vulkan render API during build configuration (selected by default)
 - https://lunarg.com/vulkan-sdk/
 - If CMake complains it cannot find Vulkan, manually set the Vulkan_INSTALL_DIRS to your installation directory
 
**Python 3.5** (Optional)
 - Only needed if you plan on running Python scripts in the /Scripts folder
 - https://www.python.org/downloads/

**FMOD Low Level Programmer API** (Optional)
 - Only needed if you selected the FMOD audio module during build configuration (not selected by default)
 - http://www.fmod.org/download/
 - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
 - Copy the dynamic libraries from {INSTALLDIR}/api/lowlevel/lib into /bin folder in Banshee source code folder
  - Use logging libraries for the Debug builds, and non-logging for OptimizedDebug and Release builds
  
**Mono 4.2** (Optional)
 - If you wish to compile managed assemblies using a Microsoft compiler (e.g. using Visual Studio) yet still be able to debug the generated assemblies, you must install Mono 4.2. and set up an environment variable MONO_INSTALL_DIR pointing to the Mono installation directory. When this is set up "pdb2mdb" script will trigger on next compile generating the needed debug symbols.
 - http://www.mono-project.com/download/