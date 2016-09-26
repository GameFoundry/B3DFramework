Porting									{#porting}
===============
[TOC]

This guide will try to offer you a solid set of guidelines how to go about porting Banshee to a different operating system.

# Compilation {#porting_a}
Banshee currently compiles using MSVC and Clang on Windows. There should be little issues compiling using Clang on other platforms. GCC was not tested but considering compilation works with two separate compilers there should be no major issues. Banshee also supports the CMake build system, which can generate make files and project files for various popular compilers and IDEs. 

This means that as far as the compilation goes most of the work should be done for you.

# Platform specific functionality {#porting_b}
Most of the porting work remains in adding platform specific functionality like file-system, windows and similar. Banshee comes with a fully working implementation of OpenGL, which means the rendering API is already cross-platform (for the most part), and the platform functionality mostly lies in various utility functionality.

Banshee was built with multi-platform in mind from day one, and it tries to minimize the amount of platform specific functionality as much as possible. The functionality that is platform specific is encapsulated so external code never needs to access it directly, making the porting process transparent to higher level systems. All of the platform specific functionality is cleanly marked either in an \#ifdef block or is in a separate source file with a special prefix.

Banshee is built in layers, higher layers referencing lower layers. This should make porting easier as you can start with the lowest layer and work your way up. This way you can compile and test layer by layer instead of needing to fully port the entire engine to properly compile it. Additionally a lot of functionality is in plugins and those generally don't have any platform specific code (except the OpenGL plugin), which should also help with dividing the work into manageable chunks. These aspects of Banshee should significantly help with the porting effort, so keep the layers/plugins in mind.

Aside from dividing the work by layers/plugins you should most definitely also divide it by functionality needed: editor requires significantly more platform specific code than a standalone game. You should first strive to port all the features used by the standalone game, then after everything is working should you proceed with working on editor features. You can use the `ExampleProject` as a test-bed for the standalone features. 

All the features that need porting are wrapped in a BS_PLATFORM \#ifdef block, or in files prefixed with "Win32". In a very limited set of cases there are also BS_COMPILER defines for functionality specific to a compiler. For every such block and file you will need to write equivalent code for the destination platform. 

Below you will find a fairly complete list of all such blocks/files that need to be modified, to give you a good idea of the scope. Each listed feature has an indication whether this is a engine or editor-only feature.
Additionally, not all features are critical, meaning you can get the engine to run without them (e.g. platform cursors or clipboard), which are also specially marked. For them it is suggested you implement a dummy version first, and only proceed with actual implementation once the critical features are done.

## Critical features {#porting_b_a}
A list of all critical features that require the engine to be ran standalone (no editor), in the rough order they should be implemented.

Feature                                         | Editor only 	| Library                       | Dummy implementation  | Relevant files										 | Description
------------------------------------------------|---------------|-------------------------------|-----------------------|--------------------------------------------------------|-----------------
File system*								   	| No			| BansheeUtility				| No					| BsFileSystem.h/BsWin32FileSystem.cpp 					 | Opening/creating files, iterating over directories
Dynamic library loading							| No			| BansheeUtility				| No					| BsDynLib.h/BsDynLib.cpp 							     | Loading dynamic libraries (.dll, .so)
OpenGL initialization*							| No			| BansheeGLRenderAPI			| No					| BsGLUtil.h, BsGLSupport.h/BsWin32GLSupport.cpp, BsWin32Context.h/BsWin32Context.cpp, BsWin32VideoModeInfo.cpp | Initializing the OpenGL context (or other context if non-OpenGL API is used for the port)
Window creation*								| No			| BansheeUtility, BansheeGLRenderAPI | No				| BsWin32Window.h/BsWin32Window.cpp, BsWin32Platform.h/BsWin32Platform.cpp, BsWin32RenderWindow.h/BsWin32RenderWindow.cpp | Creating and interacting with the window
OS message loop*								| No			| BansheeCore					| No					| BsWin32Platform.h/BsWin32Platform.cpp 				 | Running the main message loop, responding to its events
Input*											| No			| BansheeCore					| Maybe					| BsPlatform.h/BsWin32Platform.cpp 						 | Receive input from OS (mouse, keyboard)
UUID generation									| No			| BansheeUtility				| No					| BsPlatformUtility.h/BsWin32PlatformUtility.cpp 		 | Generate UUID/GUID

## Non-critical features {#porting_b_b} 
A list of non-critical features, and editor-only features, in the rough order they should be implemented. You should be able to get the engine running without these, or with just dummy implementations (that do nothing).

Feature                                         | Editor only 	| Library                       | Dummy implementation  | Relevant files											| Description
------------------------------------------------|---------------|-------------------------------|-----------------------|-----------------------------------------------------------|-------------------
Crash handler									| No			| BansheeUtility				| Yes					| BsCrashHandler.h, ThreadPool.cpp, BansheeEditorExec.cpp, Main.cpp (in Game project) | Save a log with a callstack when a crash occurs
Process termination								| No			| BansheeUtility				| Yes					| BsPlatformUtility.h/BsWin32PlatformUtility.cpp | Terminate the application on user request
Cursor											| No			| BansheeCore					| Yes					| BsPlatform.h/BsWin32Platform.cpp | Get/set cursor position, clip cursor, change cursor look
Window non-client areas 						| Yes			| BansheeCore					| Yes					| BsPlatform.h/BsWin32Platform.cpp | Set up OS window borders used for resize/move operations
Changing executable icon*						| Yes			| BansheeCore					| Yes					| BsPlatform.h/BsWin32Platform.cpp | Ability to inject an icon into an executable, used by the build process
Clipboard										| No			| BansheeUtility				| Yes					| BsPlatformUtility.h/BsWin32PlatformUtility.cpp | Ability to copy/paste text from the editor and the OS
Converting keyboard code to character			| Yes			| BansheeUtility				| Yes					| BsPlatformUtility.h/BsWin32PlatformUtility.cpp | Converting keyboard codes into a character symbol
Retrieving MAC address							| Yes			| BansheeUtility				| Yes					| BsPlatformUtility.h/BsWin32PlatformUtility.cpp | Retrieving a MAC address of the computer
Browse file/folder dialogs						| Yes			| BansheeUtility				| Yes					| BsPlatformUtility.h/BsWin32BrowseDialogs.cpp, BsPlatformUtility.h/BsWin32PlatformUtility.cpp | OS built-in dialogs for browsing/creating files/folders
Folder monitor*									| Yes			| BansheeCore					| Yes					| BsFolderMonitor.h, BsWin32FolderMonitor.h/BsWin32FolderMonitor.cpp | Monitor that can track and report  file changes/additions/deletions in a folder
Drop target*									| Yes			| BansheeCore					| Yes					| BsWin32DropTarget.h/BsWin32Platform.cpp | Target that can be used for drag and drop operations initiated by the OS
Script compilation								| Yes			| BansheeMono					| Yes					| BsMonoManager.cpp | Starting of the external compiler tool, and copying its output files into proper location.
Game build										| Yes			| Game, BansheeEditor, MBansheeEditor | Yes				| Main.cpp in Game, BuildManager.cpp in BansheeEditor, BuildManager.cs in MBansheeEditor | Copying the right libraries, and assemblies during build. Platform-specific options in the build manager.
Splash Screen									| Yes			| BansheeEngine					| Yes					| BsSplashScreen.cpp | Displaying a splash screen with the Banshee logo
MonoDevelop integration*						| Yes			|BansheeEditor					| Yes					| BsCodeEditor.cpp | Ability to open/edit script files with MonoDevelop, similar to how VS integration works

(*) - This is a larger and/or non-trivial task. Most listed tasks are just a few dozen up to a couple of hundred lines of code, or if larger they're trivially simple. Larger tasks are a few hundred lines of code (less than a 1000) and/or might be more difficult to implement than others. This is noted here to give you a better idea of the scope.

# Compiling third party dependencies {#porting_c} 
In order to run Banshee on different platforms you will also need to compile all of Banshee's dependencies. Most of Banshee's dependencies are only used in plugins, which should make it easier to compile and test them individually.

All used dependencies are already multi-platform and you should have little trouble compiling them for major platforms. See "CompilingDependenciesManually.txt" (distributed with the source code) for information which dependencies are needed.

# Mobile platforms {#porting_d} 
If building for mobile platforms you will also need to provide a compatible render API plugin. It is suggested you use the BansheeOpenGL plugin as an example of creating such an API (for example OpenGL ES). API's like Metal or Vulkan for mobiles will require more work but you can still use the existing render API plugins for a good basis of what needs to be implemented.
