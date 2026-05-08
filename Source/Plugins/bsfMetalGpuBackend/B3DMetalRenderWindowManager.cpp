//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalRenderWindowManager.h"
#include "B3DMetalRenderWindowSurface.h"
#include "B3DMetalGpuBackend.h"
#include "B3DMetalGpuDevice.h"

namespace b3d
{
	TShared<render::IRenderWindowSurface> MetalRenderWindowManager::CreateRenderWindowSurface(const render::RenderWindowSurfaceCreateInformation& createInformation)
	{
		auto device = std::static_pointer_cast<render::MetalGpuDevice>(GetMetalGpuBackend().GetDevice(0));
		return B3DMakeShared<render::MetalRenderWindowSurface>(*device, createInformation);
	}
} // namespace b3d
