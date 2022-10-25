//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullAudioPrerequisites.h"
#include "BsNullAudio.h"
#include "Audio/BsAudioManager.h"
#include "Importer/BsImporter.h"

namespace bs
{
	class NullFactory : public AudioFactory
	{
	public:
		void StartUp()
		{
			Audio::StartUp<NullAudio>();
		}

		void ShutDown()
		{
			Audio::ShutDown();
		}
	};

	/**	Returns a name of the plugin. */
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "NullAudio";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
	{
		return bs_new<NullFactory>();
	}

	/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
	extern "C" BS_PLUGIN_EXPORT void unloadPlugin(NullFactory* instance)
	{
		bs_delete(instance);
	}
} // namespace bs
