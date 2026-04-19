//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Plugin/B3DPluginLoader.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Audio-Internal
	 *  @{
	 */

	/** Creates and destroys a specific audio system implementation. */
	class B3D_EXPORT AudioFactory
	{
	public:
		virtual ~AudioFactory() = default;

		/** Initializes the audio system. */
		virtual void StartUp() = 0;

		/** Shuts down the audio system. */
		virtual void ShutDown() = 0;
	};

	/** Takes care of loading, initializing and shutting down of a particular audio system implementation. */
	class B3D_EXPORT AudioManager : public Module<AudioManager>
	{
	public:
		/**
		 * Initializes the audio manager and a particular audio system implementation.
		 *
		 * @param	pluginName	Name of the plugin containing an audio system implementation.
		 */
		AudioManager(const String& pluginName);
		~AudioManager();

	private:
		LoadedPlugin mPlugin;
		AudioFactory* mFactory = nullptr;
	};

	/** @} */
} // namespace b3d
