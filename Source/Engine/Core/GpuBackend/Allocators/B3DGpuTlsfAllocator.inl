//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

// Template method definitions for TGpuTlsfAllocator. Not a translation unit of its own — included at the end of
// B3DGpuTlsfAllocator.h.

#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"

namespace b3d
{
	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::TGpuTlsfAllocator(HeapBackend* backend, IGpuCompletionTracker* completionTracker, const Configuration& configuration)
		: Base(backend, completionTracker), mInternal(backend, configuration), mDeferralMode(configuration.DeferralMode)
	{
		B3D_ASSERT((mDeferralMode != GpuAllocatorFreeDeferralMode::FrameTracker) || completionTracker != nullptr);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::~TGpuTlsfAllocator()
	{
		// Drain unconditionally — any submissions still in flight at destructor time are the caller's
		// responsibility to wait for via WaitUntilIdle, matching the convention from TGpuAllocator.
		// Runs while mInternal is still alive; mInternal's own destructor then tears down the heaps.
		Base::ReclaimUnused(true);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuAllocation& out)
	{
		B3D_ASSERT(out.Allocator == nullptr);

		InternalAllocation allocation;
		if (!mInternal.TryAllocate(size, alignment, allocation, ToTlsfKind(kind)))
			return false;

		mInternal.SetAllocationOwner(allocation, owner);

		out.Heap = allocation.Heap;
		out.Offset = allocation.Offset;
		out.Size = allocation.Size;
		out.Allocator = this;
		out.AllocatorData0 = allocation.HeapIndex;
		out.AllocatorData1 = allocation.NodeIndex;
		return true;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::FreeImpl(GpuAllocation& allocation)
	{
		B3D_ASSERT(allocation.Allocator == this);

		if (mDeferralMode == GpuAllocatorFreeDeferralMode::ResourceLifecycle)
		{
			// Caller has gated GPU completion through IGpuResource::Destroy + Notify* — no need to
			// queue the slot. Release synchronously so a subsequent allocation can reuse it.
			FreeAndReclaimImpl(allocation.AllocatorData0, allocation.AllocatorData1);
			return;
		}

		Base::RetireAllocation(allocation);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::FreeAndReclaimImpl(u32 heapIndex, u32 nodeIndex)
	{
		InternalAllocation allocation;
		allocation.HeapIndex = heapIndex;
		allocation.NodeIndex = nodeIndex;
		mInternal.Free(allocation);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::SetAllocationOwner(const GpuAllocation& allocation, IGpuResource* owner)
	{
		typename Base::ScopedLock lock(this->GetMutex());
		B3D_ASSERT(allocation.Allocator == this);

		InternalAllocation internalAllocation;
		internalAllocation.HeapIndex = allocation.AllocatorData0;
		internalAllocation.NodeIndex = allocation.AllocatorData1;
		mInternal.SetAllocationOwner(internalAllocation, owner);
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	bool TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u32& outDestinationHeapIndex, u32& outDestinationNodeIndex)
	{
		Heap* sourceHeap = mInternal.GetHeapSlot(sourceHeapIndex);

		// Snapshot source state before TryAllocateInHeapsAtMost — within-heap CarveAllocation may
		// push_back onto the source heap's pool, which would invalidate any held Node& references.
		const detail::tlsf::Node& sourceSnapshotRef = sourceHeap->GetNode(sourceNodeIndex);
		const u64 sourceOffset = sourceSnapshotRef.Offset;
		const u64 sourceSize = sourceSnapshotRef.Size;
		IGpuResource* owner = static_cast<IGpuResource*>(sourceSnapshotRef.Owner);
		const TlsfAllocationKind sourceKind = sourceSnapshotRef.Flags.IsSet(detail::tlsf::NodeFlag::NonLinear) ? TlsfAllocationKind::NonLinear : TlsfAllocationKind::Linear;

		// 1. Reserve a destination slot in the same heap or any lower-index heap
		InternalAllocation destination;
		if (!mInternal.TryAllocateInHeapsAtMost(sourceSize, /*alignment=*/1u, sourceHeapIndex, destination, sourceKind))
			return false;

		// 2. Ensure destination is at a lower offset than the source
		if (destination.HeapIndex == sourceHeapIndex && destination.Offset >= sourceOffset)
		{
			FreeAndReclaimImpl(destination.HeapIndex, destination.NodeIndex);
			return false;
		}

		Heap* destHeap = mInternal.GetHeapSlot(destination.HeapIndex);

		// 3. Build the destination GpuAllocation
		GpuAllocation newAllocation;
		newAllocation.Heap = destHeap->Handle();
		newAllocation.Offset = destination.Offset;
		newAllocation.Size = sourceSize;
		newAllocation.Allocator = this;
		newAllocation.AllocatorData0 = destination.HeapIndex;
		newAllocation.AllocatorData1 = destination.NodeIndex;

		// 4. Notify the owner: depending on DeferralMode it will either re-allocate a brand new IGpuResource
		//    at the destination allocation (if resource tracking is used), or patch the existing
		//    resource (if frame tracking is used).
		IGpuResource* newOwner = owner->MoveAllocation(commandBuffer, newAllocation);

		// 5. Mark the destination with the new owner
		destHeap->SetNodeOwner(destination.NodeIndex, newOwner);

		// 6. Dispose of the original memory. This depends on deferral mode:
		//    - ResourceLifecycle - Consumer is tasked with disposing the memory. He should call
		//      Free() when the old IGpuResource is done being used on the GPU.
		//    - FrameTracker - The allocator is tasked with disposing the memory. The allocator
		//      waits for kMaximumFramesInFlight and then releases the memory.
		//
		//    In both cases, clear the source node's Owner so any defrag pass issued before the
		//    source is freed observes the slot as untracked rather than as a phantom candidate.
		if (mDeferralMode == GpuAllocatorFreeDeferralMode::FrameTracker)
		{
			B3D_ASSERT(newOwner == owner &&
				"FreeDeferralMode::FrameTracker requires MoveAllocation to return the same IGpuResource it was called on. "
				"Wrapper-swap patterns require FreeDeferralMode::ResourceLifecycle.");

			GpuAllocation sourceSnapshot;
			sourceSnapshot.Allocator = this;
			sourceSnapshot.AllocatorData0 = sourceHeapIndex;
			sourceSnapshot.AllocatorData1 = sourceNodeIndex;

			Base::RetireAllocation(sourceSnapshot);
		}

		sourceHeap->SetNodeOwner(sourceNodeIndex, nullptr);

		outDestinationHeapIndex = destination.HeapIndex;
		outDestinationNodeIndex = destination.NodeIndex;
		return true;
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	typename TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::DefragmentationStats
	TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info)
	{
		typename Base::ScopedLock lock(this->GetMutex());
		DefragmentationStats stats{};

		// Tracks destination (heap, node) pairs stamped with NodeFlag::DefragDestination so we can
		// clear the flag at end of pass. Bounded by stats.MovesCompleted ≤ info.MaxAllocationsPerCall.
		Vector<DefragDestinationKey> destinationNodes;
		if (info.MaxAllocationsPerCall != 0)
			destinationNodes.reserve(info.MaxAllocationsPerCall);

		// Walk heaps high-index → low-index — newer (typically sparser) heaps drain first.
		// Destinations land in the same heap (within-heap compaction) or any lower-index heap
		// (multi-heap drain); both placements rely on NodeFlag::DefragDestination to keep the
		// destination invisible to subsequent iteration in the same pass.
		for (i32 outerIndex = (i32)mInternal.GetHeapSlotCount() - 1; outerIndex >= 0; outerIndex--)
		{
			const u32 heapIndex = (u32)outerIndex;
			if (mInternal.GetHeapSlot(heapIndex) == nullptr)
				continue;

			Heap& heap = *mInternal.GetHeapSlot(heapIndex);
			if (heap.NullNodeIndex() == detail::tlsf::Utility::kInvalidIndex)
				continue;

			// Walk the physical chain backwards (highest offset → lowest) starting just before
			// the trailing null block. Compaction is more productive draining high-offset
			// allocations into freshly-vacated low-offset slots.
			u32 nodeIndex = heap.GetNode(heap.NullNodeIndex()).PrevPhysical;
			while (nodeIndex != detail::tlsf::Utility::kInvalidIndex)
			{
				// Capture the chain link before any state change — TryMoveAllocation may modify
				// the source node's NextPhysical/PrevPhysical pointers via CarveAllocation when
				// the destination lands in the same heap.
				const u32 prevIndex = heap.GetNode(nodeIndex).PrevPhysical;

				const detail::tlsf::Node& node = heap.GetNode(nodeIndex);
				if (node.IsFree() || node.IsNullNode() || node.IsDefragDestination())
				{
					nodeIndex = prevIndex;
					continue;
				}

				// Untracked slots (owner is null) cannot be relocated - there is no consumer to invoke MoveAllocation on.
				IGpuResource* owner = static_cast<IGpuResource*>(node.Owner);
				if (owner == nullptr)
				{
					nodeIndex = prevIndex;
					continue;
				}

				const u64 sourceSize = node.Size;

				if (info.MaxBytesPerCall != 0 && stats.BytesMoved + sourceSize > info.MaxBytesPerCall)
				{
					stats.BudgetExhausted = true;
					break;
				}
				if (info.MaxAllocationsPerCall != 0 && stats.MovesAttempted >= info.MaxAllocationsPerCall)
				{
					stats.BudgetExhausted = true;
					break;
				}

				stats.MovesAttempted++;
				u32 destinationHeapIndex = detail::tlsf::Utility::kInvalidIndex;
				u32 destinationNodeIndex = detail::tlsf::Utility::kInvalidIndex;
				if (TryMoveAllocation(nodeIndex, heapIndex, commandBuffer, destinationHeapIndex, destinationNodeIndex))
				{
					stats.MovesCompleted++;
					stats.BytesMoved += sourceSize;
					mInternal.GetHeapSlot(destinationHeapIndex)->SetDefragDestinationFlag(destinationNodeIndex);
					destinationNodes.push_back({ destinationHeapIndex, destinationNodeIndex });
				}

				nodeIndex = prevIndex;
			}

			if (stats.BudgetExhausted)
				break;
		}

		// Clear destination markers — destinations are now ordinary live allocations and become
		// valid candidates for future Defrag() calls. The marker is in effect only inside this
		// single Defrag() invocation.
		for (const DefragDestinationKey& key : destinationNodes)
			mInternal.GetHeapSlot(key.HeapIndex)->ClearDefragDestinationFlag(key.NodeIndex);

		return stats;
	}
} // namespace b3d
