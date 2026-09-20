//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalResourceManager.h"
#include "B3DMetalGpuDevice.h"

namespace b3d::render
{
	MetalResourceManager::MetalResourceManager(MetalGpuDevice& device)
		: GpuResourceManager(device)
	{}

	MetalGpuDevice& MetalResourceManager::GetDevice() const
	{
		return static_cast<MetalGpuDevice&>(mDevice);
	}
} // namespace b3d::render
