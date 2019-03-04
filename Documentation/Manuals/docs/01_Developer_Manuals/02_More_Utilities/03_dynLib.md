---
title: Dynamic libraries
---

In order to load dynamic libraries like .dll or .so, you can use the @bs::DynLibManager module. It has two main methods:
 - @bs::DynLibManager::load - Accepts a file name to the library (without extension), and returns the @bs::DynLib object if the load is successful or null otherwise. 
 - @bs::DynLibManager::unload - Unloads a previously loaded library.
 
Once the library is loaded you can use the **DynLib** object and its @bs::DynLib::getSymbol method to retrieve a function pointer within the dynamic library, and call into it. 

~~~~~~~~~~~~~{.cpp}
// Load library
DynLib* myLibrary = DynLibManager::instance().load("myPlugin");

// Retrieve function pointer (symbol) to the "loadPlugin" method
typedef void* (*LoadPluginFunc)();
LoadPluginFunc loadPluginFunc = (LoadPluginFunc)myLibrary->getSymbol("loadPlugin");

// Call the function
loadPluginFunc();

// Assuming we're done, unload the plugin
DynLibManager::instance().unload(myLibrary);
~~~~~~~~~~~~~