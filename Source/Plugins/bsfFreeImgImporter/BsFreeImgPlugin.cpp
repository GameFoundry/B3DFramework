//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFreeImgPrerequisites.h"
#include "BsFreeImgImporter.h"

namespace bs
{
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "FreeImageImporter";
		return pluginName;
	}

	extern "C" BS_PLUGIN_EXPORT void* loadPlugin()
	{
		FreeImgImporter* importer = bs_new<FreeImgImporter>();
		Importer::instance().RegisterAssetImporterInternal(importer);

		return nullptr;
	}
}
