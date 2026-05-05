//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Internal types and helpers for the Two-Level Segregated Fit GPU memory allocator. Not part
	 * of the public surface — referenced only inside TGpuTlsfAllocator's implementation.
	 */
	namespace detail::tlsf
	{
		//----------------------------------------------------------------------------------------
		// Declarations
		//----------------------------------------------------------------------------------------

		/**
		 * Helper used by the TLSF allocator to ensure that non-linear images are placed at correct alignment
		 * (granularity). Some backends have different alignment requirements if a non-linear image follows or
		 * trails a linear memory allocation, which this object helps to track.
		 *
		 * One entry per granularity-aligned page. Only the start and end pages of each allocation are ever referenced.
		 *
		 * Default-constructed instances are inert (no allocation). Non-copyable.
		 */
		class GranularityTracker
		{
		public:
			GranularityTracker() = default;
			~GranularityTracker();

			GranularityTracker(const GranularityTracker&) = delete;
			GranularityTracker& operator=(const GranularityTracker&) = delete;

			/**
			 * Allocate the page table sized for @p heapSize. When @p granularity is <= 1 or
			 * <= @p disableThreshold the tracker stays inert — every call below short-circuits.
			 * @p disableThreshold is useful if allocations are guaranteed to be aligned to this
			 * value regardless of buffer-image granularity.
			 */
			void Initialize(u64 heapSize, u64 granularity, u64 disableThreshold);

			/** Releases the page table. Safe to call on an inert tracker. */
			void Destroy();

			/** True when the page table is allocated and the conflict checks are live. */
			bool IsEnabled() const { return mPages != nullptr; }

			/** Bumps the refcounts for the start + end pages of @p [offset, offset+size). */
			void MarkPages(u64 offset, u64 size, GpuResourceKind kind);

			/** Decrements the refcounts for the start + end pages; resets category to Free at zero. */
			void UnmarkPages(u64 offset, u64 size);

			/**
			 * Adjust @p inOutOffset upward to clear any granularity conflict at the start page;
			 * return false if the adjusted range overruns @p blockEnd or the end page holds
			 * a conflicting allocation. Returns true (no-op) when the tracker is inert.
			 */
			bool CheckAndAlignUp(u64& inOutOffset, u64 size, GpuResourceKind kind, u64 blockEnd) const;

#if B3D_DEBUG
			/** Asserts every page has zero LiveCount — sanity check when a heap goes empty. */
			void AssertEmpty() const;
#endif

		private:
			/** Resource-kind category stored per granularity page. */
			enum class PageCategory : u8
			{
				Linear = (u8)GpuResourceKind::Linear,
				NonLinear = (u8)GpuResourceKind::NonLinear,
				Free = 0xFF, /**< Sentinel value for an empty page (no live allocations touch it). */
			};

			/** Describes one page (memory range as wide as the granularity) and its category. */
			struct Page
			{
				PageCategory Category; /**< PageCategory::Free when no live allocation touches this page. */
				u16 LiveCount; /**< Number of allocations touching this page. */
			};

			/** Returns true if two categories cannot exist in the same granularity page. */
			static bool IsConflict(PageCategory a, PageCategory b);

			Page* mPages = nullptr;
			u32 mPageCount = 0;
			u64 mGranularity = 1;
			u32 mPageShift = 0;
		};

		/**
		 * Compile-time constants and pure helper functions shared between Heap and TGpuTlsfAllocator.
		 * Pure: no per-instance state, no template parameter.
		 */
		namespace Utility
		{
			/** First-level class count. Capped at u32 bitmap width — covers heaps up to 2^(kFirstLevelClassCount + kMemoryClassShift) bytes. */
			constexpr u32 kFirstLevelClassCount = 32;

			/** Number of low bits removed from MSB(size) when computing the first-level class for sizes > kSmallBufferSize. */
			constexpr u32 kMemoryClassShift = 7;

			/** Sizes <= this are bucketed entirely within first-level class 0. */
			constexpr u64 kSmallBufferSize = 256;

			/** Granule width for second-level buckets inside first-level class 0. */
			constexpr u32 kSmallBufferGranule = 8;

			/** log2 of the second-level bucket count per first-level class. */
			constexpr u32 kSecondLevelIndexBits = 5;

			/** Second-level buckets per first-level class. */
			constexpr u32 kSecondLevelCount = 1u << kSecondLevelIndexBits;

			/** Total free-list bucket count per heap. */
			constexpr u32 kFreeListCount = kFirstLevelClassCount * kSecondLevelCount;

			/** Sentinel index for "no node" / "end of list" — stored in physical / free-list link fields. */
			constexpr u32 kInvalidIndex = ~0u;

			// FirstLevelFreeBitmask is a u32; if the first-level class count grows past 32 the bitmap type must widen.
			static_assert(kFirstLevelClassCount <= 32, "FirstLevelFreeBitmask is u32; widen the bitmap if more first-level classes are required");

			// Likewise for SecondLevelFreeBitmask[firstLevel].
			static_assert(kSecondLevelCount <= 32, "SecondLevelFreeBitmask entries are u32; widen the type if more second-level buckets are required");

			/**
			 * Maps an allocation size to a (firstLevel, secondLevel) bucket. The first-level class is the MSB-derived
			 * size order, the second-level class linearly subdivides each first-level range into kSecondLevelCount sub-buckets.
			 *
			 * For sizes in [1, kSmallBufferSize] the first-level class is forced to 0 and the second-level index is
			 * derived from kSmallBufferGranule-byte granules so small allocations stay segregated below the natural
			 * MSB-class boundaries.
			 */
			inline void SizeToBucket(u64 size, u32& firstLevel, u32& secondLevel)
			{
				if (size <= kSmallBufferSize)
				{
					firstLevel = 0;
					secondLevel = (size > 0) ? (u32)((size - 1) / kSmallBufferGranule) : 0;
					return;
				}

				firstLevel = (u32)Bitwise::MostSignificantBit(size) - kMemoryClassShift;
				const u32 shift = firstLevel + kMemoryClassShift - kSecondLevelIndexBits;
				secondLevel = (u32)((size >> shift) ^ kSecondLevelCount);
			}

			/** Flat free-list index for a (firstLevel, secondLevel) bucket. */
			inline u32 GetListIndex(u32 firstLevel, u32 secondLevel)
			{
				return firstLevel * kSecondLevelCount + secondLevel;
			}

			/** Round @p value up to the next multiple of @p alignment (which must be a power of two). */
			inline u64 AlignUp(u64 value, u32 alignment)
			{
				const u64 mask = (u64)alignment - 1;
				return (value + mask) & ~mask;
			}
		} // namespace Utility

		/** State bits stored on each pool node. */
		enum class NodeFlag : u32
		{
			Free				= 1u << 0, /**< Set when the node is on a free list (or is the trailing null node). */
			NullNode			= 1u << 1, /**< Set when the node is the trailing null node of its heap. */
			NonLinear			= 1u << 2, /**< Set when a live allocation is non-linear (optimally-tiled image). */
			DefragDestination	= 1u << 3, /**< Set on slots reserved as defrag destinations within the current Defrag() pass; cleared at end of Defrag(). */
		};

		using NodeFlags = Flags<NodeFlag, u32>;

		/**
		 * Pool node describing a contiguous range within one heap. Indexed by u32 so node
		 * identity fits in TGpuResourceLocation::AllocatorData1.
		 */
		struct Node
		{
			u64 Offset;
			u64 Size;

			// Heap-order doubly-linked list. kInvalidIndex at the heap start / end.
			u32 PrevPhysical;
			u32 NextPhysical;

			// Free-list doubly-linked list when the node is free; unused otherwise.
			u32 PrevFree;
			u32 NextFree;

			NodeFlags Flags;

			IGpuResource* Owner; /**< Owning resource for defragmentation. nullptr when the slot is untracked or free. */

			bool IsFree() const { return Flags.IsSet(NodeFlag::Free); }
			bool IsNullNode() const { return Flags.IsSet(NodeFlag::NullNode); }
			bool IsDefragDestination() const { return Flags.IsSet(NodeFlag::DefragDestination); }
		};

		/**
		 * u32-indexed node storage with a freelist of vacated slots. Owned by a single Heap;
		 * node identities are heap-local — the orchestrator keys cross-heap references on
		 * (heapIndex, nodeIndex) pairs.
		 */
		class NodePool
		{
		public:
			NodePool() = default;

			NodePool(const NodePool&) = delete;
			NodePool& operator=(const NodePool&) = delete;

			/** Acquire a free node-pool slot, growing the underlying vector if necessary. */
			u32 Allocate();

			/** Return a node-pool slot to the free list. Clears Flags only; remaining fields are reinitialized on re-acquisition. */
			void Release(u32 nodeIndex);

			Node& operator[](u32 nodeIndex) { return mNodes[nodeIndex]; }
			const Node& operator[](u32 nodeIndex) const { return mNodes[nodeIndex]; }

		private:
			Vector<Node> mNodes;
			u32 mFreeHead = Utility::kInvalidIndex;
		};

		/**
		 * Per-heap TLSF state and algorithms. Owns its own NodePool — node indices are heap-local.
		 *
		 * Encapsulates the inner search/carve/coalesce logic for a single backend heap; cross-heap
		 * orchestration (heap pool, defragmentation, empty-spare bookkeeping) lives in TGpuTlsfAllocator.
		 *
		 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
		 */
		template <typename HeapBackend>
		class Heap
		{
		public:
			using HeapHandle = typename HeapBackend::HeapHandle;

			/**
			 * Construct a heap of @p size bytes around the already-allocated backend handle. The backend
			 * CreateHeap call is the orchestrator's job; this constructor only sets up the per-heap
			 * bookkeeping and allocates the trailing null block from the heap's own node pool.
			 */
			Heap(HeapHandle handle, u64 size, u64 granularity, u64 granularityDisableThreshold, u64 minAllocationSize);

			~Heap() = default; // mPool / mGranularity dtors release all node + page storage.

			Heap(const Heap&) = delete;
			Heap& operator=(const Heap&) = delete;

			/**
			 * Reserves a slot of @p size bytes within this heap: fast-fails on insufficient size, walks
			 * the TLSF buckets for a fitting free node, carves, marks the granularity pages, updates
			 * bookkeeping (FreeSize, LiveAllocCount). On success writes the carved node's index to
			 * @p outNodeIndex; the caller reads GetNode(outNodeIndex) for offset / size and is responsible
			 * for stamping Owner and building the public Location.
			 */
			bool TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, u32& outNodeIndex);

			/**
			 * Releases the allocation at @p nodeIndex. Coalesces with adjacent free neighbors and folds
			 * trailing free space back into the null block. Updates LiveAllocCount and FreeSize. The
			 * orchestrator handles the empty-spare bookkeeping that follows when LiveAllocCount transitions to 0.
			 */
			void FreeNode(u32 nodeIndex);

			HeapHandle Handle() const { return mHandle; }
			u64 TotalSize() const { return mTotalSize; }
			u64 FreeSize() const { return mFreeSize; }
			u32 LiveAllocCount() const { return mLiveAllocCount; }
			u32 NullNodeIndex() const { return mNullNodeIndex; }
			u32 PhysicalListHead() const { return mPhysicalListHead; }

			/**
			 * Read-only node access — used by the orchestrator's defrag walk to inspect Owner / Flags /
			 * Offset / Size / PrevPhysical without leaking the whole pool.
			 */
			const Node& GetNode(u32 nodeIndex) const { return mPool[nodeIndex]; }

			/** Owner stamp — orchestrator-driven (defrag tracking). Mutates only the Owner field. */
			void SetNodeOwner(u32 nodeIndex, IGpuResource* owner) { mPool[nodeIndex].Owner = owner; }

			/**
			 * Defrag-destination flag — stamped by the orchestrator on a destination slot reserved
			 * inside the current Defrag() pass; cleared at end of pass. The flag keeps the destination
			 * invisible to subsequent iteration in the same Defrag() invocation.
			 */
			void SetDefragDestinationFlag(u32 nodeIndex) { mPool[nodeIndex].Flags |= NodeFlag::DefragDestination; }
			void ClearDefragDestinationFlag(u32 nodeIndex) { mPool[nodeIndex].Flags.Unset(NodeFlag::DefragDestination); }

		private:
			NodePool mPool;
			HeapHandle mHandle{};
			u64 mTotalSize = 0;
			u64 mFreeSize = 0;
			u32 mLiveAllocCount = 0;
			u32 mPhysicalListHead = Utility::kInvalidIndex;
			u32 mNullNodeIndex = Utility::kInvalidIndex;

			u32 mFreeListHead[Utility::kFreeListCount]; /**< Free-list head per (firstLevel, secondLevel) bucket. Updated alongside the bitmaps. */
			u32 mFirstLevelFreeBitmask = 0; /**< Bit set if any entry in mSecondLevelFreeBitmask[firstLevel] is non-zero. */
			u32 mSecondLevelFreeBitmask[Utility::kFirstLevelClassCount]; /**< Bit set when mFreeListHead[(firstLevel, secondLevel)] is non-empty. */

			GranularityTracker mGranularity; /**< Buffer-image granularity tracker — inert when the allocator is configured with granularity <= 1 or below the threshold. */
			u64 mMinAllocationSize = 0;

			/** Insert @p nodeIndex into the appropriate (firstLevel, secondLevel) bucket and update bitmaps. */
			void InsertIntoFreeList(u32 nodeIndex);

			/** Splice @p nodeIndex out of its free list and clear bitmap bits if its bucket is now empty. */
			void RemoveFromFreeList(u32 nodeIndex);

			/**
			 * Find a free node that can satisfy a (size, alignment, kind) request. Searches the natural
			 * bucket first (best-fit candidates live there) and walks larger buckets via the bitmaps if
			 * needed. The returned @p outAlignedOffset folds in both natural alignment and any buffer image
			 * granularity inflation, so the carver doesn't have to recompute either. Returns kInvalidIndex on miss.
			 */
			u32 FindFreeNode(u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const;

			/**
			 * Walk a bucket's free list and return the first node large enough to satisfy (size, alignment, kind).
			 * The returned @p outAlignedOffset contains any buffer image granularity past natural alignment.
			 */
			u32 WalkBucketForFit(u32 listIndex, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const;

			/**
			 * Carve a @p size byte allocation starting at @p alignedOffset out of the candidate node, splitting
			 * any leading padding and trailing remainder into separate free nodes. Returns the node-index of the allocated block.
			 */
			u32 CarveAllocation(u32 candidateIndex, u64 alignedOffset, u64 size);
		};

		//----------------------------------------------------------------------------------------
		// GranularityTracker definitions
		//----------------------------------------------------------------------------------------

		inline GranularityTracker::~GranularityTracker()
		{
			Destroy();
		}

		inline bool GranularityTracker::IsConflict(PageCategory a, PageCategory b)
		{
			if (a == PageCategory::Free || b == PageCategory::Free)
				return false;

			return a != b;
		}

		inline void GranularityTracker::Initialize(u64 heapSize, u64 granularity, u64 disableThreshold)
		{
			// Idempotent — covers the "Init twice" sanity case.
			Destroy();

			if (granularity <= 1 || granularity <= disableThreshold)
				return;

			B3D_ASSERT(Bitwise::IsPow2(granularity));
			mGranularity = granularity;
			mPageShift = (u32)Bitwise::MostSignificantBit(granularity);
			mPageCount = (u32)((heapSize + granularity - 1) >> mPageShift);
			mPages = (Page*)B3DAllocate(mPageCount * sizeof(Page));
			for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
			{
				mPages[pageIndex].Category = PageCategory::Free;
				mPages[pageIndex].LiveCount = 0;
			}
		}

		inline void GranularityTracker::Destroy()
		{
			if (mPages != nullptr)
				B3DFree(mPages);

			mPages = nullptr;
			mPageCount = 0;
			mGranularity = 1;
			mPageShift = 0;
		}

		inline void GranularityTracker::MarkPages(u64 offset, u64 size, GpuResourceKind kind)
		{
			if (mPages == nullptr)
				return;

			const u32 startPage = (u32)(offset >> mPageShift);
			const u32 endPage = (u32)((offset + size - 1) >> mPageShift);
			const PageCategory category = (PageCategory)kind;

			if (mPages[startPage].LiveCount == 0 || mPages[startPage].Category == PageCategory::Free)
				mPages[startPage].Category = category;

			mPages[startPage].LiveCount++;

			if (endPage != startPage)
			{
				if (mPages[endPage].LiveCount == 0 || mPages[endPage].Category == PageCategory::Free)
					mPages[endPage].Category = category;

				mPages[endPage].LiveCount++;
			}
		}

		inline void GranularityTracker::UnmarkPages(u64 offset, u64 size)
		{
			if (mPages == nullptr)
				return;

			const u32 startPage = (u32)(offset >> mPageShift);
			const u32 endPage = (u32)((offset + size - 1) >> mPageShift);

			B3D_ASSERT(mPages[startPage].LiveCount > 0);
			if (--mPages[startPage].LiveCount == 0)
				mPages[startPage].Category = PageCategory::Free;

			if (endPage != startPage)
			{
				B3D_ASSERT(mPages[endPage].LiveCount > 0);
				if (--mPages[endPage].LiveCount == 0)
					mPages[endPage].Category = PageCategory::Free;
			}
		}

		inline bool GranularityTracker::CheckAndAlignUp(u64& inOutOffset, u64 size, GpuResourceKind kind, u64 blockEnd) const
		{
			if (mPages == nullptr)
				return true;

			const PageCategory category = (PageCategory)kind;
			u32 startPage = (u32)(inOutOffset >> mPageShift);
			if (mPages[startPage].LiveCount > 0 && IsConflict(mPages[startPage].Category, category))
			{
				inOutOffset = (inOutOffset + mGranularity - 1) & ~(mGranularity - 1);
				if (inOutOffset + size > blockEnd)
					return false;

				startPage++;
			}

			const u32 endPage = (u32)((inOutOffset + size - 1) >> mPageShift);
			if (endPage != startPage && mPages[endPage].LiveCount > 0 && IsConflict(mPages[endPage].Category, category))
				return false;

			return true;
		}

#if B3D_DEBUG
		inline void GranularityTracker::AssertEmpty() const
		{
			if (mPages == nullptr)
				return;

			for (u32 pageIndex = 0; pageIndex < mPageCount; pageIndex++)
				B3D_ASSERT(mPages[pageIndex].LiveCount == 0);
		}
#endif

		//----------------------------------------------------------------------------------------
		// NodePool definitions
		//----------------------------------------------------------------------------------------

		inline u32 NodePool::Allocate()
		{
			if (mFreeHead != Utility::kInvalidIndex)
			{
				const u32 index = mFreeHead;
				mFreeHead = mNodes[index].NextFree;
				return index;
			}

			const u32 index = (u32)mNodes.size();
			mNodes.push_back(Node{});
			return index;
		}

		inline void NodePool::Release(u32 nodeIndex)
		{
			Node& node = mNodes[nodeIndex];
			node.Flags = NodeFlags{};
			node.NextFree = mFreeHead;
			mFreeHead = nodeIndex;
		}

		//----------------------------------------------------------------------------------------
		// Heap definitions
		//----------------------------------------------------------------------------------------

		template <typename HeapBackend>
		Heap<HeapBackend>::Heap(HeapHandle handle, u64 size, u64 granularity, u64 granularityDisableThreshold, u64 minAllocationSize)
			: mHandle(handle), mTotalSize(size), mFreeSize(size), mMinAllocationSize(minAllocationSize)
		{
			for (u32 listIndex = 0; listIndex < Utility::kFreeListCount; listIndex++)
				mFreeListHead[listIndex] = Utility::kInvalidIndex;

			for (u32 firstLevel = 0; firstLevel < Utility::kFirstLevelClassCount; firstLevel++)
				mSecondLevelFreeBitmask[firstLevel] = 0;

			mGranularity.Initialize(size, granularity, granularityDisableThreshold);

			// Trailing null block — covers the entire heap initially. Excluded from the free-list bitmaps;
			// FindFreeNode falls through to it after the bitmap walk fails.
			mNullNodeIndex = mPool.Allocate();
			Node& nullNode = mPool[mNullNodeIndex];
			nullNode.Offset = 0;
			nullNode.Size = size;
			nullNode.PrevPhysical = Utility::kInvalidIndex;
			nullNode.NextPhysical = Utility::kInvalidIndex;
			nullNode.PrevFree = Utility::kInvalidIndex;
			nullNode.NextFree = Utility::kInvalidIndex;
			nullNode.Flags = NodeFlags(NodeFlag::Free) | NodeFlag::NullNode;
			nullNode.Owner = nullptr;

			mPhysicalListHead = mNullNodeIndex;
		}

		template <typename HeapBackend>
		bool Heap<HeapBackend>::TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, u32& outNodeIndex)
		{
			// Cheap fast-fail: a heap whose total free size is less than the bare request can never fit.
			// Don't include alignment slack here — the natural-bucket walk in FindFreeNode rejects misaligned
			// candidates, and we don't want to skip a heap that has the bytes but might need alignment slack.
			if (mFreeSize < size)
				return false;

			u64 alignedOffset = 0;
			const u32 candidateNodeIndex = FindFreeNode(size, alignment, kind, alignedOffset);
			if (candidateNodeIndex == Utility::kInvalidIndex)
				return false;

			const u32 allocatedNodeIndex = CarveAllocation(candidateNodeIndex, alignedOffset, size);
			Node& allocated = mPool[allocatedNodeIndex];
			if (kind == GpuResourceKind::NonLinear)
				allocated.Flags |= NodeFlag::NonLinear;

			mGranularity.MarkPages(allocated.Offset, allocated.Size, kind);

			mFreeSize -= allocated.Size;
			mLiveAllocCount++;

			outNodeIndex = allocatedNodeIndex;
			return true;
		}

		template <typename HeapBackend>
		void Heap<HeapBackend>::FreeNode(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(!node.IsFree());

			mGranularity.UnmarkPages(node.Offset, node.Size);

			mFreeSize += node.Size;
			mLiveAllocCount--;
			node.Owner = nullptr;

			// Coalesce with the previous physical neighbor when it's free and not the null block.
			u32 mergedNodeIndex = nodeIndex;
			if (node.PrevPhysical != Utility::kInvalidIndex)
			{
				Node& previousNode = mPool[node.PrevPhysical];
				if (previousNode.IsFree() && !previousNode.IsNullNode())
				{
					RemoveFromFreeList(node.PrevPhysical);
					previousNode.Size += node.Size;
					previousNode.NextPhysical = node.NextPhysical;
					if (node.NextPhysical != Utility::kInvalidIndex)
						mPool[node.NextPhysical].PrevPhysical = node.PrevPhysical;

					mergedNodeIndex = node.PrevPhysical;
					mPool.Release(nodeIndex);
				}
			}

			// Coalesce with the next physical neighbor.
			Node& mergedNode = mPool[mergedNodeIndex];
			if (mergedNode.NextPhysical != Utility::kInvalidIndex)
			{
				const u32 nextNodeIndex = mergedNode.NextPhysical;
				Node& nextNode = mPool[nextNodeIndex];

				if (nextNode.IsNullNode())
				{
					// Fold our newly-freed range into the trailing null block. The merged node (if it isn't
					// the null block itself) is released back to the pool; the null block keeps its identity.
					nextNode.Offset = mergedNode.Offset;
					nextNode.Size += mergedNode.Size;
					nextNode.PrevPhysical = mergedNode.PrevPhysical;

					if (mergedNode.PrevPhysical != Utility::kInvalidIndex)
						mPool[mergedNode.PrevPhysical].NextPhysical = nextNodeIndex;
					else
						mPhysicalListHead = nextNodeIndex;

					mPool.Release(mergedNodeIndex);
					mergedNodeIndex = nextNodeIndex;
				}
				else if (nextNode.IsFree())
				{
					RemoveFromFreeList(nextNodeIndex);
					mergedNode.Size += nextNode.Size;
					mergedNode.NextPhysical = nextNode.NextPhysical;
					if (nextNode.NextPhysical != Utility::kInvalidIndex)
						mPool[nextNode.NextPhysical].PrevPhysical = mergedNodeIndex;

					mPool.Release(nextNodeIndex);
				}
			}

			// Insert the resulting node into its bucket. The null block does not participate in the free lists.
			Node& finalNode = mPool[mergedNodeIndex];
			finalNode.Flags |= NodeFlag::Free;
			if (!finalNode.IsNullNode())
				InsertIntoFreeList(mergedNodeIndex);

			if (mLiveAllocCount == 0)
			{
				B3D_DEBUG_ONLY(mGranularity.AssertEmpty());
			}
		}

		template <typename HeapBackend>
		void Heap<HeapBackend>::InsertIntoFreeList(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(node.Size, firstLevel, secondLevel);
			B3D_ASSERT(firstLevel < Utility::kFirstLevelClassCount);

			const u32 listIndex = Utility::GetListIndex(firstLevel, secondLevel);
			node.PrevFree = Utility::kInvalidIndex;
			node.NextFree = mFreeListHead[listIndex];
			if (mFreeListHead[listIndex] != Utility::kInvalidIndex)
				mPool[mFreeListHead[listIndex]].PrevFree = nodeIndex;
			mFreeListHead[listIndex] = nodeIndex;

			mSecondLevelFreeBitmask[firstLevel] |= (1u << secondLevel);
			mFirstLevelFreeBitmask |= (1u << firstLevel);
		}

		template <typename HeapBackend>
		void Heap<HeapBackend>::RemoveFromFreeList(u32 nodeIndex)
		{
			Node& node = mPool[nodeIndex];
			B3D_ASSERT(node.IsFree());
			B3D_ASSERT(!node.IsNullNode());

			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(node.Size, firstLevel, secondLevel);
			const u32 listIndex = Utility::GetListIndex(firstLevel, secondLevel);

			if (node.PrevFree != Utility::kInvalidIndex)
				mPool[node.PrevFree].NextFree = node.NextFree;
			else
				mFreeListHead[listIndex] = node.NextFree;

			if (node.NextFree != Utility::kInvalidIndex)
				mPool[node.NextFree].PrevFree = node.PrevFree;

			if (mFreeListHead[listIndex] == Utility::kInvalidIndex)
			{
				mSecondLevelFreeBitmask[firstLevel] &= ~(1u << secondLevel);

				if (mSecondLevelFreeBitmask[firstLevel] == 0)
					mFirstLevelFreeBitmask &= ~(1u << firstLevel);
			}
		}

		template <typename HeapBackend>
		u32 Heap<HeapBackend>::FindFreeNode(u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
		{
			u32 firstLevel = 0;
			u32 secondLevel = 0;
			Utility::SizeToBucket(size, firstLevel, secondLevel);

			// 1. Walk every non-empty bucket at-or-after (firstLevel, secondLevel). The natural bucket
			// (firstLevel, secondLevel) may or may not contain a fitting node depending on alignment +
			// granularity slack; strictly larger buckets would always fit absent that slack but a node
			// can still be rejected by it, so the same per-node check is applied throughout. Shifts use
			// 1ull to dodge UB at the firstLevel == 32 / secondLevel == kSecondLevelCount boundaries.
			// The second-level floor only applies on the natural first-level; higher first-levels walk
			// every set second-level bit (their nodes are strictly larger by construction).
			const u32 startFirstLevel = firstLevel;
			const u32 startSecondLevelFloor = (u32)(~((1ull << secondLevel) - 1ull));
			u32 firstLevelBitmask = mFirstLevelFreeBitmask & (u32)(~((1ull << startFirstLevel) - 1ull));

			while (firstLevelBitmask != 0)
			{
				const u32 chosenFirstLevel = (u32)Bitwise::LeastSignificantBit(firstLevelBitmask);
				const u32 secondLevelMask = (chosenFirstLevel == startFirstLevel) ? startSecondLevelFloor : ~0u;
				u32 secondLevelBitmask = mSecondLevelFreeBitmask[chosenFirstLevel] & secondLevelMask;
				while (secondLevelBitmask != 0)
				{
					const u32 chosenSecondLevel = (u32)Bitwise::LeastSignificantBit(secondLevelBitmask);
					const u32 listIndex = Utility::GetListIndex(chosenFirstLevel, chosenSecondLevel);
					const u32 candidateNodeIndex = WalkBucketForFit(listIndex, size, alignment, kind, outAlignedOffset);
					if (candidateNodeIndex != Utility::kInvalidIndex)
						return candidateNodeIndex;

					secondLevelBitmask &= ~(1u << chosenSecondLevel);
				}

				firstLevelBitmask &= ~(1u << chosenFirstLevel);
			}

			// 2. Fall back to the trailing null block. It is excluded from the bitmaps but is always free.
			if (mNullNodeIndex != Utility::kInvalidIndex)
			{
				const Node& nullBlock = mPool[mNullNodeIndex];
				u64 alignedOffset = Utility::AlignUp(nullBlock.Offset, alignment);
				if (mGranularity.CheckAndAlignUp(alignedOffset, size, kind, nullBlock.Offset + nullBlock.Size) && alignedOffset + size <= nullBlock.Offset + nullBlock.Size)
				{
					outAlignedOffset = alignedOffset;
					return mNullNodeIndex;
				}
			}

			return Utility::kInvalidIndex;
		}

		template <typename HeapBackend>
		u32 Heap<HeapBackend>::WalkBucketForFit(u32 listIndex, u64 size, u32 alignment, GpuResourceKind kind, u64& outAlignedOffset) const
		{
			u32 cursor = mFreeListHead[listIndex];
			while (cursor != Utility::kInvalidIndex)
			{
				const Node& node = mPool[cursor];
				u64 alignedOffset = Utility::AlignUp(node.Offset, alignment);

				// Buffer image granularity: adjust the offset if the start page holds a conflicting allocation. Reject the
				// candidate when the inflated range would overrun the block or end-page conflict can't be avoided.
				if (mGranularity.CheckAndAlignUp(alignedOffset, size, kind, node.Offset + node.Size) && alignedOffset + size <= node.Offset + node.Size)
				{
					outAlignedOffset = alignedOffset;
					return cursor;
				}

				cursor = node.NextFree;
			}
			return Utility::kInvalidIndex;
		}

		template <typename HeapBackend>
		u32 Heap<HeapBackend>::CarveAllocation(u32 candidateIndex, u64 alignedOffset, u64 size)
		{
			Node* candidateNode = &mPool[candidateIndex];
			B3D_ASSERT(candidateNode->IsFree());
			B3D_ASSERT(alignedOffset >= candidateNode->Offset);
			B3D_ASSERT(alignedOffset + size <= candidateNode->Offset + candidateNode->Size);

			const bool wasNullNode = candidateNode->IsNullNode();
			const u64 leadingPadding = alignedOffset - candidateNode->Offset;

			if (!wasNullNode)
				RemoveFromFreeList(candidateIndex);

			// Leading padding split. If the previous physical neighbor is free, fold the padding into it. Otherwise carve a fresh free node for it.
			if (leadingPadding > 0)
			{
				const u32 prevPhysicalIndex = candidateNode->PrevPhysical;
				if (prevPhysicalIndex != Utility::kInvalidIndex && mPool[prevPhysicalIndex].IsFree() && !mPool[prevPhysicalIndex].IsNullNode())
				{
					RemoveFromFreeList(prevPhysicalIndex);
					mPool[prevPhysicalIndex].Size += leadingPadding;
					InsertIntoFreeList(prevPhysicalIndex);
				}
				else
				{
					const u32 leadingPaddingNodeIndex = mPool.Allocate();
					candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

					Node& leadingPaddingNode = mPool[leadingPaddingNodeIndex];
					leadingPaddingNode.Offset = candidateNode->Offset;
					leadingPaddingNode.Size = leadingPadding;
					leadingPaddingNode.PrevPhysical = candidateNode->PrevPhysical;
					leadingPaddingNode.NextPhysical = candidateIndex;
					leadingPaddingNode.PrevFree = Utility::kInvalidIndex;
					leadingPaddingNode.NextFree = Utility::kInvalidIndex;
					leadingPaddingNode.Flags = NodeFlag::Free;

					if (candidateNode->PrevPhysical != Utility::kInvalidIndex)
						mPool[candidateNode->PrevPhysical].NextPhysical = leadingPaddingNodeIndex;
					else
						mPhysicalListHead = leadingPaddingNodeIndex;

					candidateNode->PrevPhysical = leadingPaddingNodeIndex;

					InsertIntoFreeList(leadingPaddingNodeIndex);
				}

				candidateNode->Offset = alignedOffset;
				candidateNode->Size -= leadingPadding;
			}

			// Trailing-remainder split. If the candidate is the null block, the remainder *becomes* the new
			// null block — we allocate a separate node for the carved-out front portion instead, so the heap
			// always retains a trailing null block.
			const u64 remainder = candidateNode->Size - size;

			u32 allocatedIndex;
			if (wasNullNode)
			{
				// Carve a new allocated node before the null block; shrink the null block to cover the rest.
				allocatedIndex = mPool.Allocate();
				candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

				Node& allocatedNode = mPool[allocatedIndex];
				allocatedNode.Offset = candidateNode->Offset;
				allocatedNode.Size = size;
				allocatedNode.PrevPhysical = candidateNode->PrevPhysical;
				allocatedNode.NextPhysical = candidateIndex;
				allocatedNode.PrevFree = Utility::kInvalidIndex;
				allocatedNode.NextFree = Utility::kInvalidIndex;
				allocatedNode.Flags = NodeFlags{}; // Not free, not null block.

				if (candidateNode->PrevPhysical != Utility::kInvalidIndex)
					mPool[candidateNode->PrevPhysical].NextPhysical = allocatedIndex;
				else
					mPhysicalListHead = allocatedIndex;

				candidateNode->PrevPhysical = allocatedIndex;

				candidateNode->Offset += size;
				candidateNode->Size -= size;
			}
			else
			{
				// Trailing remainder either splits off as a free node, or absorbs into the allocation if
				// it would be smaller than MinAllocationSize.
				if (remainder >= mMinAllocationSize)
				{
					const u32 trailingIndex = mPool.Allocate();
					candidateNode = &mPool[candidateIndex]; // Pool.Allocate may have invalidated references.

					Node& trailingNode = mPool[trailingIndex];
					trailingNode.Offset = candidateNode->Offset + size;
					trailingNode.Size = remainder;
					trailingNode.PrevPhysical = candidateIndex;
					trailingNode.NextPhysical = candidateNode->NextPhysical;
					trailingNode.PrevFree = Utility::kInvalidIndex;
					trailingNode.NextFree = Utility::kInvalidIndex;
					trailingNode.Flags = NodeFlag::Free;

					if (candidateNode->NextPhysical != Utility::kInvalidIndex)
						mPool[candidateNode->NextPhysical].PrevPhysical = trailingIndex;

					candidateNode->NextPhysical = trailingIndex;
					candidateNode->Size = size;

					InsertIntoFreeList(trailingIndex);
				}

				candidateNode->Flags = NodeFlags{}; // Not free, not null block.
				allocatedIndex = candidateIndex;
			}

			return allocatedIndex;
		}

	} // namespace detail::tlsf

	/**
	 * Two-Level Segregated Fit GPU memory allocator. O(1) bitmap-driven bucket lookup, leading-padding
	 * split for alignment, full coalescing on free, multi-heap growable. One allocator instance manages
	 * a list of backend heaps; allocations report back to the consumer via TGpuResourceLocation, with
	 * the heap index and pool node index stored in the location's two strategy-private slots.
	 *
	 * **Threading.** Single-threaded by contract —  Caller is responsible for external synchronization
	 * if the same instance is shared between threads.
	 *
	 * **Buffer-image granularity.** A single allocator instance can host mixed linear (buffer / linear image)
	 * and non-linear (optimally-tiled image) allocations safely; pass the appropriate GpuResourceKind to
	 * TryAllocate. The configured BufferImageGranularity  drives the mandatory padding between conflicting
	 * neighbors. When the configured granularity is at or below GranularityDisableThreshold the tracker
	 * is fully inert and adds zero per-allocation overhead.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 *
	 * @see TGpuAllocator
	 */
	template <typename HeapBackend>
	class TGpuTlsfAllocator : public TGpuAllocator<TGpuTlsfAllocator<HeapBackend>, HeapBackend>
	{
	public:
		using Base = TGpuAllocator<TGpuTlsfAllocator<HeapBackend>, HeapBackend>;
		using Location = typename Base::Location;
		using HeapHandle = typename HeapBackend::HeapHandle;

		/** Runtime configuration for the allocator. */
		struct Configuration
		{
			/** Size of the first heap created on demand. */
			u64 InitialHeapSize = 64ull * 1024 * 1024;

			/**
			 * Maximum size for any single heap. Single allocations larger than this are placed in dedicated heaps.
			 * Hard upper bound: 2^(kFirstLevelClassCount + kMemoryClassShift) bytes (~549 GB) — sizes beyond
			 * that exceed the bitmap's first-level class capacity.
			 */
			u64 MaxHeapSize = 256ull * 1024 * 1024;

			/** Each new heap is sized min(previousSize * GrowthFactor, MaxHeapSize). */
			u32 GrowthFactor = 2;

			/** Number of fully-empty heaps to retain as warm spares before destroying further empties. */
			u32 MaxEmptyHeapCount = 1;

			/** Allocations smaller than this are rounded up — keeps tiny allocations from over-fragmenting the small bucket. */
			u64 MinAllocationSize = 16;

			/**
			 * Buffer-image granularity in bytes (e.g. Vulkan VkPhysicalDeviceLimits). Linear and
			 * non-linear allocations sharing one heap are guaranteed to be separated by at least this
			 * many bytes. Default 1 disables buffer image granularity handling at zero cost.
			 * Must be a power of two when > 1.
			 */
			u64 BufferImageGranularity = 1;

			/**
			 * Skip the per-heap region table when BufferImageGranularity is at or below this threshold.
			 * At small granularities the natural alignment of typical buffers (>= 256 B for UBO/SSBO bindings)
			 * implicitly satisfies the constraint, so the tracker's memory cost is wasted. Set to 0 to track every granularity > 1.
			 */
			u64 GranularityDisableThreshold = 256;

			/**
			 * Controls how the allocator tracks when allocations are no longer used on the GPU, so it may safely
			 * free them.
			 */
			GpuAllocatorFreeDeferralMode DeferralMode = GpuAllocatorFreeDeferralMode::FrameTracker;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap on each grow. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		TGpuTlsfAllocator(HeapBackend* backend, IGpuFrameTracker* frameTracker, const Configuration& configuration);
		~TGpuTlsfAllocator();

		// Non-copyable — node pool and heap state are not safe to duplicate.
		TGpuTlsfAllocator(const TGpuTlsfAllocator&) = delete;
		TGpuTlsfAllocator& operator=(const TGpuTlsfAllocator&) = delete;

		/** @name TGpuAllocator CRTP surface.
		 *  @{
		 */

		bool TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out);
		void FreeImpl(Location& allocation);
		void FreeImmediateImpl(u32 heapIndex, u32 nodeIndex);

		/**
		 * Stamps an IGpuResource owner onto a previously-allocated slot. Used by callers that can't
		 * pass the owner at TryAllocate time (e.g. backends where the allocation is performed before
		 * the IGpuResource wrapper exists, then the wrapper registers itself post-construction).
		 * Pass nullptr to clear the owner — the slot remains live but becomes ineligible for defrag.
		 */
		void SetAllocationOwner(const Location& allocation, IGpuResource* owner);

		/** @} */

		/** @name Diagnostics.
		 *  @{
		 */

		/** Total number of bytes allocated by all underlying heaps. */
		u64 GetCommittedBytes() const;

		/** Total number of bytes currently held by live (non-retired, non-freed) allocations. */
		u64 GetUsedBytes() const;

		/** Number of populated heap slots (vacated slots are not counted). */
		u32 GetHeapCount() const;

		/** Number of fully-empty heaps currently retained as spares. */
		u32 GetEmptyHeapCount() const;

		/** @} */

		/** @name Defragmentation.
		 *  @{
		 */

		/** Per-call budget for Defrag. Soft caps that stop the walk early when exceeded. */
		struct DefragmentationInfo
		{
			/** Maximum number of bytes copied per call. 0 = unlimited. */
			u64 MaxBytesPerCall = 32ull * 1024 * 1024;

			/** Maximum number of moves per call. 0 = unlimited. */
			u32 MaxAllocationsPerCall = 0;
		};

		/** Counters reported by Defrag. */
		struct DefragmentationStats
		{
			/** Number of candidate slots that passed eligibility filtering and where a move was attempted. */
			u32 MovesAttempted = 0;

			/** Number of moves where a destination slot was successfully reserved. */
			u32 MovesCompleted = 0;

			/** Total bytes covered by completed moves. */
			u64 BytesMoved = 0;

			/** True if either of the DefragmentationInfo budgets aborted the walk early. */
			bool BudgetExhausted = false;
		};

		/**
		 * Compacts live allocations by moving them into lower-offset / lower-index slots. 
		 * Every tracked allocation (non-null owner) is a candidate.
		 *
		 * @param commandBuffer	Command buffer the consumer's recreate-and-copy path records into.
		 * @param info			Soft per-call budgets.
		 * @return				Counters for moves attempted / completed and budget-exhausted state.
		 */
		DefragmentationStats Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info = {});

		/** @} */

	private:
		using Heap = detail::tlsf::Heap<HeapBackend>;

		/** Destination slot reserved by TryAllocateInHeapsAtMost for a single defrag move. */
		struct DefragDestinationSlot
		{
			u32 HeapIndex = detail::tlsf::Utility::kInvalidIndex;
			u32 NodeIndex = detail::tlsf::Utility::kInvalidIndex;
			u64 Offset = 0;
		};

		/** (heap, node) pair tracked across a Defrag() pass for end-of-pass DefragDestination flag clear. */
		struct DefragDestinationKey
		{
			u32 HeapIndex;
			u32 NodeIndex;
		};

		/** Create a fresh heap and install it into mHeaps, reusing a vacated slot if one is available. */
		u32 CreateNewHeap(u64 sizeInBytes);

		/** Destroy heap @p heapIndex and vacate its slot. Caller has verified LiveAllocCount == 0. */
		void DestroyHeap(u32 heapIndex);

		/**
		 * Variant of TryAllocateImpl that limits heap iteration to indices @p maxHeapIndexInclusive
		 * and below, and never grows a fresh heap. Used during defrag so destinations can land in the
		 * same heap as the source (within-heap compaction) or any lower-index heap (drain), but never
		 * cause the allocator to expand committed memory.
		 */
		bool TryAllocateInHeapsAtMost(u64 size, u32 alignment, GpuResourceKind kind, u32 maxHeapIndexInclusive, DefragDestinationSlot& out);

		/**
		 * Reserves a destination slot for the live allocation at @p sourceNodeIndex, dispatches the
		 * consumer's MoveAllocation, and (under FreeDeferralMode::FrameTracker) retires the source 
		 * allocation against the current frame index. Under FreeDeferralMode::ResourceLifecycle the 
		 * source slot is left untracked and freed by the consumer's destructor. Returns true on a 
		 * successful move and writes the chosen destination heap and node indices to the out 
		 * parameters; false if no destination slot was available within @p sourceHeapIndex inclusive.
		 */
		bool TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u32& outDestinationHeapIndex, u32& outDestinationNodeIndex);

		Configuration mConfig;
		Vector<Heap*> mHeaps;
		u32 mEmptyHeapCount = 0;
		u64 mNextHeapSize = 0;
	};

	template <typename HeapBackend>
	TGpuTlsfAllocator<HeapBackend>::TGpuTlsfAllocator(HeapBackend* backend, IGpuFrameTracker* frameTracker, const Configuration& configuration)
		: Base(backend, frameTracker), mConfig(configuration), mNextHeapSize(configuration.InitialHeapSize)
	{
		B3D_ASSERT(mConfig.GrowthFactor >= 1);
		B3D_ASSERT(mConfig.InitialHeapSize > 0);
		B3D_ASSERT(mConfig.MaxHeapSize >= mConfig.InitialHeapSize);
		B3D_ASSERT(mConfig.MinAllocationSize > 0);
		B3D_ASSERT(mConfig.BufferImageGranularity == 1 || Bitwise::IsPow2(mConfig.BufferImageGranularity));
		// Guards the bitmap-width constraint — sizes whose MSB exceeds this cap can't be bucketed.
		B3D_ASSERT(mConfig.MaxHeapSize < (1ull << (detail::tlsf::Utility::kFirstLevelClassCount + detail::tlsf::Utility::kMemoryClassShift)));
		B3D_ASSERT((mConfig.DeferralMode != GpuAllocatorFreeDeferralMode::FrameTracker) || frameTracker != nullptr);
	}

	template <typename HeapBackend>
	TGpuTlsfAllocator<HeapBackend>::~TGpuTlsfAllocator()
	{
		// Drain unconditionally — any submissions still in flight at destructor time are the caller's
		// responsibility to wait for via WaitUntilIdle, matching the convention from TGpuAllocator.
		Base::Flush(0, true);

		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] != nullptr)
			{
				Base::mBackend->DestroyHeap(mHeaps[heapIndex]->Handle());
				B3DDelete(mHeaps[heapIndex]);
				mHeaps[heapIndex] = nullptr;
			}
		}
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateImpl(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, Location& out)
	{
		B3D_ASSERT(out.Allocator == nullptr);
		B3D_ASSERT(alignment > 0);
		B3D_ASSERT(Bitwise::IsPow2(alignment));

		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Try existing heaps oldest-first so empty-spare slots drain before any new heap is created.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			Heap* heap = mHeaps[heapIndex];
			if (heap == nullptr)
				continue;

			const bool heapWasEmpty = (heap->LiveAllocCount() == 0);
			u32 nodeIndex = detail::tlsf::Utility::kInvalidIndex;
			if (!heap->TryAllocate(requestedSize, alignment, kind, nodeIndex))
				continue;

			if (heapWasEmpty && mEmptyHeapCount > 0)
				mEmptyHeapCount--;

			heap->SetNodeOwner(nodeIndex, owner);
			const detail::tlsf::Node& allocatedNode = heap->GetNode(nodeIndex);
			out.Heap = heap->Handle();
			out.Offset = allocatedNode.Offset;
			out.Size = allocatedNode.Size;
			out.Allocator = this;
			out.AllocatorData0 = heapIndex;
			out.AllocatorData1 = nodeIndex;
			return true;
		}

		// All existing heaps full — grow.
		const u64 newHeapSize = std::max(requestedSize, mNextHeapSize);
		const u32 newHeapIndex = CreateNewHeap(newHeapSize);
		if (newHeapIndex == detail::tlsf::Utility::kInvalidIndex)
			return false;

		Heap* fresh = mHeaps[newHeapIndex];
		u32 nodeIndex = detail::tlsf::Utility::kInvalidIndex;
		const bool ok = fresh->TryAllocate(requestedSize, alignment, kind, nodeIndex);
		B3D_ASSERT(ok); // A fresh heap big enough for the request must satisfy it.

		// Fresh heap was empty by construction; charge the empty-spare counter for the transition.
		if (mEmptyHeapCount > 0)
			mEmptyHeapCount--;

		fresh->SetNodeOwner(nodeIndex, owner);
		const detail::tlsf::Node& allocatedNode = fresh->GetNode(nodeIndex);
		out.Heap = fresh->Handle();
		out.Offset = allocatedNode.Offset;
		out.Size = allocatedNode.Size;
		out.Allocator = this;
		out.AllocatorData0 = newHeapIndex;
		out.AllocatorData1 = nodeIndex;

		return ok;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::FreeImpl(Location& allocation)
	{
		B3D_ASSERT(allocation.Allocator == this);
		B3D_ASSERT(allocation.AllocatorData0 < (u32)mHeaps.size());

		if (mConfig.DeferralMode == GpuAllocatorFreeDeferralMode::ResourceLifecycle)
		{
			// Caller has gated GPU completion through IGpuResource::Destroy + Notify* — no need to
			// queue the slot. Release synchronously so a subsequent allocation can reuse it.
			FreeImmediateImpl(allocation.AllocatorData0, allocation.AllocatorData1);
			return;
		}

		Base::RetireAllocation(allocation);
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::FreeImmediateImpl(u32 heapIndex, u32 nodeIndex)
	{
		B3D_ASSERT(heapIndex < (u32)mHeaps.size());
		Heap* heap = mHeaps[heapIndex];
		B3D_ASSERT(heap != nullptr);

		heap->FreeNode(nodeIndex);

		// Release a fully-empty heap if we're already over the spare budget.
		if (heap->LiveAllocCount() == 0)
		{
			if (mEmptyHeapCount < mConfig.MaxEmptyHeapCount)
				mEmptyHeapCount++;
			else
				DestroyHeap(heapIndex);
		}
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::SetAllocationOwner(const Location& allocation, IGpuResource* owner)
	{
		B3D_ASSERT(allocation.Allocator == this);
		B3D_ASSERT(allocation.AllocatorData0 < (u32)mHeaps.size());
		Heap* heap = mHeaps[allocation.AllocatorData0];
		B3D_ASSERT(heap != nullptr);
		heap->SetNodeOwner(allocation.AllocatorData1, owner);
	}

	template <typename HeapBackend>
	u64 TGpuTlsfAllocator<HeapBackend>::GetCommittedBytes() const
	{
		u64 total = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				total += heap->TotalSize();
		}

		return total;
	}

	template <typename HeapBackend>
	u64 TGpuTlsfAllocator<HeapBackend>::GetUsedBytes() const
	{
		u64 used = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				used += heap->TotalSize() - heap->FreeSize();
		}

		return used;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::GetHeapCount() const
	{
		u32 count = 0;
		for (Heap* heap : mHeaps)
		{
			if (heap != nullptr)
				count++;
		}

		return count;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::GetEmptyHeapCount() const
	{
		return mEmptyHeapCount;
	}

	template <typename HeapBackend>
	u32 TGpuTlsfAllocator<HeapBackend>::CreateNewHeap(u64 sizeInBytes)
	{
		const HeapHandle handle = Base::mBackend->CreateHeap(sizeInBytes, mConfig.HeapCreateInfo);

		Heap* heap = B3DNew<Heap>(handle, sizeInBytes,
			mConfig.BufferImageGranularity, mConfig.GranularityDisableThreshold, mConfig.MinAllocationSize);

		// Empty heap counts as a "spare" against the warm-spare budget the moment it's created — it
		// already has zero live allocations.
		mEmptyHeapCount++;
		mNextHeapSize = std::min(mNextHeapSize * mConfig.GrowthFactor, mConfig.MaxHeapSize);

		// Reuse a vacated slot if one exists; otherwise grow the vector.
		for (u32 heapIndex = 0; heapIndex < (u32)mHeaps.size(); heapIndex++)
		{
			if (mHeaps[heapIndex] == nullptr)
			{
				mHeaps[heapIndex] = heap;
				return heapIndex;
			}
		}

		const u32 newIndex = (u32)mHeaps.size();
		mHeaps.push_back(heap);
		return newIndex;
	}

	template <typename HeapBackend>
	void TGpuTlsfAllocator<HeapBackend>::DestroyHeap(u32 heapIndex)
	{
		Heap* heap = mHeaps[heapIndex];
		B3D_ASSERT(heap != nullptr);
		B3D_ASSERT(heap->LiveAllocCount() == 0);

		Base::mBackend->DestroyHeap(heap->Handle());
		B3DDelete(heap);
		mHeaps[heapIndex] = nullptr;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryAllocateInHeapsAtMost(u64 size, u32 alignment, GpuResourceKind kind, u32 maxHeapIndexInclusive, DefragDestinationSlot& out)
	{
		const u64 requestedSize = std::max(size, mConfig.MinAllocationSize);

		// Walk heaps oldest-first (matches TryAllocateImpl's order) but bounded — defragmentation
		// must never grow committed memory.
		const u32 maxIndex = std::min(maxHeapIndexInclusive, (u32)mHeaps.size() - 1);
		for (u32 heapIndex = 0; heapIndex <= maxIndex; heapIndex++)
		{
			Heap* heap = mHeaps[heapIndex];
			if (heap == nullptr)
				continue;

			const bool heapWasEmpty = (heap->LiveAllocCount() == 0);
			u32 allocatedNodeIndex = detail::tlsf::Utility::kInvalidIndex;
			if (!heap->TryAllocate(requestedSize, alignment, kind, allocatedNodeIndex))
				continue;

			if (heapWasEmpty && mEmptyHeapCount > 0)
				mEmptyHeapCount--;

			// Owner is set by the caller (TryMoveAllocation) after this returns.

			out.HeapIndex = heapIndex;
			out.NodeIndex = allocatedNodeIndex;
			out.Offset = heap->GetNode(allocatedNodeIndex).Offset;
			return true;
		}

		return false;
	}

	template <typename HeapBackend>
	bool TGpuTlsfAllocator<HeapBackend>::TryMoveAllocation(u32 sourceNodeIndex, u32 sourceHeapIndex, render::GpuCommandBuffer& commandBuffer, u32& outDestinationHeapIndex, u32& outDestinationNodeIndex)
	{
		Heap* sourceHeap = mHeaps[sourceHeapIndex];

		// Snapshot source state before TryAllocateInHeapsAtMost — within-heap CarveAllocation may
		// push_back onto the source heap's pool, which would invalidate any held Node& references.
		const detail::tlsf::Node& sourceSnapshotRef = sourceHeap->GetNode(sourceNodeIndex);
		const u64 sourceOffset = sourceSnapshotRef.Offset;
		const u64 sourceSize = sourceSnapshotRef.Size;
		IGpuResource* owner = sourceSnapshotRef.Owner;
		const GpuResourceKind sourceKind = sourceSnapshotRef.Flags.IsSet(detail::tlsf::NodeFlag::NonLinear) ? GpuResourceKind::NonLinear : GpuResourceKind::Linear;

		// 1. Reserve a destination slot in the same heap or any lower-index heap
		DefragDestinationSlot destination;
		if (!TryAllocateInHeapsAtMost(sourceSize, /*alignment=*/1u, sourceKind, sourceHeapIndex, destination))
			return false;

		// 2. Ensure destination is at a lower offset than the source
		if (destination.HeapIndex == sourceHeapIndex && destination.Offset >= sourceOffset)
		{
			FreeImmediateImpl(destination.HeapIndex, destination.NodeIndex);
			return false;
		}

		Heap* destHeap = mHeaps[destination.HeapIndex];

		// 3. Build the destination Location
		Location newLocation;
		newLocation.Heap = destHeap->Handle();
		newLocation.Offset = destination.Offset;
		newLocation.Size = sourceSize;
		newLocation.Allocator = this;
		newLocation.AllocatorData0 = destination.HeapIndex;
		newLocation.AllocatorData1 = destination.NodeIndex;

		// 4. Notify the owner: depending on DeferralMode it will either re-allocate a brand new IGpuResource
		//    at the destination location (if resource tracking is used), or patch the existing
		//    resource (if frame tracking is used).
		IGpuResource* newOwner = owner->MoveAllocation(commandBuffer, newLocation);

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
		if (mConfig.DeferralMode == GpuAllocatorFreeDeferralMode::FrameTracker)
		{
			B3D_ASSERT(newOwner == owner &&
				"FreeDeferralMode::FrameTracker requires MoveAllocation to return the same IGpuResource it was called on. "
				"Wrapper-swap patterns require FreeDeferralMode::ResourceLifecycle.");

			Location sourceSnapshot;
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

	template <typename HeapBackend>
	typename TGpuTlsfAllocator<HeapBackend>::DefragmentationStats
	TGpuTlsfAllocator<HeapBackend>::Defrag(render::GpuCommandBuffer& commandBuffer, const DefragmentationInfo& info)
	{
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
		for (i32 outerIndex = (i32)mHeaps.size() - 1; outerIndex >= 0; outerIndex--)
		{
			const u32 heapIndex = (u32)outerIndex;
			if (mHeaps[heapIndex] == nullptr)
				continue;

			Heap& heap = *mHeaps[heapIndex];
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
				IGpuResource* owner = node.Owner;
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
					mHeaps[destinationHeapIndex]->SetDefragDestinationFlag(destinationNodeIndex);
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
			mHeaps[key.HeapIndex]->ClearDefragDestinationFlag(key.NodeIndex);

		return stats;
	}

	/** @} */
} // namespace b3d
