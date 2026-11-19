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

				const bool ok = allocation.Allocator->TryAllocate(requirements.Size, (u32)requirements.Alignment, mAllocation);
				B3D_ASSERT(ok && "Allocator failed to satisfy the allocation request.");
				(void)ok;
			}
			else
				mAllocation = allocation;

			// Allocate a dummy buffer for persistently mapped memory
			mMappedMemory = B3DAllocate(mTotalSize);
			mBuffer = CreateBufferResource();
		}

		NullGpuBuffer::~NullGpuBuffer()
		{
			mBuffer->Destroy();

			if(mAllocation.IsOwned())
				mAllocation.Allocator->Free(mAllocation);

			if (mMappedMemory)
			{
				B3DFree(mMappedMemory);
				mMappedMemory = nullptr;
			}
		}

		void NullGpuBuffer::RecreateInternalBuffer()
		{
			// The CPU memory stays, since nothing reads the old contents on the null device
			NullBufferResource* const newBuffer = CreateBufferResource();
			mBuffer->Destroy();
			mBuffer = newBuffer;
		}

		void NullGpuBuffer::SwapInternalBuffer(GpuBuffer& other)
		{
			NullGpuBuffer& otherBuffer = static_cast<NullGpuBuffer&>(other);
			std::swap(mAllocation, otherBuffer.mAllocation);
			std::swap(mBuffer, otherBuffer.mBuffer);
		}

		NullBufferResource* NullGpuBuffer::CreateBufferResource()
		{
			return static_cast<NullGpuDevice&>(GetDevice()).GetResourceManager().Create<NullBufferResource>(GetName());
		}
	} // namespace render
} // namespace b3d
