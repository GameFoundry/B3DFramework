//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuPushConstants.h"

using namespace b3d;

void GpuPushConstantPayload::Clear()
{
	Values.fill(0);
}

void GpuPushConstantPayload::Write(u32 offsetInBytes, u32 sizeInBytes, const void* data)
{
	if (sizeInBytes == 0)
		return;

	B3D_ASSERT(data != nullptr);
	B3D_ASSERT(offsetInBytes <= kMaxPushConstantSizeInBytes);
	B3D_ASSERT(sizeInBytes <= kMaxPushConstantSizeInBytes - offsetInBytes);

	std::memcpy(reinterpret_cast<u8*>(Values.data()) + offsetInBytes, data, sizeInBytes);
}
