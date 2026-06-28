//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"

namespace b3d
{
	class GpuResourceManager;
	class IGpuResource;
	struct GpuResourceLocation;
	namespace render { class GpuCommandBuffer; }

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Memory-layout category of a GPU allocation. Some APIs require different allocation granularity when
	 * linear and non-linear entries overlap (i.e. buffer image granularity), and this is used by the allocator
	 * to respect that.
	 */
	enum class GpuResourceKind : u8
	{
		Linear		= 0,
		NonLinear	= 1
	};

	/**
	 * Opaque, backend-owned GPU memory heap.
	 *
	 * The destructor is protected and non-virtual: heaps are never deleted through an @c IGpuHeap* (the
	 * backend always destroys the concrete type), so no vtable is introduced and the struct stays a
	 * trivial tag.
	 */
	struct IGpuHeap
	{
	protected:
		IGpuHeap() = default;
		~IGpuHeap() = default;
	};

	/**
	 * Backend-agnostic GPU memory allocator interface. The CRTP TGpuAllocator family implements this.
	 */
	class IGpuAllocator
	{
	public:
		virtual ~IGpuAllocator() = default;

		/**
		 * Attempts to allocate @p size bytes with @p alignment, tagged with @p kind so the strategy can
		 * honor buffer-image granularity, and optionally registering @p owner for defragmentation
		 * callbacks (pass nullptr for an untracked allocation). On success populates @p out — including
		 * stamping @p out.Allocator with this allocator — and returns true.
		 */
		virtual bool TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuResourceLocation& out) = 0;

		/** Retires @p allocation (deferred free per the allocator's policy) and resets it to the empty state. */
		virtual void Free(GpuResourceLocation& allocation) = 0;

		/** Releases @p allocation immediately, bypassing any deferred-free queue, and resets it to the empty state. */
		virtual void FreeAndReclaim(GpuResourceLocation& allocation) = 0;

		/**
		 * Releases every retired allocation whose completion marker has signaled (per
		 * IGpuCompletionTracker::IsMarkerComplete), returning that memory to the allocator's pool.
		 * @p forceReclaimAll drains unconditionally and must only be used at teardown after the GPU is
		 * known idle.
		 */
		virtual void ReclaimUnused(bool forceReclaimAll = false) = 0;

		/**
		 * Frees every live allocation in one shot, resetting the allocator to its empty state. Only the
		 * linear/bump allocator supports this — it recycles memory by the page, so retiring its open
		 * pages frees everything at once. Every other strategy frees per allocation, so the default
		 * implementation is an error.
		 */
		virtual void FreeAll()
		{
			B3D_ENSURE_LOG(false, "FreeAll is only supported by the linear/bump allocator.");
		}

		/**
		 * True when the allocator tracks per-allocation owners and can relocate its allocations during
		 * defragmentation. Consumers should only register an allocation owner (for defragmentation
		 * callbacks) with allocators that return true. Default is false; strategies that support
		 * defragmentation override this.
		 */
		virtual bool SupportsDefragmentation() const { return false; }

#if B3D_DEBUG
		/**
		 * Number of live allocations produced by this allocator that have not yet been freed. Debug-only
		 * diagnostic, used to catch allocations that outlive their allocator (e.g. a transient buffer
		 * outliving its GpuWorkContext).
		 */
		virtual u64 GetOutstandingAllocationCount() const { return 0; }
#endif

