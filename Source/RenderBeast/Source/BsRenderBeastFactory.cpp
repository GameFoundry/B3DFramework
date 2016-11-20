//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRenderBeastFactory.h"
#include "BsRenderer.h"
#include "BsRenderBeast.h"

namespace bs
{
	const char* SystemName = "RenderBeast";

	SPtr<CoreRenderer> RenderBeastFactory::create()
	{
		return bs_shared_ptr_new<RenderBeast>();
	}

	const String& RenderBeastFactory::name() const
	{
		static String StrSystemName = SystemName;
		return StrSystemName;
	}
}