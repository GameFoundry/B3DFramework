//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAPrerequisites.h"
#include "BsAudioManager.h"
#include "BsOAAudio.h"
#include "BsOAImporter.h"
#include "BsImporter.h"

namespace BansheeEngine
{
	class OAFactory : public AudioFactory
	{
	public:
		void startUp() override
		{
			Audio::startUp<OAAudio>();
		}

		void shutDown() override
		{
			Audio::shutDown();
		}
	};

	/**	Returns a name of the plugin. */
	extern "C" BS_OA_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "OpenAudio";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_OA_EXPORT void* loadPlugin()
	{
		OAImporter* importer = bs_new<OAImporter>();
		Importer::instance()._registerAssetImporter(importer);

		return bs_new<OAFactory>();
	}

	/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
	extern "C" BS_OA_EXPORT void unloadPlugin(OAFactory* instance)
	{
		bs_delete(instance);
	}
}