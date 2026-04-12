//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOAPrerequisites.h"
#include "Audio/B3DAudioManager.h"
#include "B3DOAAudio.h"
#include "B3DOAImporter.h"
#include "Importer/B3DImporter.h"

using namespace b3d;

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

extern "C" void* LoadPlugin_bsfOpenAudio()
{
	OAImporter* importer = B3DNew<OAImporter>();
	Importer::Instance().RegisterAssetImporter(importer);

	return static_cast<void*>(B3DNew<OAFactory>());
}

extern "C" void UnloadPlugin_bsfOpenAudio(void* instance)
{
	B3DDelete(static_cast<AudioFactory*>(instance));
}

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "OpenAudio";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfOpenAudio();
}

/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(OAFactory* instance)
{
	UnloadPlugin_bsfOpenAudio(static_cast<void*>(instance));
}
