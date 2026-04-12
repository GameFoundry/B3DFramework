//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Information about a plugin that has been loaded through PluginLoader. */
	struct LoadedPlugin
	{
		void* ReturnValue = nullptr;
		void (*UnloadFn)(void*) = nullptr;
		void (*UpdateFn)() = nullptr;
		const char* PluginName = nullptr;
		DynamicLibrary* Library = nullptr;
	};

	/** Unified plugin loader that works transparently in both dynamic library and monolithic modes. */
	class B3D_EXPORT PluginLoader
	{
	public:
		/**
		 * Loads a plugin by name.
		 * In dynamic library mode: loads the dynamic library and calls its LoadPlugin() entry point.
		 * In monolithic mode: looks up the static registry and calls the registration function.

		 * Falls through to dynamic loading for plugins not in the static registry.

		 * @param	pluginName		Name of the plugin to load.
		 * @param	passThrough		Optional parameter that will be passed to the LoadPlugin function. Looks up LoadPlugin function with a single
		 *							void* parameter if provided, otherwise looks up LoadPlugin function with no parameters.
		 * @return					LoadedPlugin with the result and callbacks for lifecycle management.
		 */
		static LoadedPlugin Load(const String& pluginName, void* passThrough = nullptr);

		/** Unloads a previously loaded plugin. Calls the unload function if available, then unloads the dynamic library in dynamic library mode. */
		static void Unload(LoadedPlugin& plugin);

		/** Gets the plugin name. */
		static const char* GetName(const LoadedPlugin& plugin);
	};

	/** @} */
} // namespace b3d
