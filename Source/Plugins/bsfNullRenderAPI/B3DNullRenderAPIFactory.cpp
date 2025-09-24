//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderAPIFactory.h"
#include "RenderAPI/B3DRenderAPI.h"
#include "B3DNullRenderAPI.h"

using namespace b3d;
using namespace b3d::render;

constexpr const char* NullRenderAPIFactory::SystemName;

void NullRenderAPIFactory::Create()
{
	RenderAPI::StartUp<NullRenderAPI>();
}

NullRenderAPIFactory::InitOnStart NullRenderAPIFactory::initOnStart;
