# Compiling

## Quick start
Use the guide below to get B3D Framework up and running as quickly as possible. Scroll further below for advanced build options that allow for more customization.

- Install git (https://git-scm.com) and CMake 3.31.0 or higher (https://cmake.org)
  - Ensure they are added to your *PATH* environment variable
- Install other dependencies
  - See [here](#otherDeps)
- Run the following commands in the terminal/command line:
  - `git clone https://github.com/GameFoundry/bsf.git`
  - `cd bsf`
  - `mkdir Build`
  - `cd Build`
  - `cmake -G "$generator$" ..`
    - Where *$generator$* should be replaced with any of the supported generators. Some common ones:
	  - `Visual Studio 17 2022` - Visual Studio 2022
	  - `Unix Makefiles`
	  - `Ninja`
	  - `Xcode`
	  - See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
	- Optionally provide the `CMAKE_INSTALL_PREFIX` to override the default install path:
	  - `cmake -G "$generator$" -DCMAKE_INSTALL_PREFIX=/path/to/install ..`
  - `cmake --build . --config Release`
    - Alternatively you can also run your build system manually (depending on your choice of generator):
	  - Visual Studio solution is present at `bsf/Build/bsf.sln`
	  - XCode project is present at `bsf/Build`
	  - Makefiles are present at `bsf/Build`
  - `cmake --build . --config Release --target install`
	- Alternatively you can run the install target in your chosen build tool
    - Note that files install to the default install folder, unless you have overridden it as specified above
	 
## Customizing the build

Additional variables allow you to pick between the render API (Vulkan, D3D12, Metal, Null), audio module (FMOD, OpenAudio) among other options. Run *CMake* to see all options. Note that non-default *CMake* options might require additional dependencies to be installed, see [here](#other-dependencies).

Modify *CMAKE_INSTALL_PREFIX* to choose where the library gets installed after the *install* target is ran (e.g. `make install`, or running the *INSTALL* target in Visual Studio/XCode).

Modify *CMAKE_BUILD_TYPE* to pick what kind of a build you want. Note that this is ignored for workflow generators like Visual Studio or XCode, in which you can pick this within the IDE itself. Supported values are:
 - *Debug* - Builds and unoptimized version with debug symbols. This is generally what you want for development.
 - *RelWithDebInfo* - Builds an optimized version with debug symbols and various debug checks enabled.
 - *MinSizeRel* - Builds an optimized version attempting to keep the resultant binary as small as possible. Debug checks are disabled.
 - *Release* - Builds an optimized version with no debug checks.
 
You can choose to use a different *CMake* generator than those specified above, as long as the platform/compiler is supported:  
  - Supported platforms:
    - Windows 10/11
    - Linux (currently non functional while major refactor is in progress)
    - macOS 13.0 or newer on Apple Silicon (arm64)
  - Supported compilers:
    - MSVC++ 17.0 (Visual Studio 2022)
    - GCC 7.0 (or newer) (currently non functional while major refactor is in progress)
    - Apple Clang provided by a current Xcode installation on macOS
	
### Third party dependencies
B3D Framework relies on a variety of third party dependencies. A set of pre-compiled dependencies are provided for every supported platform/compiler and these will be fetched automatically by the build process. If required, the dependencies can also be compiled manually by following [this guide](dependencies.md). This can be required if the pre-compiled dependencies don't work with your platform (e.g. unsupported Linux distro) or if you wish to update to a newer dependency version.

Note that prebuilt dependencies only come as 64-bit, if you wish to build as 32-bit you will need to compile them manually.

## Other dependencies
The following dependencies will need to be installed manually. Which ones are required depend on the selected *CMake* options and your usage of B3D Framework (check text of each entry below).

**Windows**
  - **Windows SDK**
    - Version 10.0.26100.0, but others may work as well

**Linux**
  - **X11**
    - Debian/Ubuntu: `apt-get install libx11-dev libxcursor-dev libxrandr-dev libxi-dev`
  - **LibUUID**
    - Debian/Ubuntu: `apt-get install uuid-dev`
  - **LibICU**
    - Debian/Ubuntu: `apt-get install libicu-dev`
  - **Bison & Flex** (Optional)
    - Only needed if you plan on changing BSL syntax (B3D_BUILD_BSL_PARSER option in CMake)
    - Debian/Ubuntu: `apt-get install bison flex`
  - (Or equivalent packages for your distribution)

**macOS**
  - **Homebrew**
    - `/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"`
  - **Xcode and command-line tools**
    - Install a current Xcode version that supports the target macOS SDK
    - After installation run: `xcode-select --install`
  - **LibUUID**
    - `brew install ossp-uuid`
  - **Flex** (Optional)
    - Only needed if you plan on changing BSL syntax (B3D_BUILD_BSL_PARSER option in CMake)
    - `brew install flex`
  - **Bison** (Optional)
    - Only needed if you plan on changing BSL syntax (B3D_BUILD_BSL_PARSER option in CMake)
    - `brew install bison`
    - Add Homebrew's Bison to your path: `export PATH="/opt/homebrew/opt/bison/bin:$PATH"`

**All OS**
  - **Vulkan SDK 1.4.321.1*
    - Only needed if you explicitly select the Vulkan render API during build configuration
    - https://lunarg.com/vulkan-sdk/
    - Set up `VULKAN_SDK` environment variable pointing to your installation
    - **Additional setup for macOS Vulkan builds**
      - Set up the following environment variables:
        - `VULKAN_SDK = $SDK_DIR$/macOS`
        - `VK_LAYER_PATH = $SDK_DIR$/macOS/etc/vulkan/explicit_layer.d`
        - `VK_ICD_FILENAMES = $SDK_DIR$/macOS/etc/vulkan/icd.d/MoltenVK_icd.json`
      - Replacing `$SDK_DIR$` with the directory where you extracted the SDK
  - **FMOD Low Level Programmer API 1.08.02** (Optional)
    - Only needed if you selected the FMOD audio module during build configuration
    - http://www.fmod.org/download/
    - If CMake complains it cannot find FMOD, manually set the FMOD_INSTALL_DIRS to your installation directory 
    - Copy the dynamic libraries (.dll) from {INSTALLDIR}/api/lowlevel/lib into /bin folder in B3D Framework source code folder
      - Use logging libraries for the Debug builds, and non-logging for RelWithDebInfo and Release builds
