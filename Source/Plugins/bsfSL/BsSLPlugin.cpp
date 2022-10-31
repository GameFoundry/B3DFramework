//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSLPrerequisites.h"
#include "BsSLImporter.h"
#include "Importer/BsImporter.h"

using namespace bs;


/**	Returns a name of the plugin. */
extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
{
	static constexpr const char* kSystemName = "bsfSL";
	return kSystemName;
}

/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
{
	SLImporter* importer = B3DNew<SLImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	return nullptr;
}
