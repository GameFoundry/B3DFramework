//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "bsfFMOD";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	FMODImporter* importer = B3DNew<FMODImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	return B3DNew<FMODFactory>();
}

/**	Exit point of the plugin. Called by the engine before the plugin is unloaded. */
extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(FMODFactory* instance)
{
	B3DDelete(instance);
}
