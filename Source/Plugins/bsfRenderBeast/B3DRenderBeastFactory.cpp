//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderBeastFactory.h"
#include "Renderer/B3DRenderer.h"
#include "B3DRenderBeast.h"

using namespace b3d;

constexpr const char* RenderBeastFactory::kSystemName;

TShared<render::Renderer> RenderBeastFactory::Create()
{
	return B3DMakeShared<render::RenderBeast>();
}

const String& RenderBeastFactory::Name() const
{
	static String StrSystemName = kSystemName;
	return StrSystemName;
}
