//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullAudioPrerequisites.h"
#include "B3DNullAudio.h"
#include "Audio/B3DAudioManager.h"
#include "Importer/B3DImporter.h"

using namespace b3d;

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

extern "C" void* LoadPlugin_bsfNullAudio()
{
	return static_cast<void*>(B3DNew<NullFactory>());
}

extern "C" void UnloadPlugin_bsfNullAudio(void* instance)
{
	B3DDelete(static_cast<AudioFactory*>(instance));
}

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "NullAudio";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfNullAudio();
}

/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(NullFactory* instance)
{
	UnloadPlugin_bsfNullAudio(static_cast<void*>(instance));
}
