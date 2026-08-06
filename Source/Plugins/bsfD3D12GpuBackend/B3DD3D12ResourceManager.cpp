//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12GpuDevice.h"

namespace b3d::render
{
	D3D12ResourceManager::D3D12ResourceManager(D3D12GpuDevice& device)
		: GpuResourceManager(device)
	{}

	D3D12GpuDevice& D3D12ResourceManager::GetDevice() const
	{
		return static_cast<D3D12GpuDevice&>(mDevice);
	}
} // namespace b3d::render
