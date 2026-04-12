//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFBXPrerequisites.h"
#include "B3DFBXImporter.h"
#include "Importer/B3DImporter.h"

using namespace b3d;

/**	Returns a name of the plugin. */
extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "FBXImporter";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	FBXImporter* importer = B3DNew<FBXImporter>();
	Importer::Instance().RegisterAssetImporter(importer);

	return nullptr;
}
