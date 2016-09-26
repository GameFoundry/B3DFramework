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
 2. Set up [third party dependencies](#dependencies)
 3. Generate a build file (e.g. Visual Studio solution or a Makefile) using CMake
  - You can customize your build by choosing options like render API (DirectX, OpenGL), audio module and whether to build the entire editor or just the engine.
 4. Compile using your favorite tool

# <a name="dependencies"></a>Third party dependencies
Banshee relies on a variety of third party dependencies. We provide a set of pre-compiled dependencies needed for the latest version of Banshee using Visual Studio 2015, to save you the hassle of compiling them yourself. If you are not using VS2015 or want to compile the dependencies yourself for some other reason, a guide is provided below.

 * [Download dependencies (VS2015)] (http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_0.4.0.zip)
 * [Compiling dependencies manually](dependencies.md)
 
For older versions of pre-compiled dependencies check the git release tag descriptions for links.

## Other dependencies
The following dependencies will need to be installed manually regardless if you used pre-compiled dependency package or have compiled them yourself.

**DirectX SDK**
 - Only needed if on Windows 7 or earlier and using DirectX 11 render API
 - Or for all Windows versions if using DirectX 9 render API
 - Set up DXSDK_DIR environment variable pointing to the DirectX instalation
 
**Windows SDK**
 - Only needed if on Windows 8 or later and using DirectX 11 render API
 
**DirectX Debug Layer**
 - Only needed if on Windows 10 and using DirectX 11 render API
 - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"
 
**Python 3.5**
 - Only needed if you plan on running Python scripts in the /Scripts folder
 - https://www.python.org/downloads/

**FMOD Low Level Programmer API**
 - Only needed if you selected the FMOD audio module during build configuration (not selected by default)
 - http://www.fmod.org/download/
 - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
 - Copy the dynamic libraries from {INSTALLDIR}/api/lowlevel/lib into /bin folder in Banshee source code folder
  - Use logging libraries for the Debug builds, and non-logging for OptimizedDebug and Release builds
  
**Mono 4.2**
 - If you wish to compile managed assemblies using a Microsoft compiler (e.g. using Visual Studio) yet still be able to debug the generated assemblies, you must install Mono 4.2. and set up an environment variable MONO_INSTALL_DIR pointing to the Mono installation directory. When this is set up "pdb2mdb" script will trigger on next compile generating the needed debug symbols.
 - http://www.mono-project.com/download/