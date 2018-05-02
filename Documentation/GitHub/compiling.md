# Compiling

## Quick start
Use the guide below to get bs::framework up and running as quickly as possible. Scroll further below for advanced build options that allow for more customization.

- Install git (https://git-scm.com) and CMake 3.9.0 or higher (https://cmake.org)
  - Ensure they are added to your *PATH* environment variable
- Install other dependencies
  - See [here](#otherDeps)
- Run the following commands in the terminal/command line:
  - `git clone https://github.com/GameFoundry/bsf.git`
  - `cd bsf`
  - `mkdir Build`
  - `cd Build`
  - `cmake -G "$generator$" ../`
    - Where *$generator$* should be replaced with any of the supported generators. Some common ones:
	  - `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
	  - `Unix Makefiles`
	  - `Ninja`
	  - `Xcode`
	- See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
- Build the project using your chosen tool
  - Build files will be placed in `bsf\Build` folder
	 
## Customizing the build

Additional variables allow you to pick between the render API (Vulkan, DirectX, OpenGL), audio module (FMOD, OpenAudio) among other options. Run *CMake* to see all options. Note that non-default *CMake* options might require additional dependencies to be installed, see [here](#otherDeps).

Modify *CMAKE_INSTALL_PREFIX* to choose where the library gets installed after the *install* target is ran (e.g. `make install`, or running the *INSTALL* target in Visual Studio/XCode).

Modify *CMAKE_BUILD_TYPE* to pick what kind of a build you want. Note that this is ignored for workflow generators like Visual Studio or XCode, in which you can pick this within the IDE itself. Supported values are:
 - *Debug* - Builds and unoptimized version with debug symbols. This is generally what you want for development.
 - *OptimizedDebug* - Builds an optimized version with debug symbols.
 - *Release* - Builds an optimized version with no debug symbols.
 
You can choose to use a different *CMake* generator than those specified above, as long as the platform/compiler is supported:  
  - Supported platforms:
    - Windows 7, 8, 10
    - Linux
    - macOS 10.11 or newer
  - Supported compilers:
    - MSVC++ 15.0 (Visual Studio 2017)
    - GCC 7.0 (or newer)
    - Clang 5.0 (or newer)
	
### <a name="dependencies"></a>Third party dependencies
bs::framework relies on a variety of third party dependencies. A set of pre-compiled dependencies are provided for every supported platform/compiler and these will be fetched automatically by the build process. If required, the dependencies can also be compiled manually by following [this guide](dependencies.md). This can be required if the pre-compiled dependencies don't work with your platform (e.g. unsupported Linux distro) or if you wish to update to a newer dependency version.

Note that prebuilt dependencies only come as 64-bit, if you wish to build as 32-bit you will need to compile them manually.

## <a name="otherDeps"></a>Other dependencies
The following dependencies will need to be installed manually. Which ones are required depend on the selected *CMake* options and your usage of bs::framework (check text of each entry below).

**Windows**
  - **DirectX SDK** (Required by default on Windows 7 or earlier)
	- Optional if you have choosen a different RenderAPI in *CMake* options
    - Set up DXSDK_DIR environment variable pointing to the DirectX instalation
  - **Windows SDK** (Required by default on Windows 8 or later)
	- Optional if you have choosen a different RenderAPI in *CMake* options
  - **DirectX Debug Layer** (Required by default on Windows 10)
    - Optional if you have choosen a different RenderAPI in *CMake* options
    - Go to Settings panel (type "Settings" in Start)->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"
 
**Linux**
  - **OpenGL**
    - Required by default, but optional if you have chosen a different RenderAPI in *CMake* options
    - Debian/Ubuntu: `apt-get install libgl1-mesa-dev libglu1-mesa-dev mesa-common-dev`
  - **X11**
    - Debian/Ubuntu: `apt-get install libx11-dev libxcursor-dev libxrandr-dev libxi-dev`
  - **LibUUID**
    - Debian/Ubuntu: `apt-get install uuid-dev`
  - **LibICU**
    - Debian/Ubuntu: `apt-get install libicu-dev`
  - **Bison & Flex** (Optional)
    - Only needed if you plan on changing BSL syntax (BUILD_BSL option in CMake)
    - Debian/Ubuntu: `apt-get install bison flex`
  - (Or equivalent packages for your distribution)

**macOS**
  - **Homebrew**
    - `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
  - **Xcode 9**
    - Grab from App Store
    - After install make sure to run: `xcode-select -install`
  - **LibUUID**
    - `brew install ossp-uuid`
  - **Flex** (Optional)
    - Only needed if you plan on changing BSL syntax (BUILD_BSL option in CMake)
    - `brew install flex`
  - **Bison** (Optional)
    - Only needed if you plan on changing BSL syntax (BUILD_BSL option in CMake)
    - `brew install bison`
    - Make sure old version of Bison that comes with Xcode is overriden:
      - Add this to $HOME/.bash_profile: `export PATH=“/usr/local/opt/bison/bin:$PATH”`
      - `mv /usr/bin/bison /usr/bin/bison-2.3`

**All OS**
  - **Vulkan SDK 1.0.65.1** (Optional) 
    - Only needed if you selected the Vulkan render API during build configuration
    - https://lunarg.com/vulkan-sdk/
    - Set up VULKAN_SDK environment variable pointing to your instalation
  - **FMOD Low Level Programmer API 1.08.02** (Optional)
    - Only needed if you selected the FMOD audio module during build configuration
    - http://www.fmod.org/download/
    - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
    - Copy the dynamic libraries (.dll) from {INSTALLDIR}/api/lowlevel/lib into /bin folder in bs::framework source code folder
      - Use logging libraries for the Debug builds, and non-logging for OptimizedDebug and Release builds
