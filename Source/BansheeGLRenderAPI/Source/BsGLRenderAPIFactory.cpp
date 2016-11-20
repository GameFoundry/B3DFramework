//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLRenderAPIFactory.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	void GLRenderAPIFactory::create()
	{
		RenderAPICore::startUp<GLRenderAPI>();
	}

	GLRenderAPIFactory::InitOnStart GLRenderAPIFactory::initOnStart;
}