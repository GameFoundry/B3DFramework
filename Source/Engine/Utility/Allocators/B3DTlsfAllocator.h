//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DFlags.h"

namespace b3d
{
	/** @addtogroup Memory-Internal
	 *  @{
	 */

	/**
	 * Category of a TLSF allocation, used for granularity conflict tracking. Two allocations of differing kinds are
	 * never placed within the same granularity page (see TTlsfAllocator::Configuration::Granularity). Callers that
	 * don't need conflict tracking pass Linear for everything.
	 */
	enum class TlsfAllocationKind : u8
	{
		Linear = 0, /**< Plain memory (e.g. buffers, linearly-tiled images). */
		NonLinear = 1, /**< Memory with a conflicting layout (e.g. optimally-tiled images). */
	};

	/**
	 * Internal types and helpers for the Two-Level Segregated Fit allocator. Not part of the public
	 * surface — referenced only inside TTlsfAllocator's / TGpuTlsfAllocator's implementations.
	 */
	namespace detail::tlsf
	{
		//----------------------------------------------------------------------------------------
		// Declarations
		//----------------------------------------------------------------------------------------

		/**
		 * Helper used by the TLSF allocator to ensure that non-linear allocations are placed at correct alignment
		 * (granularity). Some GPU backends have different alignment requirements if a non-linear image follows or
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
			void MarkPages(u64 offset, u64 size, TlsfAllocationKind kind);

			/** Decrements the refcounts for the start + end pages; resets category to Free at zero. */
			void UnmarkPages(u64 offset, u64 size);

			/**
			 * Adjust @p inOutOffset upward to clear any granularity conflict at the start page;
			 * return false if the adjusted range overruns @p blockEnd or the end page holds
			 * a conflicting allocation. Returns true (no-op) when the tracker is inert.
			 */
			bool CheckAndAlignUp(u64& inOutOffset, u64 size, TlsfAllocationKind kind, u64 blockEnd) const;

#if B3D_DEBUG
			/** Asserts every page has zero LiveCount — sanity check when a heap goes empty. */
			void AssertEmpty() const;
#endif

		private:
			/** Allocation-kind category stored per granularity page. */
			enum class PageCategory : u8
			{
				Linear = (u8)TlsfAllocationKind::Linear,
				NonLinear = (u8)TlsfAllocationKind::NonLinear,
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
		 * Compile-time constants and pure helper functions shared between Heap and the orchestrators.
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

			/** Round @p value up to the next multiple of a non-zero @p alignment. */
			inline u64 AlignUp(u64 value, u32 alignment)
			{
				const u64 remainder = value % alignment;
				return remainder == 0 ? value : value + alignment - remainder;
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
		 * identity fits in a single 32-bit slot of the orchestrator's allocation record.
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

			/**
			 * Opaque owner tag stamped by the orchestrator (e.g. the owning resource for GPU defragmentation).
			 * Never dereferenced by the heap — only stored, cleared and handed back. nullptr when the slot is
			 * untracked or free.
			 */
			void* Owner;

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
		 * orchestration (heap pool, defragmentation, empty-spare bookkeeping) lives in the
		 * orchestrators (TTlsfAllocator / TGpuTlsfAllocator).
		 *
		 * @tparam HeapHandleType	Backend-defined handle identifying the underlying heap memory. The heap never
		 *							operates on the handle — it only stores it for the orchestrator to read back.
		 */
		template <typename HeapHandleType>
		class Heap
		{
		public:
			using HeapHandle = HeapHandleType;

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
			 * for stamping Owner and building the public allocation record.
			 */
			bool TryAllocate(u64 size, u32 alignment, TlsfAllocationKind kind, u32& outNodeIndex);

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
			void SetNodeOwner(u32 nodeIndex, void* owner) { mPool[nodeIndex].Owner = owner; }

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

			GranularityTracker mGranularity; /**< Granularity tracker — inert when the allocator is configured with granularity <= 1 or below the threshold. */
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
			u32 FindFreeNode(u64 size, u32 alignment, TlsfAllocationKind kind, u64& outAlignedOffset) const;

			/**
			 * Walk a bucket's free list and return the first node large enough to satisfy (size, alignment, kind).
			 * The returned @p outAlignedOffset contains any buffer image granularity past natural alignment.
			 */
			u32 WalkBucketForFit(u32 listIndex, u64 size, u32 alignment, TlsfAllocationKind kind, u64& outAlignedOffset) const;

			/**
			 * Carve a @p size byte allocation starting at @p alignedOffset out of the candidate node, splitting
			 * any leading padding and trailing remainder into separate free nodes. Returns the node-index of the allocated block.
			 */
			u32 CarveAllocation(u32 candidateIndex, u64 alignedOffset, u64 size);
		};
	} // namespace detail::tlsf

