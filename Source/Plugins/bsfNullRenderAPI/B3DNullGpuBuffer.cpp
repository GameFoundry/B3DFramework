//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuBuffer.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuBuffer::NullGpuBuffer(NullGpuDevice& device, const GpuBufferCreateInformation& createInformation)
			: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device))
		{
			// Allocate a dummy buffer for persistently mapped memory
			mMappedMemory = B3DAllocate(mTotalSize);
		}

		NullGpuBuffer::~NullGpuBuffer()
		{
			if (mMappedMemory)
			{
				B3DFree(mMappedMemory);
				mMappedMemory = nullptr;
			}
		}
	} // namespace render
} // namespace b3d
