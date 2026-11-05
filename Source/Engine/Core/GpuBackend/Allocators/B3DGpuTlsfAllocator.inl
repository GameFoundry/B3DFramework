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
	bool TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::TryAllocateImpl(u64 size, u32 alignment, GpuAllocation& out)
	{
		B3D_ASSERT(out.Allocator == nullptr);

		InternalAllocation allocation;
		if (!mInternal.TryAllocate(size, alignment, allocation))
			return false;

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
	void TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::SetAllocationOwner(const GpuAllocation& allocation, GpuAllocationOwner owner)
	{
		typename Base::ScopedLock lock(this->GetMutex());
		B3D_ASSERT(allocation.Allocator == this);

		InternalAllocation internalAllocation;
		internalAllocation.HeapIndex = allocation.AllocatorData0;
		internalAllocation.NodeIndex = allocation.AllocatorData1;
		mInternal.SetAllocationOwner(internalAllocation, owner.ToOpaque());
	}

	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy>
	void TGpuTlsfAllocator<HeapBackend, ThreadPolicy>::PlanDefragmentation(const GpuDefragmentationBudget& budget, const Function<bool(GpuAllocationOwner)>& fnIsMovable,
		TArray<GpuAllocationMove>& outMoves)
	{
		typename Base::ScopedLock lock(this->GetMutex());

		u64 byteCount = 0;
		u32 moveCount = 0;
		bool isBudgetExhausted = false;

		// Destinations are flagged for the pass so later iterations neither move them nor count them as candidates
		Vector<DefragDestinationKey> destinationNodes;

		// Newer heaps (higher index, typically sparser) drain first, into the same heap or any lower-index one. Within a heap,
		// allocations at high offsets move first into low offsets vacated earlier.
		for (i32 heapIndexSigned = (i32)mInternal.GetHeapSlotCount() - 1; heapIndexSigned >= 0 && !isBudgetExhausted; heapIndexSigned--)
		{
			const u32 heapIndex = (u32)heapIndexSigned;
			if (mInternal.GetHeapSlot(heapIndex) == nullptr)
				continue;

			if (mInternal.GetHeapSlot(heapIndex)->NullNodeIndex() == detail::tlsf::Utility::kInvalidIndex)
				continue;

			u32 nodeIndex = mInternal.GetHeapSlot(heapIndex)->GetNode(mInternal.GetHeapSlot(heapIndex)->NullNodeIndex()).PrevPhysical;
			while (nodeIndex != detail::tlsf::Utility::kInvalidIndex)
			{
				// Reserving a destination in the same heap may grow its node pool, so nodes are copied rather than referenced
				const detail::tlsf::Node node = mInternal.GetHeapSlot(heapIndex)->GetNode(nodeIndex);
				const u32 previousNodeIndex = node.PrevPhysical;

				const GpuAllocationOwner owner = GpuAllocationOwner::FromOpaque(node.Owner);
				if (node.IsFree() || node.IsNullNode() || node.IsDefragDestination() || owner.IsEmpty() || !fnIsMovable(owner))
				{
					nodeIndex = previousNodeIndex;
					continue;
				}

				if (budget.MaximumMoveCount != 0 && moveCount >= budget.MaximumMoveCount)
				{
					isBudgetExhausted = true;
					break;
				}

				// Allocations larger than what is left of the byte budget are skipped, so they don't block smaller ones
				if (budget.MaximumByteCount != 0 && byteCount + node.Size > budget.MaximumByteCount)
				{
					nodeIndex = previousNodeIndex;
					continue;
				}

				InternalAllocation destination;
				if (!mInternal.TryAllocateInHeapsAtMost(node.Size, node.Alignment, heapIndex, destination))
				{
					nodeIndex = previousNodeIndex;
					continue;
				}

				// A move within the heap only helps if it lowers the offset
				if (destination.HeapIndex == heapIndex && destination.Offset >= node.Offset)
				{
					FreeAndReclaimImpl(destination.HeapIndex, destination.NodeIndex);
					nodeIndex = previousNodeIndex;
					continue;
				}

				mInternal.GetHeapSlot(destination.HeapIndex)->SetDefragDestinationFlag(destination.NodeIndex);
				destinationNodes.push_back({ destination.HeapIndex, destination.NodeIndex });

				GpuAllocationMove move;
				move.Owner = owner;

				move.Source.Heap = mInternal.GetHeapSlot(heapIndex)->Handle();
				move.Source.Offset = node.Offset;
				move.Source.Size = node.Size;
				move.Source.Allocator = this;
				move.Source.AllocatorData0 = heapIndex;
				move.Source.AllocatorData1 = nodeIndex;

				move.Destination.Heap = mInternal.GetHeapSlot(destination.HeapIndex)->Handle();
				move.Destination.Offset = destination.Offset;
				move.Destination.Size = node.Size;
				move.Destination.Allocator = this;
				move.Destination.AllocatorData0 = destination.HeapIndex;
				move.Destination.AllocatorData1 = destination.NodeIndex;
				outMoves.Add(move);

#if B3D_DEBUG
				this->mDebugOutstandingAllocations++;
#endif

				byteCount += node.Size;
				moveCount++;
				nodeIndex = previousNodeIndex;
			}
		}

		for (const DefragDestinationKey& key : destinationNodes)
			mInternal.GetHeapSlot(key.HeapIndex)->ClearDefragDestinationFlag(key.NodeIndex);
	}
} // namespace b3d