	/**
	 * General-purpose Two-Level Segregated Fit memory allocator. O(1) bitmap-driven bucket lookup,
	 * leading-padding split for alignment, full coalescing on free, multi-heap growable. One allocator
	 * instance manages a list of backend heaps; the backend decides what a heap actually is (a malloc'd
	 * block, a mapped memory region, ...) — the allocator only sub-divides the byte ranges.
	 *
	 * **Threading.** When ThreadPolicy is ThreadSafe (the default), every public entry point acquires an
	 * allocator-wide mutex. When ThreadPolicy is ThreadUnsafe, the locking compiles out and the caller is
	 * responsible for external synchronization.
	 *
	 * @tparam HeapBackend	Backend providing:
	 * 						 - typedef `HeapHandle` — copyable heap identifier, comparable to nullptr,
	 * 						   value-initialized state meaning "no heap".
	 * 						 - typedef `HeapCreateInformation` — extra creation parameters, forwarded verbatim.
	 * 						 - `HeapHandle CreateHeap(u64 size, const HeapCreateInformation&)` — returns nullptr on failure.
	 * 						 - `void DestroyHeap(HeapHandle)`.
	 * @tparam ThreadPolicy	Compile-time thread-safety policy. ThreadSafe (default) wraps state with a
	 * 						mutex; ThreadUnsafe compiles out all locking.
	 */
	template <typename HeapBackend, ThreadSafetyPolicy ThreadPolicy = ThreadSafe>
	class TTlsfAllocator
	{
	public:
		using HeapHandle = typename HeapBackend::HeapHandle;
		using Heap = detail::tlsf::Heap<HeapHandle>;

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
			 * Granularity in bytes separating allocations of conflicting TlsfAllocationKind (e.g. Vulkan
			 * buffer-image granularity). Default 1 disables granularity handling at zero cost. Must be a
			 * power of two when > 1. Irrelevant when every allocation uses the same kind.
			 */
			u64 Granularity = 1;

			/**
			 * Skip the per-heap region table when Granularity is at or below this threshold. At small
			 * granularities the natural alignment of typical allocations implicitly satisfies the
			 * constraint, so the tracker's memory cost is wasted. Set to 0 to track every granularity > 1.
			 */
			u64 GranularityDisableThreshold = 256;

			/** Backend create-info forwarded verbatim to HeapBackend::CreateHeap on each grow. */
			typename HeapBackend::HeapCreateInformation HeapCreateInfo{};
		};

		/** Record describing a live allocation. Returned by TryAllocate; required to Free. */
		struct Allocation
		{
			HeapHandle Heap{}; /**< Backend handle of the heap the allocation lives in. */
			u64 Offset = 0; /**< Byte offset of the allocation within the heap. */
			u64 Size = 0; /**< Allocated size in bytes (>= the requested size after MinAllocationSize rounding). */
			u32 HeapIndex = detail::tlsf::Utility::kInvalidIndex; /**< Allocator-internal heap slot. */
			u32 NodeIndex = detail::tlsf::Utility::kInvalidIndex; /**< Allocator-internal node identity within the heap. */

			bool IsValid() const { return HeapIndex != detail::tlsf::Utility::kInvalidIndex; }
		};

		TTlsfAllocator(HeapBackend* backend, const Configuration& configuration);
		~TTlsfAllocator();

		// Non-copyable — node pool and heap state are not safe to duplicate.
		TTlsfAllocator(const TTlsfAllocator&) = delete;
		TTlsfAllocator& operator=(const TTlsfAllocator&) = delete;

