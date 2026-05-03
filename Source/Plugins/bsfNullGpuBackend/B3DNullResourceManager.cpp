//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullResourceManager.h"
#include "B3DNullGpuDevice.h"

namespace b3d::render
{
	NullResourceManager::NullResourceManager(NullGpuDevice& device)
		: GpuResourceManager(device)
	{}

	NullGpuDevice& NullResourceManager::GetDevice() const
	{
		return static_cast<NullGpuDevice&>(mDevice);
	}
} // namespace b3d::render
