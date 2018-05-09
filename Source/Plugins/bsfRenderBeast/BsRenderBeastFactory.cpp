//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastFactory.h"
#include "Renderer/BsRenderer.h"
#include "BsRenderBeast.h"

namespace bs
{
	constexpr const char* RenderBeastFactory::SystemName;

	SPtr<ct::Renderer> RenderBeastFactory::create()
	{
		return bs_shared_ptr_new<ct::RenderBeast>();
	}

	const String& RenderBeastFactory::name() const
	{
		static String StrSystemName = SystemName;
		return StrSystemName;
	}
}