	protected:
		IGpuAllocator() = default;
	};

	/**
	 * GPU memory allocation as returned by a GPU memory allocator. Used for freeing the allocation, as well
	 * as referencing the underlying memory. Each consumer owns their location and is the sole writer; the
	 * allocator only writes to the consumer's location once during the initial TryAllocate, and then
	 * supplies a fresh replacement location to IGpuResource::MoveAllocation when defragmentation
	 * moves the allocation.
	 *
	 * Must stay standard-layout and trivially-copyable.
	 */
	struct GpuResourceLocation
	{
		IGpuHeap* Heap = nullptr;
		u64 Offset = 0;
		u64 Size = 0;

		/** Allocator that produced this allocation; used to free or relocate it. Stamped at TryAllocate. */
		IGpuAllocator* Allocator = nullptr;

		// Strategy-private bookkeeping. Interpretation is private to the owning allocator.
		u32 AllocatorData0 = 0;
		u32 AllocatorData1 = 0;

		/** Returns true if the location currently refers to a live allocation owned by some allocator. */
		bool IsValid() const
		{
			return Allocator != nullptr;
		}

		/** Resets the location to the empty state. */
		void Reset()
		{
			Heap = nullptr;
			Offset = 0;
			Size = 0;
			Allocator = nullptr;
			AllocatorData0 = 0;
			AllocatorData1 = 0;
		}
	};

	/** @} */

	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Common base for backend GPU resources (VulkanResource, D3D12Resource, MetalResource, NullResource).
	 * Provides the cross-backend portion of the lifetime state machine — aggregate bound/in-use counters,
	 * deferred destruction, and the relocation hook used by allocators during defragmentation.
	 *
	 * @par Lifecycle
	 *
	 * Backends call the Notify* methods on their resources at the appropriate command-buffer lifecycle points:
	 *   - NotifyBound  — resource recorded into a command buffer (not yet submitted)
	 *   - NotifyUsed   — command buffer submitted to a GPU queue
	 *   - NotifyDone   — GPU finished executing the command buffer
	 *   - NotifyUnbound — command buffer destroyed/reset before submission
	 *
	 * The framework does not drive these calls itself; per-command-buffer tracking is each backend's
	 * concern. The framework only guarantees that, once Destroy() has been called and the bound count
	 * eventually drops to zero, OnWillDestroy() fires exactly once and the manager frees the resource.
	 *
	 * @par Threading
	 *
	 * Notify* may be called from queue-submission threads. Deferred-destroy fires on the thread that
	 * decrements the bound count to zero. Callers do not need to take external locks.
	 *
	 * @par Construction
	 *
	 * Resources must be created via GpuResourceManager::Create<T> so that allocation and free are
	 * symmetric.
	 */
	class B3D_EXPORT IGpuResource
	{
	public:
		/**
		 * Constructs a manager-owned resource.
		 *
		 * @param	owner	Manager responsible for freeing this resource. Must be non-null.
		 * @param	name	Optional debug name.
		 */
		IGpuResource(GpuResourceManager* owner, const StringView& name);

		virtual ~IGpuResource();

	protected:
		/**
		 * Constructs an unmanaged resource (no owner). Reserved for subclasses that take responsibility
		 * for their own lifetime — primarily test mocks. Production resources must use the manager-owned
		 * constructor above so that allocation and free remain symmetric.
		 */
		IGpuResource() = default;

	public:

		/** Sets a debug name. Stored only in development builds. */
		void SetDebugName(const StringView& name)
		{
#if B3D_BUILD_TYPE_DEVELOPMENT
			mDebugName = name;
#endif
		}

		/**
		 * Notifies the resource that it is currently bound to a command buffer. Buffer hasn't yet been submitted so the
		 * resource isn't being used on the GPU yet. Must eventually be followed by a NotifyUsed() or NotifyUnbound().
		 */
		void NotifyBound();

		/**
		 * Notifies the resource that it is currently being used on a submitted command buffer. Must follow a
		 * NotifyBound(). Must eventually be followed by a NotifyDone().
		 *
		 * @param	queueId		ID of the queue the resource is being used in.
		 * @param	useFlags	Flags that determine in what way is the resource being used.
		 */
		void NotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags);

		/**
		 * Notifies the resource that it is no longer being used on the GPU. Must follow a NotifyUsed().
		 *
		 * @param	queueId		ID of the queue the resource was being used in.
		 * @param	useFlags	Use flags that specify how was the resource being used.
		 */
		void NotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags);

		/**
		 * Notifies the resource that it is no longer queued on the command buffer without ever being submitted to the GPU.
		 * Must follow a NotifyBound() if NotifyUsed() wasn't called.
		 */
		void NotifyUnbound();

		/**
		 * Checks if the resource is currently in use by the GPU.
		 *
		 * @note Resource usage is only checked at certain points of the program. This means the resource could be
		 *       done on the device but this method may still report true.
		 */
		bool IsUsed() const
		{
			Lock lock(mMutex);
			return mUsedCount > 0;
		}

		/**
		 * Checks if the resource is currently bound to any command buffer.
		 *
		 * @note Resource usage is only checked at certain points of the program. This means the resource could be
		 *       done on the device but this method may still report true.
		 */
		bool IsBound() const
		{
			Lock lock(mMutex);
			return mBountCount > 0;
		}

		/** Checks if the resource has been queued for destruction (i.e. Destroy() was called). */
		bool IsDestroyRequested() const
		{
			Lock lock(mMutex);
			return mDestroyRequested;
		}

		/** Number of recorded-but-not-yet-submitted command buffers currently referencing this resource. */
		virtual u32 GetBoundCount() const
		{
			Lock lock(mMutex);
			return mBountCount;
		}

		/** Number of in-flight submissions currently referencing this resource. */
		virtual u32 GetUseCount() const
		{
			Lock lock(mMutex);
			return mUsedCount;
		}

		/**
		 * Queues the resource for destruction. If the resource is currently bound to a command buffer, the actual free
		 * is deferred until the bound count drops to zero; otherwise the manager frees it immediately. Only valid for
		 * resources constructed with a non-null manager.
		 *
		 * Marked virtual so specialty subclasses can wedge in pre-deferral work (e.g. unregistering from a cache,
		 * draining a message queue). Overrides must call the base implementation as their last step — once the base
		 * returns, this may already have been freed by the deferred-destroy path.
		 */
		virtual void Destroy();

		/**
		 * Called after the owning allocator has reserved a new home for this resource during defragmentation.
		 * Inside this call, the consumer's old GpuResourceLocation is still intact — the implementation can
		 * read its source heap / offset / size off it. The implementation must:
		 *   1. Record a copy from the source range to the destination range using @p commandBuffer.
		 *   2. Recreate any placed backend object (VkBuffer / VkImage / ...) bound to the new memory range.
		 *   3. Replace the IGpuResource's GpuResourceLocation with @p newLocation, so the location identifies
		 *      the destination slot from now on. The IGpuResource holding the new location must be returned
		 *      from MoveAllocation; how this is done depends on the allocator's @c FreeDeferralMode:
		 *
		 *      - FreeDeferralMode::ResourceLifecycle: the implementation must create a brand-new IGpuResource,
				  and call Destroy() on the old one. Resource lifecycle tracking will take care of releasing the
				  old object's memory once its no longer used on the GPU.
		 *
		 *      - FreeDeferralMode::FrameTracker: the implementation must patch the existing IGpuResource
		 *        in place to the new memory location and return 'this'. The allocator will interally free old memory 
		 *		  after the IFrameTracker reports it is no longer being used.
		 *
		 * @p newLocation is a backend-agnostic GpuResourceLocation; the consumer downcasts its opaque
		 * IGpuHeap* to the concrete backend heap to access native fields and slot identity.
		 *
		 * Must succeed; backends should not pick candidates whose recreation can fail.
		 */
		virtual IGpuResource* MoveAllocation(render::GpuCommandBuffer& commandBuffer, const GpuResourceLocation& newLocation)
		{
			(void)commandBuffer;
			(void)newLocation;
			return this;
		}

	protected:
		/**
		 * Hook invoked under the resource mutex from inside NotifyUsed, after the aggregate use counter has
		 * been incremented. Backends override this to perform their own per-queue / per-access accounting.
		 * Default implementation is empty.
		 */
		virtual void OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
		{
			(void)queueId;
			(void)useFlags;
		}

		/**
		 * Hook invoked under the resource mutex from inside NotifyDone, after the aggregate counters have been
		 * decremented and before the deferred-destroy condition is evaluated. Backends override this for their own
		 * per-queue / per-access accounting. Default implementation is empty.
		 */
		virtual void OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
		{
			(void)queueId;
			(void)useFlags;
		}

		/**
		 * Pre-delete cleanup hook. Fires exactly once, on the thread that decrements the bound count to zero
		 * after Destroy() has been called. The implementation should release any native handles
		 * (ComPtr / id<MTL...> / VkXxx) but must not delete this — the manager handles the B3DDelete that
		 * follows immediately after this call. Default implementation is empty.
		 */
		virtual void OnWillDestroy() {}

		String mDebugName;
		GpuResourceManager* mOwner = nullptr;

		/**
		 * Lock guarding the lifetime state. Held during all Notify* calls (and around the OnNotifyUsed /
		 * OnNotifyDone hooks), so backend overrides and backend queries can use this same mutex to protect
		 * their own state without re-entering it.
		 */
		mutable Mutex mMutex;

		/** Aggregate in-flight submission count. Updated only by IGpuResource under mMutex. */
		u32 mUsedCount = 0;

		/** Aggregate command-buffer binding count. Updated only by IGpuResource under mMutex. */
		u32 mBountCount = 0;

	private:
		/** Deletes the resource. Caller must ensure resource is not being used on the GPU or bound to a command buffer. */
		void DestroyImmediately();

		bool mDestroyRequested = false;
	};

