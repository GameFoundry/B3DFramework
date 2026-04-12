//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFreeImgPrerequisites.h"
#include "B3DFreeImgImporter.h"

using namespace b3d;

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	static const char* pluginName = "FreeImageImporter";
	return pluginName;
}

extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	FreeImgImporter* importer = B3DNew<FreeImgImporter>();
	Importer::Instance().RegisterAssetImporter(importer);

	return nullptr;
}
