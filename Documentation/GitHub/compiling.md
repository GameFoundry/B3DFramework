# Compiling

## Quick start
Use the guide below to get Banshee up and running as quickly as possible. Scroll further below for advanced build options that allow for more customization.

- Install git (https://git-scm.com) and CMake 3.9.0 or higher (https://cmake.org)
  - Ensure they are added to your *PATH* environment variable
- Install other dependencies
  - See [here](#otherDeps)
- Run the following commands in the terminal/command line:
  - `git clone https://github.com/BearishSun/BansheeEngine.git`
  - `cd BansheeEngine`
  - `mkdir Build`
  - `cd Build`
  - `cmake -G "$generator$" ../Source`
    - Where *$generator$* should be replaced with any of the supported generators. Some common ones:
	  - `Visual Studio 14 2015 Win64` - Visual Studio 2015 (64-bit build)
	  - `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
	  - `Unix Makefiles`
	  - `Ninja`
	  - `Xcode`
	- See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
- Build the project using your chosen tool
  - Build files will be placed in `BansheeEngine\Build` folder
	 
## Customizing the build

By default the process above will fetch the *master* branch. If required switch to a different branch before building:
 - **preview** - This branch always contains latest "stable" pre-release version. This might be a few months behind master.
 - **master** - This branch contains the latest changes, might be quite unstable and might not even compile. Use this if you absolutely need the bleeding edge changes.
 
Variety of *CMake* variables are provided that allow you to customize the build:
 
- By default the process above will build the complete Banshee 3D runtime. You can pick between two modes using the *BUILD_SCOPE* variable in *CMake*:
  - **Runtime** (default) - This will compile the entire engine, scripting system and the editor. This is intended for those that plan on using the editor as their main environment for development.
  - **Framework** - This will compile only the C++ engine framework without the editor or the scripting API. This is intended for those that prefer to develop more directly with less high level abstractions.
- Additional variables allow you to pick between the render API (Vulkan, DirectX, OpenGL), audio module (FMOD, OpenAudio) among other options. Run *CMake* to see all options.
- Note that non-default *CMake* options might require additional dependencies to be installed, see [here](#otherDeps).
 
You can choose to use a different *CMake* generator than those specified above, as long as the platform/compiler is supported:  
  - Supported platforms:
    - Windows, Linux
    - (macOS coming soon)
  - Supported compilers:
    - MSVC++ 14.0 (Visual Studio 2015)
	- MSVC++ 15.0 (Visual Studio 2017)
    - GCC 7.0 (or newer)
    - Clang 4 (or newer)

### <a name="dependencies"></a>Third party dependencies
Banshee relies on a variety of third party dependencies. A set of pre-compiled dependencies are provided for every supported platform/compiler. These will be fetched automatically by the build process, but you may also download them manually or compile them from source:
 * [Download dependencies (VS2015/VS2017)](http://data.banshee3d.com/BansheeDependencies_VS2015_Master.zip)
 * [Compiling dependencies manually](dependencies.md)
 
For older versions of pre-compiled dependencies check the git release tag descriptions for links.

## <a name="otherDeps"></a>Other dependencies
The following dependencies will need to be installed manually. Which ones are required depend on the selected *CMake* options and your usage of Banshee (check text of each entry below).

**Windows**
  - **DirectX SDK** (Required by default on Windows 7 or earlier)
	- Optional if you have choosen a different RenderAPI in *CMake* options
    - Set up DXSDK_DIR environment variable pointing to the DirectX instalation
  - **Windows SDK** (Required by default on Windows 8 or later)
	- Optional if you have choosen a different RenderAPI in *CMake* options
  - **DirectX Debug Layer** (Required by default on Windows 10)
    - Optional if you have choosen a different RenderAPI in *CMake* options
    - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"
  - **Mono 4.2** (Optional)
    - If you wish to compile managed assemblies using a Microsoft compiler (e.g. using Visual Studio) yet still be able to debug the generated assemblies, you must install Mono 4.2. and set up an environment variable MONO_INSTALL_DIR pointing to the Mono installation directory. When this is set up "pdb2mdb" script will trigger on next compile generating the needed debug symbols.
    - http://www.mono-project.com/download/
 
**Linux**
  - **OpenGL**
    - Required by default, but optional if you have choosen a different RenderAPI in *CMake* options
    - Debian/Ubuntu: *apt-get install libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev*
  - **X11**
    - Debian/Ubuntu: *apt-get install libx11-dev libxcursor-dev libxrandr-dev*
  - **LibUUID**
    - Debian/Ubuntu: *apt-get install uuid-dev*
  - **GTK+ 3.0** (Editor only)
    - Optional if not building the editor
    - Debian/Ubuntu: *apt-get install libgtk-3-dev*
  - **Mono** (Editor only)
    - Required for compilation of C# code. Optional if not building the editor.
    - Debian/Ubuntu: *apt-get install mono-complete*
  - **Bison & Flex**
    - Debian/Ubuntu: *apt-get install bison flex*
  - (Or equivalent packages for your distribution)

**All OS**
  - **Vulkan SDK** (Optional) 
    - Only needed if you selected the Vulkan render API during build configuration
    - https://lunarg.com/vulkan-sdk/
    - Set up VULKAN_SDK environment variable pointing to your instalation
  - **Python 3.5** (Optional)
    - Only needed if you plan on running Python scripts in the /Scripts folder
    - https://www.python.org/downloads/
  - **FMOD Low Level Programmer API 1.08.02** (Optional)
    - Only needed if you selected the FMOD audio module during build configuration
    - http://www.fmod.org/download/
    - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
    - Copy the dynamic libraries (.dll) from {INSTALLDIR}/api/lowlevel/lib into /bin folder in Banshee source code folder
      - Use logging libraries for the Debug builds, and non-logging for OptimizedDebug and Release builds
