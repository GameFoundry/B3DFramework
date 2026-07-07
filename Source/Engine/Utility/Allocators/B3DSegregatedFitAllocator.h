//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DBitwise.h"
#include "Allocators/B3DMemoryAllocator.h"
#include <utility>

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Default backend for TSegregatedFitAllocator: sources its slabs straight from the system heap using aligned
	 * malloc/free. Suitable whenever the sub-allocated memory only needs to be CPU-addressable.
	 *
	 * A backend is any type providing this two-method interface (see TSegregatedFitAllocator for the contract):
	 *   void* Allocate(u64 sizeBytes, u64 alignment);   // returns a base pointer, or nullptr on failure
	 *   void  Free(void* pointer);                       // releases a pointer previously returned by Allocate
	 */
	class SystemMemoryBackend
	{
	public:
		void* Allocate(u64 sizeBytes, u64 alignment) { return B3DAllocateAligned((size_t)sizeBytes, (size_t)alignment); }
		void Free(void* pointer) { B3DFreeAligned(pointer); }
	};

	/**
	 * Single-level segregated free-list allocato. It sub-divides one or more large slabs into variable-sized blocks
	 * measured in fixed-size slots, and services allocations from per-size-class free lists so a fitting free run
	 * is found without scanning the whole heap.
	 *
	 * Free runs are binned by their length in slots into power-of-two size classes: separate list heads for runs of
	 * 1, [2,3], [4,7], ... slots, with the final class unbounded (holding everything at or above its lower bound).
	 * Allocation rounds the request up to whole slots, scans from the request's own class upward, and takes the first
	 * fitting run (splitting off any surplus tail as a new free run). Freeing coalesces a run with its immediate
	 * physical neighbours in the same slab. All bookkeeping lives in CPU-side, index-linked node records (recycled on
	 * free) - the managed memory itself is never touched, so the allocator is equally usable over GPU-visible memory.
	 *
	 * The allocator owns its memory: it grows a new slab through the backend on demand (when no existing free run fits)
	 * and frees every slab through the backend on destruction. It never touches memory beyond what the backend hands
	 * back.
	 *
	 * @tparam	Backend			Supplies and releases slab memory. Must provide @c void* @c Allocate(u64 sizeBytes,
	 *							u64 alignment) (returning nullptr on failure) and @c void @c Free(void* pointer). Held by
	 *							value, so it must be movable. See SystemMemoryBackend for the default.
	 * @tparam	SlotSizeBytes	Allocation granularity, in bytes. Every request is rounded up to a whole number of
	 *							these slots. Must be a power of two.
	 * @tparam	SizeClassCount	Number of segregated free lists. Class @c i (for @c i < SizeClassCount-1) holds runs of
	 *							[2^i, 2^(i+1)-1] slots; the last class is unbounded.
	 *
	 * @note	Not thread safe. Intended to be owned and driven from a single thread.
	 */
	template <typename Backend, u32 SlotSizeBytes = 16, u32 SizeClassCount = 7>
	class TSegregatedFitAllocator
	{
	public:
		/** Allocation granularity, in bytes. Every request is rounded up to a whole number of these slots. */
		static constexpr u32 kSlotSizeBytes = SlotSizeBytes;

		/** Sentinel node handle - "no node" / "end of list" / "not a live allocation". */
		static constexpr u32 kInvalidNode = ~0u;

		static_assert(SlotSizeBytes > 0 && (SlotSizeBytes & (SlotSizeBytes - 1)) == 0, "SlotSizeBytes must be a power of two.");
		static_assert(SizeClassCount > 0, "SizeClassCount must be at least one.");

		/** Handle to one live allocation: the usable base pointer plus the private free-list node backing it. */
		struct Allocation
		{
			/** Base pointer of the allocated block, or null for an invalid (failed) allocation. */
			void* Data = nullptr;

			/** Private free-list node backing the block; kInvalidNode for an invalid allocation. */
			u32 Node = kInvalidNode;

			bool IsValid() const { return Data != nullptr; }
		};

		/** Construction parameters. */
		struct Configuration
		{
			/** 
			 * Size of each slab the allocator grows through the backend, in bytes. Must be a multiple of the slot
			 * size and at least one slot; an allocation larger than this grows a slab sized to fit it instead. 
			 */
			u64 SlabSizeBytes = 0;

			/** Alignment requested from the backend for a slab base. Every block start inherits at most this. */
			u64 SlabAlignment = SlotSizeBytes;
		};

		/**
		 * @param	configuration	Slab sizing/alignment. SlabSizeBytes must be a positive multiple of the slot size.
		 * @param	backend			Backend instance, taken by value (moved into the allocator).
		 */
		explicit TSegregatedFitAllocator(const Configuration& configuration, Backend backend = Backend())
			: mBackend(std::move(backend)), mConfiguration(configuration)
		{
			B3D_ASSERT(mConfiguration.SlabSizeBytes >= kSlotSizeBytes && (mConfiguration.SlabSizeBytes % kSlotSizeBytes) == 0);

			for(u32& head : mFreeListHeads)
				head = kInvalidNode;
		}

		~TSegregatedFitAllocator()
		{
			for(Slab& slab : mSlabs)
			{
				if(slab.Base != nullptr)
					mBackend.Free(slab.Base);
			}
		}

		TSegregatedFitAllocator(const TSegregatedFitAllocator&) = delete;
		TSegregatedFitAllocator& operator=(const TSegregatedFitAllocator&) = delete;

		/**
		 * Allocates @p sizeBytes (rounded up to whole slots), growing a new slab through the backend if no existing
		 * free run fits. Returns an invalid Allocation when @p sizeBytes is zero or the backend cannot supply memory.
		 */
		Allocation Allocate(u64 sizeBytes)
		{
			if(sizeBytes == 0)
				return Allocation();

			const u32 sizeSlots = (u32)((sizeBytes + kSlotSizeBytes - 1) / kSlotSizeBytes);

			u32 foundNode = FindFreeRun(sizeSlots);
			if(foundNode == kInvalidNode)
			{
				// No free run fits: grow a new slab and retry once.
				if(!GrowSlab(sizeBytes))
					return Allocation();

				foundNode = FindFreeRun(sizeSlots);
				if(foundNode == kInvalidNode)
					return Allocation();
			}

			RemoveFree(foundNode);

			// Split the surplus tail into its own free run. AllocateNode() may grow mNodes, so all accesses go
			// through indices - no references into the vector are held across it.
			if(mNodes[foundNode].SizeSlots > sizeSlots)
			{
				const u32 remainderNode = AllocateNode();

				mNodes[remainderNode].SlabIndex = mNodes[foundNode].SlabIndex;
				mNodes[remainderNode].OffsetSlots = mNodes[foundNode].OffsetSlots + sizeSlots;
				mNodes[remainderNode].SizeSlots = mNodes[foundNode].SizeSlots - sizeSlots;
				mNodes[remainderNode].PhysPrev = foundNode;
				mNodes[remainderNode].PhysNext = mNodes[foundNode].PhysNext;
				mNodes[remainderNode].FreePrev = kInvalidNode;
				mNodes[remainderNode].FreeNext = kInvalidNode;
				mNodes[remainderNode].IsFree = false;

				if(mNodes[foundNode].PhysNext != kInvalidNode)
					mNodes[mNodes[foundNode].PhysNext].PhysPrev = remainderNode;

				mNodes[foundNode].PhysNext = remainderNode;
				mNodes[foundNode].SizeSlots = sizeSlots;

				PushFree(remainderNode);
			}

			Allocation allocation;
			allocation.Data = static_cast<u8*>(mSlabs[mNodes[foundNode].SlabIndex].Base) + (u64)mNodes[foundNode].OffsetSlots * kSlotSizeBytes;
			allocation.Node = foundNode;
			return allocation;
		}

		/**
		 * Returns a block to the allocator, coalescing it with any free physical neighbours. Passing an invalid
		 * Allocation (never allocated, or already freed and default-reset) is a no-op.
		 */
		void Free(const Allocation& allocation)
		{
			if(allocation.Node == kInvalidNode)
				return;

			FreeNode(allocation.Node);
		}

	private:
		/** A single slab handed back by the backend; blocks are carved out of it in slot units. */
		struct Slab
		{
			void* Base = nullptr; /**< Backend base pointer of the slab. */
			u32 SizeSlots = 0; /**< Length of the slab, in slots. */
		};

		/** One contiguous run of slots within a slab; either live (handed out) or sitting on a free list. */
		struct Node
		{
			u32 SlabIndex = 0; /**< Slab the run lives in. */
			u32 OffsetSlots = 0; /**< First slot of the run, relative to the slab start. */
			u32 SizeSlots = 0; /**< Length of the run, in slots. */
			u32 PhysPrev = kInvalidNode; /**< Physically preceding run in the same slab, or kInvalidNode. */
			u32 PhysNext = kInvalidNode; /**< Physically following run in the same slab, or kInvalidNode. */
			u32 FreePrev = kInvalidNode; /**< Previous node in the size-class free list (free nodes only). */
			u32 FreeNext = kInvalidNode; /**< Next node in the size-class free list; doubles as the node-recycle link. */
			bool IsFree = false; /**< True while the run sits on a free list. */
		};

		/** Maps a run length in slots to its size-class list index (capped at the unbounded top class). */
		static u32 GetSizeClass(u32 sizeSlots)
		{
			B3D_ASSERT(sizeSlots > 0);

			// One level of segregation: power-of-two ranges [1], [2,3], [4,7], ... with the final class unbounded.
			const u32 msb = Bitwise::MostSignificantBit(sizeSlots);
			return msb < SizeClassCount ? msb : SizeClassCount - 1;
		}

		/** First-fit scan from the request's own size class upward. Returns the fitting node, or kInvalidNode. */
		u32 FindFreeRun(u32 sizeSlots)
		{
			// The request's own class (and the unbounded top class) can hold runs smaller than the request, so
			// candidate sizes are always checked; every run in an intermediate class is at least the class lower
			// bound and therefore always fits.
			for(u32 sizeClass = GetSizeClass(sizeSlots); sizeClass < SizeClassCount; sizeClass++)
			{
				for(u32 candidate = mFreeListHeads[sizeClass]; candidate != kInvalidNode; candidate = mNodes[candidate].FreeNext)
				{
					if(mNodes[candidate].SizeSlots >= sizeSlots)
						return candidate;
				}
			}

			return kInvalidNode;
		}

		/** Grows a new slab through the backend, seeding it as one free run. Returns false on backend failure. */
		bool GrowSlab(u64 minSizeBytes)
		{
			// The slab must fit an oversized request; otherwise it takes the configured size.
			const u64 minSlabBytes = Bitwise::AlignUp<u64>(minSizeBytes, kSlotSizeBytes);
			const u64 slabBytes = mConfiguration.SlabSizeBytes >= minSlabBytes ? mConfiguration.SlabSizeBytes : minSlabBytes;

			void* const base = mBackend.Allocate(slabBytes, mConfiguration.SlabAlignment);
			if(base == nullptr)
				return false;

			const u32 slabIndex = (u32)mSlabs.size();
			mSlabs.push_back(Slab{ base, (u32)(slabBytes / kSlotSizeBytes) });

			const u32 nodeIndex = AllocateNode();
			mNodes[nodeIndex].SlabIndex = slabIndex;
			mNodes[nodeIndex].OffsetSlots = 0;
			mNodes[nodeIndex].SizeSlots = mSlabs[slabIndex].SizeSlots;
			mNodes[nodeIndex].PhysPrev = kInvalidNode;
			mNodes[nodeIndex].PhysNext = kInvalidNode;
			mNodes[nodeIndex].FreePrev = kInvalidNode;
			mNodes[nodeIndex].FreeNext = kInvalidNode;
			mNodes[nodeIndex].IsFree = false;

			PushFree(nodeIndex);
			return true;
		}

		/** Takes a node record off the recycle list, or grows the node pool. Fields are left stale - caller initializes them. */
		u32 AllocateNode()
		{
			if(mRecycledNodeHead != kInvalidNode)
			{
				const u32 nodeIndex = mRecycledNodeHead;
				mRecycledNodeHead = mNodes[nodeIndex].FreeNext;
				return nodeIndex;
			}

			mNodes.push_back(Node());
			return (u32)mNodes.size() - 1;
		}

		/** Returns a node record to the recycle list. */
		void ReleaseNode(u32 nodeIndex)
		{
			mNodes[nodeIndex].IsFree = false;
			mNodes[nodeIndex].FreeNext = mRecycledNodeHead;
			mRecycledNodeHead = nodeIndex;
		}

		/** Pushes a node onto the free list matching its current SizeSlots and marks it free. */
		void PushFree(u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			const u32 sizeClass = GetSizeClass(node.SizeSlots);

			node.IsFree = true;
			node.FreePrev = kInvalidNode;
			node.FreeNext = mFreeListHeads[sizeClass];

			if(node.FreeNext != kInvalidNode)
				mNodes[node.FreeNext].FreePrev = nodeIndex;

			mFreeListHeads[sizeClass] = nodeIndex;
		}

		/** Unlinks a node from the free list matching its current SizeSlots and marks it live. */
		void RemoveFree(u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			const u32 sizeClass = GetSizeClass(node.SizeSlots);

			if(node.FreePrev != kInvalidNode)
				mNodes[node.FreePrev].FreeNext = node.FreeNext;
			else
				mFreeListHeads[sizeClass] = node.FreeNext;

			if(node.FreeNext != kInvalidNode)
				mNodes[node.FreeNext].FreePrev = node.FreePrev;

			node.IsFree = false;
			node.FreePrev = kInvalidNode;
			node.FreeNext = kInvalidNode;
		}

		/** Frees the run held by @p nodeIndex, coalescing with free physical neighbours in the same slab. */
		void FreeNode(u32 nodeIndex)
		{
			B3D_ASSERT(nodeIndex < (u32)mNodes.size());
			B3D_ASSERT(!mNodes[nodeIndex].IsFree && "Block freed twice.");

			// Coalesce with free physical neighbours. The physical chain never crosses a slab boundary (each slab's
			// runs form their own chain, seeded by GrowSlab), so merged runs always stay within one slab. RemoveFree
			// must run before SizeSlots changes - it locates the free list by the node's current size class.
			u32 mergedNode = nodeIndex;

			const u32 previousNode = mNodes[nodeIndex].PhysPrev;
			if(previousNode != kInvalidNode && mNodes[previousNode].IsFree)
			{
				RemoveFree(previousNode);

				mNodes[previousNode].SizeSlots += mNodes[nodeIndex].SizeSlots;
				mNodes[previousNode].PhysNext = mNodes[nodeIndex].PhysNext;
				if(mNodes[nodeIndex].PhysNext != kInvalidNode)
					mNodes[mNodes[nodeIndex].PhysNext].PhysPrev = previousNode;

				ReleaseNode(nodeIndex);
				mergedNode = previousNode;
			}

			const u32 nextNode = mNodes[mergedNode].PhysNext;
			if(nextNode != kInvalidNode && mNodes[nextNode].IsFree)
			{
				RemoveFree(nextNode);

				mNodes[mergedNode].SizeSlots += mNodes[nextNode].SizeSlots;
				mNodes[mergedNode].PhysNext = mNodes[nextNode].PhysNext;
				if(mNodes[nextNode].PhysNext != kInvalidNode)
					mNodes[mNodes[nextNode].PhysNext].PhysPrev = mergedNode;

				ReleaseNode(nextNode);
			}

			PushFree(mergedNode);
		}

		Backend mBackend; /**< Supplies and releases slab memory. */
		Configuration mConfiguration; /**< Slab sizing/alignment. */

		Vector<Slab> mSlabs; /**< Slabs this allocator owns, indexed by Node::SlabIndex. */
		Vector<Node> mNodes; /**< Node record pool; handles are indices into it, freed records are recycled. */
		u32 mRecycledNodeHead = kInvalidNode; /**< Head of the singly-linked (via FreeNext) recycle list. */
		u32 mFreeListHeads[SizeClassCount]; /**< Head of the doubly-linked free list per size class. */
	};

	/** @} */
	/** @} */
} // namespace b3d
