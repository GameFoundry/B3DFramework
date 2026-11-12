//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalBindlessHeaps.h"
#include "B3DMetalGpuBuffer.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalResourceManager.h"
#include "B3DMetalSamplerState.h"
#include "B3DMetalTexture.h"
#include "B3DMetalUtility.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuQueue.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Creates a buffer of @p capacity null entries in shared memory, written from the CPU. Returns null on failure. */
			TShared<GpuBuffer> CreateHeapBuffer(MetalGpuDevice& device, u32 capacity)
			{
				TShared<GpuBuffer> buffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u64), capacity, GpuBufferFlag::StoreOnCPUWithGPUAccess));
				if (buffer == nullptr)
					return nullptr;

				std::memset([static_cast<MetalGpuBuffer&>(*buffer).GetMetalBuffer() contents], 0, (size_t)capacity * sizeof(u64));
				return buffer;
			}
		}

		MetalBindlessHeap::MetalBindlessHeap(MetalResourceManager* owner, u32 capacity, const TShared<GpuBuffer>& heapBuffer)
			: TMetalResource<GpuBindlessHeap>(owner, capacity), mHeapBuffer(heapBuffer)
		{ }

		MetalBindlessHeaps::MetalBindlessHeaps(MetalGpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation)
			: GpuBindlessHeaps(device, completionTracker, createInformation), mDevice(device), mSamplerCapacity(createInformation.SamplerCapacity)
		{
			B3D_ASSERT(createInformation.ResourceCapacity == createInformation.ResourceMaximumCapacity);
		}

		MetalBindlessHeaps::~MetalBindlessHeaps()
		{
			Destroy();

			// Changes are applied on whichever thread ends the frame, which need not be the thread destroying the device
			if (mPatchCommandBufferPool != nullptr)
			{
				mPatchCommandBufferPool->SetThisAsOwnerThread();
				mPatchCommandBufferPool->Destroy();
				mPatchCommandBufferPool = nullptr;
			}

			mSamplerHeapBuffer = nullptr;
		}

		bool MetalBindlessHeaps::Initialize()
		{
			mSamplerHeapBuffer = CreateHeapBuffer(mDevice, mSamplerCapacity);
			if (mSamplerHeapBuffer == nullptr)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create the bindless sampler heap buffer of {0} entries.", mSamplerCapacity);
				return false;
			}

			return GpuBindlessHeaps::Initialize();
		}

		id<MTLBuffer> MetalBindlessHeaps::GetHeapBuffer(GpuBindlessHeapType heap) const
		{
			const TShared<GpuBuffer>& buffer = heap == GpuBindlessHeapType::Resource
				? static_cast<MetalBindlessHeap*>(GetResourceHeapPool().GetCurrent())->GetHeapBuffer()
				: mSamplerHeapBuffer;

			return static_cast<MetalGpuBuffer&>(*buffer).GetMetalBuffer();
		}

		u64 MetalBindlessHeaps::GetResourceEntry(const GpuBindlessResourceView& view) const
		{
			B3D_ASSERT(view.Type != GpuBindlessResourceType::None && "The heaps reject empty views.");

			switch (view.Type)
			{
			case GpuBindlessResourceType::Texture:
				return MetalUtility::GetArgumentValue(static_cast<MetalTexture*>(view.Texture.Object)->GetSubresourceView(view.Texture.Surface, false).gpuResourceID);
			case GpuBindlessResourceType::Buffer:
			{
				auto* buffer = static_cast<MetalGpuBuffer*>(view.Buffer.Object);
				if (buffer->GetInformation().Type == GpuBufferType::StructuredStorage)
					return buffer->GetMetalBuffer().gpuAddress;

				return MetalUtility::GetArgumentValue(buffer->GetTextureBufferView(view.Buffer.Format, 0, 0, false).gpuResourceID);
			}
			case GpuBindlessResourceType::AccelerationStructure:
			case GpuBindlessResourceType::None:
				break;
			}

			// No Metal acceleration structures yet. A zero entry reads as a null resource.
			B3D_LOG(Error, LogRenderBackend, "Cannot write the view of '{0}' into a bindless resource slot, Metal doesn't support it.", view.GetResourceName());
			return 0;
		}

		GpuBindlessHeap* MetalBindlessHeaps::CreateResourceHeap(u32 capacity)
		{
			const TShared<GpuBuffer> heapBuffer = CreateHeapBuffer(mDevice, capacity);
			if (heapBuffer == nullptr)
				return nullptr;

			return mDevice.GetResourceManager().Create<MetalBindlessHeap>(capacity, heapBuffer);
		}

		void MetalBindlessHeaps::CopyResourceSlots(GpuBindlessHeap& /*destination*/, TArrayView<const TIndexRange<u32>> /*ranges*/)
		{
			B3D_ASSERT(false && "The Metal resource heap never grows, so its native heap never switches.");
		}

		void MetalBindlessHeaps::WriteResourceSlots(GpuBindlessHeap& /*destination*/, TArrayView<const GpuBindlessSlotUpdate> /*updates*/)
		{
			B3D_ASSERT(false && "The Metal resource heap never grows, so its native heap never switches.");
		}

		void MetalBindlessHeaps::WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view)
		{
			((u64*)[GetHeapBuffer(GpuBindlessHeapType::Resource) contents])[slotIndex] = GetResourceEntry(view);
		}

		void MetalBindlessHeaps::ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType)
		{
			u64* entries = (u64*)[GetHeapBuffer(GpuBindlessHeapType::Resource) contents];

			// Reads through a null texture return zero. Structured and raw buffers are device pointers, so they point to the
			// zero-filled dummy buffer instead.
			const bool isPointer = viewType == GPOT_STRUCTURED_BUFFER || viewType == GPOT_BYTE_ADDRESS_BUFFER;
			entries[slotIndex] = isPointer ? mDevice.GetDummyArgumentBuffer().gpuAddress : 0;
		}

		void MetalBindlessHeaps::WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler)
		{
			const u64 entry = MetalUtility::GetArgumentValue(static_cast<const MetalSamplerState&>(sampler).GetMetalSampler().gpuResourceID);
			((u64*)[GetHeapBuffer(GpuBindlessHeapType::Sampler) contents])[slotIndex] = entry;
		}

		bool MetalBindlessHeaps::PatchResourceSlots(TArrayView<const GpuBindlessSlotUpdate> updates)
		{
			// The new entries go to a staging buffer in slot order, so runs of consecutive slots copy with one command
			TArray<GpuBindlessSlotUpdate> sortedUpdates(updates.begin(), updates.end());
			std::sort(sortedUpdates.begin(), sortedUpdates.end(), [](const GpuBindlessSlotUpdate& lhs, const GpuBindlessSlotUpdate& rhs) { return lhs.SlotIndex < rhs.SlotIndex; });

			const TShared<GpuBuffer> staging = mDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite((u32)(sortedUpdates.Size() * sizeof(u64))));
			if (staging == nullptr)
				return false;

			{
				const GpuBufferMappedScope mapping = staging->Map(GpuMapOption::Write);
				if (!mapping.IsValid())
					return false;

				u64* entries = static_cast<u64*>(mapping.GetMappedMemory());
				for (u32 updateIndex = 0; updateIndex < (u32)sortedUpdates.Size(); updateIndex++)
					entries[updateIndex] = GetResourceEntry(sortedUpdates[updateIndex].View);
			}

			TArray<GpuBufferCopyRange> copies;
			u32 runStart = 0;
			for (u32 updateIndex = 1; updateIndex <= (u32)sortedUpdates.Size(); updateIndex++)
			{
				const bool isRunEnd = updateIndex == (u32)sortedUpdates.Size() || sortedUpdates[updateIndex].SlotIndex != sortedUpdates[updateIndex - 1].SlotIndex + 1;
				if (!isRunEnd)
					continue;

				GpuBufferCopyRange copy;
				copy.SourceOffset = runStart * (u32)sizeof(u64);
				copy.DestinationOffset = sortedUpdates[runStart].SlotIndex * (u32)sizeof(u64);
				copy.Size = (updateIndex - runStart) * (u32)sizeof(u64);
				copies.Add(copy);

				runStart = updateIndex;
			}

			if (mPatchCommandBufferPool == nullptr)
				mPatchCommandBufferPool = mDevice.CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
			else if (mPatchCommandBufferPool->GetOwnerThread() != B3D_CURRENT_THREAD_ID)
				mPatchCommandBufferPool->SetThisAsOwnerThread();

			// Shaders read the heap without barriers. Earlier reads must complete before the copies, and later reads must see them.
			const TShared<GpuBuffer>& heap = static_cast<MetalBindlessHeap*>(GetResourceHeapPool().GetCurrent())->GetHeapBuffer();
			const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*heap, GQT_GRAPHICS);
			const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

			const TShared<GpuCommandBuffer> commandBuffer = mPatchCommandBufferPool->FindOrCreate(GpuCommandBufferCreateInformation::Create("Bindless heap patch"));
			commandBuffer->IssueBarriers(GpuBufferBarrier(heap, restingState, transferWrite));
			commandBuffer->CopyBufferToBuffer(staging, heap, copies);
			commandBuffer->IssueBarriers(GpuBufferBarrier(heap, transferWrite, restingState));

			// The patch must not overlap reads on any queue, so it waits for all earlier work. It is submitted before the frame boundary,
			// so the frame fence makes the next frame's work on every queue wait for it.
			GpuSubmissionInformation submissionInformation;
			submissionInformation.CommandBuffer = commandBuffer;
			submissionInformation.SyncMask = GpuQueueMask::kAll;

			mDevice.GetQueue(GQT_GRAPHICS, 0)->SubmitCommandBuffer(submissionInformation);

			return true;
		}
	} // namespace render
} // namespace b3d
