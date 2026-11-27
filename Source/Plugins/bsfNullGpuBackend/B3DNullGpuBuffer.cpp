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

			// The native resource owns the persistently mapped memory, so the memory stays while the native resource is pinned or bound. A
			// virtual buffer has no memory to map.
			mBuffer = CreateBufferResource();
			mMappedMemory = mBuffer->GetMemory();
		}

		NullGpuBuffer::~NullGpuBuffer()
		{
			mBuffer->Destroy();

			if(mAllocation.IsOwned())
				mAllocation.Allocator->Free(mAllocation);

			mMappedMemory = nullptr;
		}

		void NullGpuBuffer::RecreateInternalBuffer()
		{
			NullBufferResource* const newBuffer = CreateBufferResource();
			mBuffer->Destroy();
			mBuffer = newBuffer;
			mMappedMemory = mBuffer->GetMemory();
		}

		void NullGpuBuffer::SwapInternalBuffer(GpuBuffer& other)
		{
			// The CPU memory moves with the native resource that owns it
			NullGpuBuffer& otherBuffer = static_cast<NullGpuBuffer&>(other);
			std::swap(mAllocation, otherBuffer.mAllocation);
			std::swap(mBuffer, otherBuffer.mBuffer);
			std::swap(mMappedMemory, otherBuffer.mMappedMemory);
		}

		NullBufferResource* NullGpuBuffer::CreateBufferResource()
		{
			const u32 memorySize = mInformation.Flags.IsSet(GpuBufferFlag::Virtual) ? 0 : mTotalSize;
			return static_cast<NullGpuDevice&>(GetDevice()).GetResourceManager().Create<NullBufferResource>(GetName(), memorySize);
		}
	} // namespace render
} // namespace b3d
