//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderAPIFactory.h"
#include "RenderAPI/BsRenderAPI.h"
#include "BsNullRenderAPI.h"

namespace bs { namespace ct
{
	constexpr const char* NullRenderAPIFactory::SystemName;

	void NullRenderAPIFactory::create()
	{
		RenderAPI::startUp<NullRenderAPI>();
	}

	NullRenderAPIFactory::InitOnStart NullRenderAPIFactory::initOnStart;
}}
