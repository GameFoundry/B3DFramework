# Compiling

Banshee is available on two branches:
 - **preview** - This branch always contains latest "stable" pre-release version.
 - **master** - This branch contains the latest changes, might be quite unstable and might not even compile. Use this if you absolutely need the bleeding edge changes.
 
It can be compiled using:
 - MSVC++ 14.0 (Visual Studio 2015)
 - Clang
 
Use CMake to generate a VS solution, makefile or other type of build file. CMake root file is available at Source/CMakeLists.txt.
 
Banshee currently only compiles on Windows, but Mac & Linux ports are coming soon.

# Third party dependencies
Aside from the source code you will also need various third party dependencies. You may retrieve/compile those dependencies yourself by following a guide in "CompilingDependenciesManually.txt" (provided with the source code). 

Optionally, if you are using VS2015 you can avoid compiling dependencies by downloading the archive below, containing a set of pre-compiled dependencies. These should be extracted in the root of the directory containing Banshee source code.

[Download dependencies (VS2015)] (http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_v0.3.0.zip)

For older versions check the git release tag descriptions for links.

## Windows dependencies
For Windows additional dependencies not included in the dependencies package are needed.

Make sure to:
 - Install Windows SDK if on Windows 8 or higher.
 - Install DirectX SDK.
   - Set up DXSDK_DIR environment variable to point to the SDK install directory.
 - Install DirectX Debug Layer in Windows 10:
   - Settings panel->System->Apps & features->Manage optional Features->Add a feature->Select "Graphics Tools"

# Data files
You will also need data files (non-code resources like images, shaders, etc.) in order to run the editor, engine and example projects.

[Download data files] (http://bearishsun.thalassa.feralhosting.com/BansheeData_v0.3.0.zip)

For older versions check the git release tag descriptions for links.
