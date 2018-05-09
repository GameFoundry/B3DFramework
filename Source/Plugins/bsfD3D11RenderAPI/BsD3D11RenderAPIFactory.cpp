//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11RenderAPIFactory.h"
#include "RenderAPI/BsRenderAPI.h"

namespace bs { namespace ct
{
	constexpr const char* D3D11RenderAPIFactory::SystemName;

	void D3D11RenderAPIFactory::create()
	{
		RenderAPI::startUp<D3D11RenderAPI>();
	}

	D3D11RenderAPIFactory::InitOnStart D3D11RenderAPIFactory::initOnStart;
}}
