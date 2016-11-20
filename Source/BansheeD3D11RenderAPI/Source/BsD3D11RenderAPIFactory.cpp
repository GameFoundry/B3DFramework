//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11RenderAPIFactory.h"
#include "BsRenderAPI.h"

namespace bs
{
	const char* SystemName = "BansheeD3D11RenderSystem";

	void D3D11RenderAPIFactory::create()
	{
		RenderAPICore::startUp<D3D11RenderAPI>();
	}

	D3D11RenderAPIFactory::InitOnStart D3D11RenderAPIFactory::initOnStart;
}