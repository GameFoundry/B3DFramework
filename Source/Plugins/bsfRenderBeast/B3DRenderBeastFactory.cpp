//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderBeastFactory.h"
#include "Renderer/BsRenderer.h"
#include "BsRenderBeast.h"

using namespace b3d;

constexpr const char* RenderBeastFactory::kSystemName;

SPtr<render::Renderer> RenderBeastFactory::Create()
{
	return B3DMakeShared<render::RenderBeast>();
}

const String& RenderBeastFactory::Name() const
{
	static String StrSystemName = kSystemName;
	return StrSystemName;
}
