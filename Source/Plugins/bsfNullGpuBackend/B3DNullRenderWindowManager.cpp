//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullRenderWindowManager.h"
#include "B3DNullRenderWindowSurface.h"

namespace b3d
{
	TShared<render::IRenderWindowSurface> NullRenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
	{
		auto surface = B3DMakeShared<render::NullRenderWindowSurface>(createInformation);
		return surface;
	}
} // namespace b3d
