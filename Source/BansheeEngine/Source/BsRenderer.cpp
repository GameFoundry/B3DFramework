//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderer.h"
#include "BsRendererManager.h"

namespace bs
{
	SPtr<Renderer> gRenderer()
	{
		return std::static_pointer_cast<Renderer>(RendererManager::instance().getActive());
	}
}