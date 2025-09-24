//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderAPIFactory.h"
#include "RenderAPI/BsRenderAPI.h"
#include "BsNullRenderAPI.h"

using namespace b3d;
using namespace b3d::render;

constexpr const char* NullRenderAPIFactory::SystemName;

void NullRenderAPIFactory::Create()
{
	RenderAPI::StartUp<NullRenderAPI>();
}

NullRenderAPIFactory::InitOnStart NullRenderAPIFactory::initOnStart;
