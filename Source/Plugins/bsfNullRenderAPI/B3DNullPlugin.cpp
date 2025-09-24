//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullPrerequisites.h"
#include "B3DNullRenderAPIFactory.h"

using namespace b3d;

extern "C" B3D_PLUGIN_EXPORT const char* GetPluginName()
{
	return render::NullRenderAPIFactory::SystemName;
}