#if B3D_BUILD_TYPE_DEVELOPMENT
	/** Tracks the bound/use state of a single suballocation within a buffer. */
	struct SuballocationTrackingState
	{
		u32 BoundCount = 0;  /**< Number of command buffers this suballocation is bound to. */
		u32 UseCount = 0;    /**< Number of submitted command buffers using this suballocation. */
	};
#endif

	/** Base for GPU buffer resources. */
	class B3D_EXPORT IGpuBufferResource : public IGpuResource
	{
	public:
		IGpuBufferResource(GpuResourceManager* owner, const StringView& name)
			: IGpuResource(owner, name)
		{}

#if B3D_BUILD_TYPE_DEVELOPMENT
		/**
		 * Initializes suballocation tracking for the specified count. Called during buffer creation. Only needs
		 * to be called for buffers with more than one suballocation.
		 *
		 * @param suballocationCount	Number of suballocations in the buffer.
		 * @param suballocationSize		Size of each suballocation in bytes.
		 */
		void InitializeSuballocationTracking(u32 suballocationCount, u32 suballocationSize);

		/** Notifies that a suballocation is bound to a command buffer. */
		void NotifySuballocationBound(u32 suballocationIndex);

		/** Notifies that a suballocation is used (command buffer submitted). */
		void NotifySuballocationUsed(u32 suballocationIndex);

		/** Notifies that a suballocation is done being used (command buffer completed). */
		void NotifySuballocationDone(u32 suballocationIndex);

		/** Notifies that a suballocation is unbound (command buffer destroyed without submit). */
		void NotifySuballocationUnbound(u32 suballocationIndex);

		/** Checks if a suballocation is currently bound to any command buffer. */
		bool IsSuballocationBound(u32 suballocationIndex) const;

		/** Checks if a suballocation is currently in use on the GPU. */
		bool IsSuballocationInUse(u32 suballocationIndex) const;

		/** Checks if any suballocation overlapping the given byte range is bound. */
		bool IsRangeBound(u32 offset, u32 size) const;

		/** Checks if any suballocation overlapping the given byte range is in use. */
		bool IsRangeInUse(u32 offset, u32 size) const;

		/** Returns the suballocation index for the given byte offset. */
		u32 GetSuballocationIndexForOffset(u32 offset) const;
#endif

	protected:
		IGpuBufferResource() = default;

#if B3D_BUILD_TYPE_DEVELOPMENT
		TInlineArray<SuballocationTrackingState, 2> mSuballocationStates;
		u32 mSuballocationSize = 0;  // Size of each suballocation (for range-to-index conversion)
#endif
	};

	/**
	 * Base for GPU image resources. Stores the full-image subresource range and
	 * the per-(face × mip) subresource resource pointers shared across backends.
	 * The subresource array is allocated by the constructor (zero-initialized);
	 * backends fill in the pointers during their own construction.
	 */
	class B3D_EXPORT IGpuImageResource : public IGpuResource
	{
	public:
		/**
		 * Constructs the image resource, recording its shape (face/mip counts + full-image subresource range)
		 * and allocating the per-(face × mip) subresource pointer storage. The pointers are zero-initialized;
		 * the backend fills them in during its own construction.
		 */
		IGpuImageResource(GpuResourceManager* owner, const StringView& name, u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask);

		~IGpuImageResource() override;

		/** Retrieves a subresource range covering all the sub-resources of the image. */
		const GpuTextureSubresourceRange& GetRange() const { return mFullRange; }

		/**
		 * Retrieves a separate resource for a specific image face & mip level. This allows the caller to track
		 * subresource usage individually, instead of for the entire image.
		 */
		IGpuResource* GetSubresource(u32 face, u32 mipLevel) const
		{
			B3D_ASSERT(mipLevel * mFaceCount + face < mFaceCount * mMipLevelCount);
			return mSubresources[mipLevel * mFaceCount + face];
		}

	protected:
		IGpuImageResource() = default;

		u32 mFaceCount = 0;
		u32 mMipLevelCount = 0;
		GpuTextureSubresourceRange mFullRange;
		IGpuResource** mSubresources = nullptr;
	};

	/** Base GPU swap chain resources. */
	class B3D_EXPORT IGpuSwapChainResource : public IGpuResource
	{
	public:
		IGpuSwapChainResource(GpuResourceManager* owner, const StringView& name)
			: IGpuResource(owner, name)
		{}

	protected:
		IGpuSwapChainResource() = default;
	};

	/** @} */
} // namespace b3d
