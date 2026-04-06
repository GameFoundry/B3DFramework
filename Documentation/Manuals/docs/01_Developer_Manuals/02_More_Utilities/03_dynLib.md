---
title: Dynamic libraries
---

In order to load dynamic libraries like .dll or .so, you can use the @b3d::DynamicLibraryManager module. It has two main methods:
 - @b3d::DynamicLibraryManager::Load - Accepts a file name to the library (without extension), and returns the @b3d::DynamicLibrary object if the load is successful or null otherwise. 
 - @b3d::DynamicLibraryManager::Unload - Unloads a previously loaded library.
 
Once the library is loaded you can use the **DynamicLibrary** object and its @b3d::DynamicLibrary::GetSymbol method to retrieve a function pointer within the dynamic library, and call into it. 

~~~~~~~~~~~~~{.cpp}
// Load library
DynamicLibrary* myLibrary = GetDynamicLibraryManager().Load("myPlugin");

// Retrieve function pointer (symbol) to the "LoadPlugin" method
typedef void* (*LoadPluginFunc)();
LoadPluginFunc loadPluginFunc = (LoadPluginFunc)myLibrary->GetSymbol("LoadPlugin");

// Call the function
loadPluginFunc();

// Assuming we're done, unload the plugin
GetDynamicLibraryManager().Unload(myLibrary);
~~~~~~~~~~~~~
