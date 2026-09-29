//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuBuffer.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuBuffer::NullGpuBuffer(NullGpuDevice& device, const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation)
			: GpuBuffer(device, createInformation, b3d::GpuBuffer::CalculateSuballocatedBufferSize(createInformation, device), allocation)
		{
			if(allocation.IsPending())
			{
				const GpuMemoryRequirements requirements = device.GetMemoryRequirements(createInformation);

				const bool ok = allocation.Allocator->TryAllocate(requirements.Size, (u32)requirements.Alignment, requirements.Kind, nullptr, mAllocation);
				B3D_ASSERT(ok && "Allocator failed to satisfy the allocation request.");
				(void)ok;
			}
			else
				mAllocation = allocation;

			// Allocate a dummy buffer for persistently mapped memory
			mMappedMemory = B3DAllocate(mTotalSize);
		}

		NullGpuBuffer::~NullGpuBuffer()
		{
			if(mAllocation.IsOwned())
				mAllocation.Allocator->Free(mAllocation);

			if (mMappedMemory)
			{
				B3DFree(mMappedMemory);
				mMappedMemory = nullptr;
			}
		}
	} // namespace render
} // namespace b3d
