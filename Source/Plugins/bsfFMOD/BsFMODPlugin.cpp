//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODPrerequisites.h"
#include "Audio/BsAudioManager.h"
#include "BsFMODAudio.h"
#include "BsFMODImporter.h"
#include "Importer/BsImporter.h"

namespace bs
{
	class FMODFactory : public AudioFactory
	{
	public:
		void startUp() override
		{
			Audio::startUp<FMODAudio>();
		}

		void shutDown() override
		{
			Audio::shutDown();
		}
	};

	/**	Returns a name of the plugin. */
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "bsfFMOD";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
	{
		FMODImporter* importer = bs_new<FMODImporter>();
		Importer::instance()._registerAssetImporter(importer);

		return bs_new<FMODFactory>();
	}

	/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
	extern "C" BS_PLUGIN_EXPORT void unloadPlugin(FMODFactory* instance)
	{
		bs_delete(instance);
	}
}