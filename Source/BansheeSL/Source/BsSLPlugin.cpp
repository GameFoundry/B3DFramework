//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSLPrerequisites.h"
#include "BsSLImporter.h"
#include "BsImporter.h"

namespace bs
{
	const char* SystemName = "BansheeSL";

	/**	Returns a name of the plugin. */
	extern "C" BS_SL_EXPORT const char* getPluginName()
	{
		return SystemName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_SL_EXPORT void* loadPlugin()
	{
		SLImporter* importer = bs_new<SLImporter>();
		Importer::instance()._registerAssetImporter(importer);

		return nullptr;
	}
}