# Compiling

Banshee is available on two branches:
 - **preview** - This branch always contains latest "stable" pre-release version.
 - **master** - This branch contains the latest changes, might be quite unstable and might not even compile. Use this if you absolutely need the bleeding edge changes.
 
It can be compiled using:
 - MSVC++ 14.0 (Visual Studio 2015)
 - Clang
 
Banshee currently only compiles on Windows, but Mac & Linux ports are coming soon.

To compile follow these steps:
 1. Download source code
 2. Download [data files](#data) and extract them into the source directory
 3. Set up [third party dependencies](#dependencies)
 4. Generate a build file (e.g. Visual Studio solution or a Makefile) using CMake
  - You can customize your build by choosing options like render API (DirectX, OpenGL), audio module and whether to build the entire editor or just the engine.
 5. Compile using your favorite tool

# <a name="data"></a>Data files
Data files are non-code resources like images, shaders, etc. needed to run the editor, engine and example projects.

[Download data files (Latest)] (http://bearishsun.thalassa.feralhosting.com/BansheeData_v0.3.0.zip)

For older versions check the git release tag descriptions for links.
 
# <a name="dependencies"></a>Third party dependencies
Banshee relies on a variety of third party dependencies. We provide a set of pre-compiled dependencies needed for Visual Studio 2015, to save you the hassle of compiling them yourself. If you are not using VS2015 or want to compile the dependencies yourself, a guide is provided as well.

## Pre-compiled dependencies
If going with the pre-compiled dependencies route, download the file below and extract it to the source directory.

[Download dependencies (VS2015)] (http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_v0.3.0.zip)

For older versions check the git release tag descriptions for links.

You will also need to install the following dependencies manually:

**DirectX SDK**
 - Only for Windows 7 or earlier if using DirectX 11 render API
 - Or for all Windows versions if using DirectX 9 render API
 - Set up DXSDK_DIR environment variable pointing to the DirectX instalation
 
**Windows SDK**
 - Only for Windows 8 or later if using DirectX 11 render API
 
**DirectX Debug Layer**
 - Only for Windows 10 if using DirectX 11 render API
 - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"
 
**FMOD Low Level Programmer API**
 - Only if using the FMOD audio module (not selected by default)
 - http://www.fmod.org/download/
 - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
 - Copy the dynamic libraries from {INSTALLDIR}/api/lowlevel/lib into /bin folder in Banshee source code folder
  - Use logging libraries for the Debug builds, and non-logging for OptimizedDebug and Release builds
 
## Compiling dependencies manually

