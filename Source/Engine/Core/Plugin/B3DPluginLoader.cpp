//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Plugin/B3DPluginLoader.h"

#if B3D_MONOLITHIC_BUILD
#include "B3DStaticPluginRegistry.h"
#endif

#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"

namespace b3d
{
	LoadedPlugin PluginLoader::Load(const String& pluginName, void* passThrough)
	{
		LoadedPlugin result;

#if B3D_MONOLITHIC_BUILD
		const StaticPluginEntry* entry = FindStaticPlugin(pluginName);
		if(entry != nullptr)
		{
			if(entry->LoadFn != nullptr)
			{
				if(passThrough == nullptr)
					result.ReturnValue = entry->LoadFn();
				else
					result.ReturnValue = ((void*(*)(void*))entry->LoadFn)(passThrough);
			}

			result.UnloadFn = entry->UnloadFn;
			result.UpdateFn = entry->UpdateFn;
			result.PluginName = entry->Name;
			return result;
		}
		// Fall through to dynamic loading for plugins not in static registry
		// (e.g. importers, which always remain dynamic)
#endif

		// Dynamic loading path
		DynamicLibrary* library = DynamicLibraryManager::Instance().Load(pluginName);
		if(library != nullptr)
		{
			result.Library = library;

			if(passThrough == nullptr)
			{
				auto loadFn = (void*(*)())library->GetSymbol("LoadPlugin");
				if(loadFn != nullptr)
					result.ReturnValue = loadFn();
			}
			else
			{
				auto loadFn = (void*(*)(void*))library->GetSymbol("LoadPlugin");
				if(loadFn != nullptr)
					result.ReturnValue = loadFn(passThrough);
			}

			result.UnloadFn = (void(*)(void*))library->GetSymbol("UnloadPlugin");
			result.UpdateFn = (void(*)())library->GetSymbol("UpdatePlugin");

			auto getNameFn = (const char*(*)())library->GetSymbol("GetPluginName");
			if(getNameFn != nullptr)
				result.PluginName = getNameFn();
		}

		return result;
	}

	void PluginLoader::Unload(LoadedPlugin& plugin)
	{
		if(plugin.UnloadFn != nullptr)
			plugin.UnloadFn(plugin.ReturnValue);

		if(plugin.Library != nullptr)
			DynamicLibraryManager::Instance().Unload(plugin.Library);

		plugin = {};
	}

	const char* PluginLoader::GetName(const LoadedPlugin& plugin)
	{
		return plugin.PluginName ? plugin.PluginName : "";
	}
} // namespace b3d
