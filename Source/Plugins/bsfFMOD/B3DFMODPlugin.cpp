//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODPrerequisites.h"
#include "Audio/B3DAudioManager.h"
#include "B3DFMODAudio.h"
#include "B3DFMODImporter.h"
#include "Importer/B3DImporter.h"

using namespace b3d;

class FMODFactory : public AudioFactory
{
public:
	void StartUp() override
	{
		Audio::StartUp<FMODAudio>();
	}

	void ShutDown() override
	{
		Audio::ShutDown();
	}
};

extern "C" void* LoadPlugin_bsfFMOD()
{
	FMODImporter* importer = B3DNew<FMODImporter>();
	Importer::Instance().RegisterAssetImporter(importer);

	return static_cast<void*>(B3DNew<FMODFactory>());
}

extern "C" void UnloadPlugin_bsfFMOD(void* instance)
{
	B3DDelete(static_cast<AudioFactory*>(instance));
}

#if !B3D_MONOLITHIC_BUILD
/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "bsfFMOD";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_bsfFMOD();
}

/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(FMODFactory* instance)
{
	UnloadPlugin_bsfFMOD(static_cast<void*>(instance));
}
#endif
