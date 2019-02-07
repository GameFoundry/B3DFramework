//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullPrerequisites.h"
#include "BsNullRenderAPIFactory.h"

namespace bs
{
    extern "C" BS_PLUGIN_EXPORT const char* getPluginName()
    {
        return ct::NullRenderAPIFactory::SystemName;
    }
}
