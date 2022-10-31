//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFontPrerequisites.h"
#include "Importer/BsImporter.h"
#include "BsFontImporter.h"

using namespace bs;

/**	Returns a name of the plugin. */
extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
{
	static const char* pluginName = "FontImporter";
	return pluginName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
{
	FontImporter* importer = B3DNew<FontImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	return nullptr;
}
