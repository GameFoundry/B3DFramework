//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "GpuBackend/B3DGpuSubmissionTimeline.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "Utility/B3DConfigVariable.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** If disabled, no two transient resources of one scope share memory. Scopes still share memory with each other. */
	extern B3D_EXPORT TConfigVariable<bool> gGpuTransientAliasing;

	/** If disabled, memory last used in the scope on another queue than the queue of the new resource's first use is not reused. */
	extern B3D_EXPORT TConfigVariable<bool> gGpuTransientAliasAcrossQueues;

	/** Accesses of a resource on one queue that no later barrier on that queue orders. */
	struct GpuTransientLastUse
	{
		GpuTransientLastUse() = default;
		GpuTransientLastUse(u32 submission, const render::GpuAccessScope& access)
			: Submission(submission), Access(access)
		{ }

		/** Index of the last submission that used the resource on its queue, in the timeline of the scope that releases the resource. */
		u32 Submission = 0;

		/** Stages of the last write on the queue, and of every read on the queue since that write, over all subresources. */
		render::GpuAccessScope Access;
	};

	/** Memory use of one scope. */
	struct GpuTransientScopeStatistics
	{
		u64 PeakUsed = 0; /**< Largest total size of live allocations during the scope, in bytes. */

		/** Total size of the allocations made during the scope, in bytes. The difference to PeakUsed is the memory aliasing saved. */
		u64 RequestedBytes = 0;
	};

	/**
	 * Places the memory of transient resources on the heaps of transient heap pools, within a scope. Memory released in the
	 * scope is reused once the GPU work using the earlier resources is ordered before the first use of the new resource.
	 * Returns the earlier accesses that the first use of the new resource must still be ordered after, as a GpuAliasAcquire.
	 *
	 * Each scope describes its GPU work with a GpuSubmissionTimeline, and memory released in the scope is tagged with the
	 * submissions that last used it on each queue. Every heap starts free in every scope, and tags never outlive their scope:
	 * the system that schedules scopes guarantees that no two scopes execute on the GPU at the same time, and that the work of
	 * a scope is ordered after the work of every earlier scope.
	 *
	 * The allocator places memory only and creates no resources. Allocations are non-owning.
	 *
	 * @note	Not thread safe. Allocators of different threads may place memory on the same pools at the same time.
	 */
	class B3D_EXPORT GpuAliasingAllocator
	{
	public:
		GpuAliasingAllocator() = default;
		~GpuAliasingAllocator();

		GpuAliasingAllocator(const GpuAliasingAllocator&) = delete;
		GpuAliasingAllocator& operator=(const GpuAliasingAllocator&) = delete;

		/**
		 * Starts a scope, in which memory is allocated and released. @p timeline describes the GPU work of the scope, and must
		 * remain valid and unchanged until EndScope(). Only one scope may be open at a time.
		 */
		void BeginScope(const GpuSubmissionTimeline& timeline);

		/** Ends the scope started with BeginScope(). Drops every allocation and tag of the scope, and its heap references. */
		void EndScope();

		/**
		 * Places @p size bytes of memory with @p alignment in @p pool, for a resource whose first use is in submission
		 * @p firstSubmission of the scope's timeline. Must be called within a scope.
		 *
		 * @param	pool				Pool to place the memory in.
		 * @param	size				Size of the memory, in bytes.
		 * @param	alignment			Required alignment of the memory's offset. Must be a power of two.
		 * @param	firstSubmission		Index of the submission in the scope's timeline that uses the resource first.
		 * @param	outAllocation		Non-owning allocation referencing the memory. Identifies the memory to Release() and
		 *								TryAllocateAt().
		 * @param	outAcquire			Accesses on the queue of @p firstSubmission that the first use of the resource must be
		 *								ordered after. In development builds also lists the earlier resources on the memory,
		 *								valid until the next call to TryAllocate() or TryAllocateAt().
		 * @return						False if a heap that fits the memory could not be created.
		 */
		bool TryAllocate(IGpuTransientHeapPool& pool, u64 size, u64 alignment, u32 firstSubmission, GpuAllocation& outAllocation, render::GpuAliasAcquire& outAcquire);

		/**
		 * Places memory at exactly the location of @p location, an allocation that TryAllocate() or TryAllocateAt() returned in
		 * this or an earlier scope, and whose heap is still referenced. Fails if that memory is not free in the scope, or if
		 * the first use is not ordered after its earlier uses. Parameters otherwise match TryAllocate().
		 */
		bool TryAllocateAt(IGpuTransientHeapPool& pool, const GpuAllocation& location, u32 firstSubmission, GpuAllocation& outAllocation, render::GpuAliasAcquire& outAcquire);

		/**
		 * Releases the memory of @p allocation, so later allocations of the scope may reuse it once they are ordered after
		 * @p lastUses. Must be called within the scope that allocated the memory.
		 *
		 * @param	allocation	Allocation returned by TryAllocate() or TryAllocateAt().
		 * @param	lastUses	Last accesses of the resource placed on the memory, at most one per queue, in the scope's timeline.
		 *						Empty if the resource was never accessed, in which case the memory keeps the accesses of the
		 *						resources released on it before.
		 * @param	owner		Resource placed on the memory. Reported to later allocations of the memory in development builds.
		 */
		void Release(const GpuAllocation& allocation, TArrayView<const GpuTransientLastUse> lastUses, IGpuResource* owner);

		/** Returns the memory use of the open scope, or of the last scope if none is open. */
		GpuTransientScopeStatistics GetStatistics() const;

	private:
		static constexpr u32 kNoTag = ~0u;
		static constexpr u32 kInvalidIndex = ~0u;

		/** Last use of released memory on one queue. */
		struct TagUse
		{
			TagUse(GpuQueueId queue, u32 lastSubmission, const render::GpuAccessScope& access)
				: Queue(queue), LastSubmission(lastSubmission), Access(access)
			{ }

			GpuQueueId Queue;
			u32 LastSubmission; /**< Index of the submission in the scope's timeline. */
			render::GpuAccessScope Access;
		};

		/** Last uses of the resource released on a range of memory in the scope. */
		struct Tag
		{
			u32 FirstUse = 0; /**< Index of the tag's first use in mTagUses. */
			u32 UseCount = 0; /**< At most one use per queue. */

#if B3D_BUILD_TYPE_DEVELOPMENT
			IGpuResource* Owner = nullptr; /**< Resource released on the memory. */
#endif
		};

		/** Range of heap memory, tagged with its last uses. */
		struct Segment
		{
			Segment(u64 offset, u64 size, u32 tag)
				: Offset(offset), Size(size), Tag(tag)
			{ }

			u64 GetEnd() const { return Offset + Size; }

			u64 Offset;
			u64 Size;
			u32 Tag; /**< Index into mTags, or kNoTag if the memory has no earlier uses in the scope. */
		};

		/** Heap the scope placed memory on, and its free memory in the scope. */
		struct Heap
		{
			Heap(IGpuTransientHeapPool* pool, u32 slot, const GpuTransientHeap& heap)
				: Pool(pool), Slot(slot), Handle(heap.Heap), Size(heap.Size)
			{ }

			IGpuTransientHeapPool* Pool;
			u32 Slot; /**< Slot of the heap in its pool. */
			IGpuHeap* Handle; /**< Heap the scope holds a reference to. */
			u64 Size;
			TArray<Segment> FreeSegments; /**< Ordered by offset. Adjacent segments always have different tags. */
		};

		/** Live allocation, and the tagged segments it was placed over. */
		struct Allocation
		{
			u32 HeapIndex = kInvalidIndex; /**< Index into mHeaps, or kInvalidIndex if the allocation slot is unused. */
			u64 Offset = 0;
			u64 Size = 0;
			u32 FirstInheritedSegment = 0; /**< Index of the allocation's first inherited segment in mInheritedSegments. */
			u32 InheritedSegmentCount = 0;
		};

		/** Returns the index of the scope's heap in @p slot of @p pool, or kInvalidIndex if the scope has not placed memory on it. */
		u32 FindHeap(const IGpuTransientHeapPool& pool, u32 slot) const;

		/** Adds @p heap, acquired from @p slot of @p pool, to the scope with all of its memory free. Returns its index in mHeaps. */
		u32 AddHeap(IGpuTransientHeapPool& pool, u32 slot, const GpuTransientHeap& heap);

		/**
		 * Returns true if memory tagged with @p tag may be reused by a resource whose first use is in @p firstSubmission: its
		 * last uses on every queue are ordered before the first use.
		 */
		bool IsEligible(u32 tag, u32 firstSubmission) const;

		/**
		 * Allocates @p size bytes at @p offset of the heap at @p heapIndex, over its free segments @p firstSegmentIndex to
		 * @p lastSegmentIndex. Returns the allocation and the acquire of its first use in @p firstSubmission.
		 */
		void Place(u32 heapIndex, u32 firstSegmentIndex, u32 lastSegmentIndex, u64 offset, u64 size, u32 firstSubmission, GpuAllocation& outAllocation,
			render::GpuAliasAcquire& outAcquire);

		/** Returns the live allocation that @p allocation refers to. */
		Allocation& GetAllocation(const GpuAllocation& allocation);

		/** Adds @p segment to the free memory of @p heap, merging it with adjacent segments with the same tag. */
		static void InsertFreeSegment(Heap& heap, const Segment& segment);

		const GpuSubmissionTimeline* mTimeline = nullptr; /**< Timeline of the open scope, or null outside of a scope. */
		bool mIsAliasingEnabled = true;
		bool mIsAliasingAcrossQueuesEnabled = true;

		TArray<Heap> mHeaps;
		TArray<Tag> mTags;
		TArray<TagUse> mTagUses; /**< Uses of every tag of the scope, in tag order. Tags never change once created. */
		TArray<Allocation> mAllocations;

		/**
		 * Inherited segments of every allocation placed in the scope, in placement order. Segments of released allocations stay
		 * until the scope ends.
		 */
		TArray<Segment> mInheritedSegments;
		TArray<u32> mFreeAllocationIndices;

		u64 mUsedBytes = 0;
		u64 mPeakUsedBytes = 0;
		u64 mRequestedBytes = 0;

#if B3D_BUILD_TYPE_DEVELOPMENT
		TInlineArray<IGpuResource*, 4> mPredecessors; /**< Earlier resources on the memory of the last allocation. */
#endif
	};

	/** @} */
} // namespace b3d
