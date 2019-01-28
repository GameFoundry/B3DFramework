//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsMonoPrerequisites.h"

namespace bs
{
	extern "C" BS_MONO_EXPORT const char* getPluginName()
	{
		static const char* pluginName = "bsfMono";
		return pluginName;
	}
}