		/**
		 * Allocates @p size bytes at @p alignment (power of two), growing a new backend heap if no existing
		 * heap can satisfy the request. Returns false when the backend fails to provide a new heap. @p kind
		 * only matters when Configuration::Granularity is active — see TlsfAllocationKind.
		 */
		bool TryAllocate(u64 size, u32 alignment, Allocation& out, TlsfAllocationKind kind = TlsfAllocationKind::Linear);

		/**
		 * Releases a previously-allocated slot immediately. Fully-empty heaps beyond the warm-spare budget
		 * are returned to the backend.
		 */
		void Free(const Allocation& allocation);

		/** @name Diagnostics.
		 *  @{
		 */

		/** Total number of bytes allocated by all underlying heaps. */
		u64 GetCommittedBytes() const;

		/** Total number of bytes currently held by live allocations. */
		u64 GetUsedBytes() const;

		/** Number of populated heap slots (vacated slots are not counted). */
		u32 GetHeapCount() const;

		/** Number of fully-empty heaps currently retained as spares. */
		u32 GetEmptyHeapCount() const;

		/** @} */

		/** @name Orchestration surface.
		 *
		 *  Access points for wrappers that layer additional tracking on top of the allocator (e.g.
		 *  TGpuTlsfAllocator's defragmentation): owner stamping, bounded placement and raw heap access.
		 *  Not needed for plain allocate/free use. Raw heap access is not covered by the internal lock —
		 *  wrappers are expected to use the ThreadUnsafe policy under an external mutex of their own.
		 *  @{
		 */

		/**
		 * Stamps an opaque owner tag onto a previously-allocated slot. The allocator never dereferences
		 * the owner — it is only stored and handed back through Heap::GetNode during orchestrator walks.
		 * Pass nullptr to clear the owner; freshly-allocated slots start untracked.
		 */
		void SetAllocationOwner(const Allocation& allocation, void* owner);

		/**
		 * Variant of TryAllocate that only considers heap slots at index @p maxHeapIndexInclusive and
		 * below, and never grows a fresh heap. Lets defragmentation place moves into the source heap or
		 * any older heap without ever expanding committed memory.
		 */
		bool TryAllocateInHeapsAtMost(u64 size, u32 alignment, u32 maxHeapIndexInclusive, Allocation& out, TlsfAllocationKind kind = TlsfAllocationKind::Linear);

		/** Number of heap slots, including vacated ones (GetHeapSlot returns nullptr for those). */
		u32 GetHeapSlotCount() const { return (u32)mHeaps.size(); }

		/** Direct access to the heap in slot @p heapIndex, or nullptr when the slot is vacated. */
		Heap* GetHeapSlot(u32 heapIndex) { return mHeaps[heapIndex]; }

		/** @} */

	private:
		static constexpr bool kThreadSafe = (ThreadPolicy == ThreadSafe);

		/** Create a fresh heap and install it into mHeaps, reusing a vacated slot if one is available. */
		u32 CreateNewHeap(u64 sizeInBytes);

		/** Destroy heap @p heapIndex and vacate its slot. Caller has verified LiveAllocCount == 0. */
		void DestroyHeap(u32 heapIndex);

		/**
		 * Attempt an allocation from the heap in slot @p heapIndex (false when the slot is vacated or the
		 * heap can't fit the request). Handles the empty-spare bookkeeping and fills @p out on success.
		 * @p requestedSize has already been rounded up to MinAllocationSize. Caller holds the lock.
		 */
		bool TryAllocateFromHeap(u32 heapIndex, u64 requestedSize, u32 alignment, TlsfAllocationKind kind, Allocation& out);

		HeapBackend* mBackend = nullptr;
		Configuration mConfig;
		Vector<Heap*> mHeaps;
		u32 mEmptyHeapCount = 0;
		u64 mNextHeapSize = 0;

		mutable LockingPolicy<kThreadSafe> mLockPolicy;
	};


	/** @} */
} // namespace b3d

#include "Allocators/B3DTlsfAllocator.inl"
