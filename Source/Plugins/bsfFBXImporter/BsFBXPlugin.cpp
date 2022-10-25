//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFBXPrerequisites.h"
#include "BsFBXImporter.h"
#include "Importer/BsImporter.h"

namespace bs
{
	/**	Returns a name of the plugin. */
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "FBXImporter";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
	{
		FBXImporter* importer = bs_new<FBXImporter>();
		Importer::Instance().RegisterAssetImporterInternal(importer);

		return nullptr;
	}
} // namespace bs
