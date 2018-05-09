//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLPrerequisites.h"
#include "BsGLRenderAPIFactory.h"

namespace bs
{
	extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
	{
		return ct::GLRenderAPIFactory::SystemName;
	}
}
