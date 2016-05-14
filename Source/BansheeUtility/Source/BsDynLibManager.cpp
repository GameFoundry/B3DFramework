//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDynLibManager.h"
#include "BsDynLib.h"

namespace BansheeEngine
{
	DynLibManager::DynLibManager()
	{
	}

    DynLib* DynLibManager::load(const String& name)
    {
		// Add the extension (.dll, .so, ...) if necessary.
		String filename = name;
		const int length = filename.length();
		const String extension = "." + DynLib::extension;
		const int ext_length = extension.length();
		if (length <= ext_length || filename.substr(length - ext_length) != extension)
			filename += extension;

		auto iterFind = mLoadedLibraries.find(filename);
		if (iterFind != mLoadedLibraries.end())
		{
			return iterFind->second;
		}
		else
		{
	        DynLib* newLib = new (bs_alloc<DynLib>()) DynLib(filename);
        	mLoadedLibraries[filename] = newLib;

	        return newLib;
		}
    }

	void DynLibManager::unload(DynLib* lib)
	{
		auto iterFind = mLoadedLibraries.find(lib->getName());
		if (iterFind != mLoadedLibraries.end())
		{
			mLoadedLibraries.erase(iterFind);
		}

		lib->unload();
		bs_delete(lib);
	}

	DynLibManager::~DynLibManager()
    {
        // Unload & delete resources in turn
        for(auto& entry : mLoadedLibraries)
        {
            entry.second->unload();
			bs_delete(entry.second);
        }

        // Empty the list
        mLoadedLibraries.clear();
    }

	DynLibManager& gDynLibManager()
	{
		return DynLibManager::instance();
	}
}
