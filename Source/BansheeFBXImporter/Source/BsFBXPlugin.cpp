//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFBXPrerequisites.h"
#include "BsFBXImporter.h"
#include "BsImporter.h"

namespace bs
{
	/**	Returns a name of the plugin. */
	extern "C" BS_FBX_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "FBXImporter";
		return pluginName;
	}

	/**	Entry point to the plugin. Called by the engine when the plugin is loaded. */
	extern "C" BS_FBX_EXPORT void* loadPlugin()
	{
		FBXImporter* importer = bs_new<FBXImporter>();
		Importer::instance()._registerAssetImporter(importer);

		return nullptr;
	}
}