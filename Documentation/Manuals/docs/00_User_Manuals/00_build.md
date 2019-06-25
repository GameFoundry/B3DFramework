---
title: Setting up a project
---

This manual will explain how to set up your own project and link it with bs::framework. We'll explore two options:
 - **Link with binaries** - Either download the precompiled set of binaries or compile them yourself. Then create a project that links with the binaries. If you do not wish to compile `bsf` yourself, this is the best approach to follow.
 - **Link with source** - Grab the source code and include it as part of your own project. Requires you to go through the same steps as if you were compiling `bsf` from scratch, but allows you to easily recompile the framework from latest source, or due to your own modifications. If you plan on making changes to the framework, or want to frequently update to the latest version, this is the best approach. Linux users should also prefer this approach as binary incompatibilities between different Linux distributions might make the provided binaries incompatible with your particular distribution.

# Link with binaries
 
The first step is to actually get `bsf` binaries. You have two options:
 1. Get the precompiled set of binaries from the [Download](https://www.bsframework.io/download.html) page. This is the simplest and easiest solution.
 2. Grab the source code and compile the program yourself. You can either get the latest code from the [GitHub](https://github.com/GameFoundry/bsf) repository, or download an archived version of one of the releases from the [Download](https://www.bsframework.io/download.html) page. Follow the compilation guide on the GitHub page for detailed instructions. Note that you should probably consider going with the **Link with source** approach if you plan on compiling `bsf` anyway.
 
Whether you downloaded precompiled binaries or compiled them yourself you should now have three folders containing the entirety of `bsf`:
 - `bin` - Contains dynamic libraries and data files
 - `include` - Contains public header files
 - `lib` - Contains shared and import libraries
 
If you downloaded a precompiled package these folders will be part of the downloaded archive.
If you manually compiled they will be placed in the chosen `install` folder after you run the install step (as described by the compilation guide on the GitHub page).

In order to get your project running you will need to:
 - Provide a path to the bsf's include folder to your compiler
 - Link with the `bsf` dynamic/shared library
 - Make sure your executable can find all the dynamic libraries and data files

We'll handle these steps by creating a CMake project that will ultimately work with any major build tool (like Visual Studio, XCode or Makefiles).
 
## CMake
CMake is a build system that allows you define a project that can then be used for building across multiple platforms and build tools. Such CMake project can be used to create a Visual Studio solution, XCode project or Unix Makefiles. `bsf` also comes with modules that make using `bsf` in your CMake project easier.

Note that CMake is fairly complex and we don't go into details about all of its syntax. There are many tutorials that do that. However our project will be pretty simple and should be easy to understand even to a newcomer to CMake.

## Preparation
First create a new directory where you would like your project to live. Grab the `Findbsf.cmake` file from [here](https://github.com/GameFoundry/bsf/raw/master/Source/CMake/Modules/Findbsf.cmake) and place it in the root of your project folder. This file will be used for helping us find `bsf` libraries and includes.

## CMake project
In your project folder create `CMakeLists.txt`, and fill it out like so:
```
# Minimum version of CMake as required by bsf
cmake_minimum_required (VERSION 3.12.0)

# Name of your project
project (myProject)

# Add necessary compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR 
   CMAKE_CXX_COMPILER_ID MATCHES "AppleClang" OR
   CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	add_compile_options(-fno-rtti)
endif()

# Make sure to use the C++14 standard
set(CMAKE_CXX_STANDARD 14)

# Build an executable from the provided C++ files
add_executable(myApp WIN32 "Main.cpp")

# Path to where you have installed bsf
set(bsf_INSTALL_DIR "C:/path/to/bsf/install")

# Let CMake know where to find the Findbsf.cmake file (at current folder)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

# Find bsf libraries and headers
find_package(bsf REQUIRED)

# Link bsf library with your application
target_link_libraries(myApp PRIVATE bsf)
```

Make sure to modify `C:/path/to/bsf/install` and set it to the path where you extracted/installed `bsf`.

## Code
The CMake file above references `Main.cpp` file. The most basic file that runs the framework looks like so:
```
#include "BsEntry.h"

int bs_main(int argc, char* argv[])
{
	using namespace bs;

	VideoMode videoMode(1280, 720);
	Application::startUp(videoMode, "Example", false);
	Application::instance().runMainLoop();
	Application::shutDown();

	return 0;
}
```

We will explain the code above in following manuals, but for now you can just copy/paste it and use it for the starting point.

Place the `Main.cpp` into your project's root folder.

## Get CMake
If you don't already have it, grab the latest version of CMake from [www.cmake.org](https://cmake.org/download/). Make sure to grab version 3.12.4 or later. 

## Build
You are now ready to build the project. From your project's root folder execute the following commands in terminal/command line:
 - `mkdir build`
 - `cd build`
 - `cmake -G "$generator" ..`
   - Where *$generator* should be replaced with any of the supported generators. Some common ones:
     - `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
	 - `Unix Makefiles`
	 - `Ninja`
	 - `Xcode`
	 - See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
   - If your system complains that `cmake` is not a valid command, you need to add the path of CMake binaries to your $PATH environment variable
	 
Your build files will be output to the `build` folder. 
 - If you are using Visual Studio open up the `build/myProject.sln` solution and proceed building as normal
 - If you are using XCode open up the project in the `build` folder and proceed building as normal
 - If you are using Makefiles simply run `make` within the `build` folder
 
## Copy files
Before you can run the application you must first copy the data files and dynamic libraries so that your application can find them when it starts. 
 - Copy the `bin/Data` folder to the location where is your executable is located
 - If on Windows you should also copy any .dll files in the `bin` folder to where your executable is located. Note that precompiled binaries package comes with separate set of dynamic libraries for Debug and Release builds in `bin/Debug` and `bin/Release` folders. You should copy them to the appropriate folder depending on the configuration you are building with. 
 
For example in Visual Studio the executable will be placed at `Release/` folder within your project root, for a Release configuration. This is where you should place the dynamic libraries and the data files.

![Structure of the executable folder on Windows](../Images/BuildStructure.png)

## Run
You are now ready to run the application. If you used the default `Main.cpp` code above an empty window should open.

# Link with source
This is an alternative approach to linking with binaries. `bsf` source code will be included as part of your own project, making sure any changes you make can easily be compiled and tested.

## Preparation
First make sure to follow the compilation guide on [GitHub](https://github.com/GameFoundry/bsf). You only need to do the first few steps of that guide and ensure you have installed all the required dependencies, as well as checked out `bsf` from its repository. Actual CMake calls can be skipped at this point.

## CMake project
Create a new project folder, with a `CMakeLists.txt` file with the following contents:
```
# Minimum version of CMake as required by bsf
cmake_minimum_required (VERSION 3.12.0)

# Name of your project
project (myProject)

# Let the system know where `bsf` source code is located
set(BSF_DIRECTORY "C:/bsf" CACHE STRING "bsf directory")

# Include bsf projects and their CMake code
include(${BSF_DIRECTORY}/Source/CMake/ProjectDefaults.cmake)

# Build an executable from the provided C++ files
add_executable(myApp WIN32 "Main.cpp")

# Add bsf as a dependency
target_link_libraries(myApp bsf)

# Add required plugin bsf dependencies (as selected through CMake properties)
add_engine_dependencies(myApp)

# Add optional dependencies (specify all, some, or none of these as required)
add_dependencies(myApp bsfFBXImporter bsfFontImporter bsfFreeImgImporter)

# When 'myApp' target is built, copy required binaries from bsf
install_dll_on_build(myApp ${BSF_DIRECTORY})
```

Make sure to set `C:/bsf` to the directory where you cloned `bsf`.

## Code
The CMake file above references `Main.cpp` file. The most basic file that runs the framework looks like so:
```
#include "BsEntry.h"

int bs_main(int argc, char* argv[])
{
	using namespace bs;

	VideoMode videoMode(1280, 720);
	Application::startUp(videoMode, "Example", false);
	Application::instance().runMainLoop();
	Application::shutDown();

	return 0;
}
```

We will explain the code above in following manuals, but for now you can just copy/paste it and use it for the starting point.

Place the `Main.cpp` into your project's root folder.

## Build
You are now ready to build the project. From your project's root folder execute the following commands in terminal/command line:
 - `mkdir build`
 - `cd build`
 - `cmake -G "$generator" ..`
   - Where *$generator$* should be replaced with any of the supported generators. Some common ones:
     - `Visual Studio 15 2017 Win64` - Visual Studio 2017 (64-bit build)
	 - `Unix Makefiles`
	 - `Ninja`
	 - `Xcode`
	 - See all valid generators: [cmake-generators](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html)
   - If your system complains that `cmake` is not a valid command, you need to add the path of CMake binaries to your $PATH environment variable
	 
Your build files will be output to the `build` folder. 
 - If you are using Visual Studio open up the `build/myProject.sln` solution and proceed building as normal
 - If you are using XCode open up the project in the `build` folder and proceed building as normal
 - If you are using Makefiles simply run `make` within the `build` folder
 
After building any executables/dynamic libraries will be placed in the `bin` sub-folder in your `build` folder, and any static/shared libraries will be placed in the `lib` sub-folder.

Unlike with **Link with binaries** approach you do not need to do any file copying, until you are ready to send the application to your users. After the build the application is ready to run.
