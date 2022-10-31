//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAPrerequisites.h"
#include "Audio/BsAudioManager.h"
#include "BsOAAudio.h"
#include "BsOAImporter.h"
#include "Importer/BsImporter.h"

using namespace bs;

class OAFactory : public AudioFactory
{
public:
	void StartUp()
	{
		Audio::StartUp<OAAudio>();
	}

	void ShutDown()
	{
		Audio::ShutDown();
	}
};

/**	Returns a name of the plugin. */
extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
{
	static const char* pluginName = "OpenAudio";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
{
	OAImporter* importer = B3DNew<OAImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	return B3DNew<OAFactory>();
}

/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
extern "C" BS_PLUGIN_EXPORT void unloadPlugin(OAFactory* instance)
{
	B3DDelete(instance);
}
