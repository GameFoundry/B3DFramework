//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastFactory.h"
#include "Renderer/BsRenderer.h"
#include "BsRenderBeast.h"

using namespace bs;

constexpr const char* RenderBeastFactory::kSystemName;

SPtr<ct::Renderer> RenderBeastFactory::Create()
{
	return B3DMakeShared<ct::RenderBeast>();
}

const String& RenderBeastFactory::Name() const
{
	static String StrSystemName = kSystemName;
	return StrSystemName;
}
