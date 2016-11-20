//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11Prerequisites.h"
#include "BsD3D11RenderAPIFactory.h"

namespace bs
{
	extern "C" BS_D3D11_EXPORT const char* getPluginName()
	{
		return SystemName;
	}
}