//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"

namespace b3d
{
	class GpuResourceManager;
	class IGpuResource;
	struct GpuAllocation;
	namespace render { class GpuCommandBuffer; }

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Memory-layout category of a GPU allocation. Some APIs require different allocation granularity when
	 * linear and non-linear entries overlap (i.e. buffer image granularity), and this is used by the allocator
	 * to respect that.
	 */
	enum class GpuResourceKind : u8 // TODO - Rename to GpuResourceMemoryLayout
	{
		Linear		= 0,
		NonLinear	= 1
	};

	/** Memory a texture or a buffer needs, as reported by the GPU device before the resource is created. */
	struct GpuMemoryRequirements
	{
		/** Memory type reported for a resource the device cannot create. */
		static constexpr u32 kUnsupportedMemoryType = ~0u;

		u32 MemoryType = 0; /**< Backend-defined index of the memory pool the resource must be placed in. */
		u64 Size = 0; /**< Minimum size of the memory range, in bytes. */
		u64 Alignment = 1; /**< Required alignment of the memory range's offset, in bytes. */
		GpuResourceKind Kind = GpuResourceKind::Linear;
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
		virtual bool TryAllocate(u64 size, u32 alignment, GpuResourceKind kind, IGpuResource* owner, GpuAllocation& out) = 0;

		/** Retires @p allocation (deferred free per the allocator's policy) and resets it to the empty state. */
		virtual void Free(GpuAllocation& allocation) = 0;

		/** Releases @p allocation immediately, bypassing any deferred-free queue, and resets it to the empty state. */
		virtual void FreeAndReclaim(GpuAllocation& allocation) = 0;

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
		 * diagnostic, used to catch allocations that outlive their allocator (e.g. a scratch buffer
		 * outliving its GpuWorkContext).
		 */
		virtual u64 GetOutstandingAllocationCount() const { return 0; }
#endif

	protected:
		IGpuAllocator() = default;
	};

	/**
	 * GPU memory allocation as returned by a GPU memory allocator. Used for freeing the allocation, as well
	 * as referencing the underlying memory. Each consumer owns their allocation and is the sole writer; the
	 * allocator only writes to the consumer's allocation once during the initial TryAllocate, and then
	 * supplies a fresh replacement allocation to IGpuResource::MoveAllocation when defragmentation
	 * moves the allocation.
	 *
	 * An allocation is in one of four states, determined by which of Heap and Allocator are set:
	 *  - Empty (neither): refers to nothing.
	 *  - Pending (Allocator only): requests memory from Allocator. See CreatePending().
	 *  - Non-owning (Heap only): refers to memory owned elsewhere, which must never be freed through the allocation.
	 *  - Owned (both): a live allocation owned through Allocator, and freed through it.
	 *
	 * Must stay standard-layout and trivially-copyable.
	 */
	struct GpuAllocation
	{
		IGpuHeap* Heap = nullptr;
		u64 Offset = 0;
		u64 Size = 0;

		/** Allocator that produced this allocation; used to free or relocate it. Stamped at TryAllocate. */
		IGpuAllocator* Allocator = nullptr;

		// Strategy-private bookkeeping. Interpretation is private to the owning allocator.
		u32 AllocatorData0 = 0;
		u32 AllocatorData1 = 0;

		/** Creates a pending allocation, which requests its memory from @p allocator. */
		static GpuAllocation CreatePending(IGpuAllocator& allocator)
		{
			GpuAllocation output;
			output.Allocator = &allocator;

			return output;
		}

		/** Returns true if the allocation refers to memory, owned or not. */
		bool HasMemory() const { return Heap != nullptr; }

		/** Returns true if the allocation requests memory from its allocator, but has none yet. */
		bool IsPending() const { return Heap == nullptr && Allocator != nullptr; }

		/** Returns true if the allocation refers to a live allocation owned through its allocator. */
		bool IsOwned() const { return Heap != nullptr && Allocator != nullptr; }

		/** Resets the allocation to the empty state. */
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
		static constexpr u32 kMaximumUniqueQueueCount = B3D_MAX_QUEUES_PER_TYPE * GQT_COUNT;

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

		/** Returns the debug name. Empty if none was set. */
		const String& GetDebugName() const { return mDebugName; }

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

		/** Returns queues on which the resource currently has in-flight accesses matching @p useFlags. */
		GpuQueueMask GetUseInfo(GpuAccessFlags useFlags) const;

#if B3D_BUILD_TYPE_DEVELOPMENT
		/**
		 * Marks whether a resource sharing memory with this one has started a new lifetime on it (see GpuBarrier::AliasAcquire).
		 * Accessing a superseded resource is invalid until it is acquired again. Development builds only.
		 */
		void SetSupersededByAlias(bool superseded) { mIsSupersededByAlias.store(superseded, std::memory_order_relaxed); }

		/** Returns true if a resource sharing memory with this one has started a new lifetime on it since this resource was last acquired. */
		bool IsSupersededByAlias() const { return mIsSupersededByAlias.load(std::memory_order_relaxed); }
#endif

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
		 * Inside this call, the consumer's old GpuAllocation is still intact — the implementation can
		 * read its source heap / offset / size off it. The implementation must:
		 *   1. Record a copy from the source range to the destination range using @p commandBuffer.
		 *   2. Recreate any placed backend object (VkBuffer / VkImage / ...) bound to the new memory range.
		 *   3. Replace the IGpuResource's GpuAllocation with @p newAllocation, so the allocation identifies
		 *      the destination slot from now on. The IGpuResource holding the new allocation must be returned
		 *      from MoveAllocation; how this is done depends on the allocator's @c FreeDeferralMode:
		 *
		 *      - FreeDeferralMode::ResourceLifecycle: the implementation must create a brand-new IGpuResource,
				  and call Destroy() on the old one. Resource lifecycle tracking will take care of releasing the
				  old object's memory once its no longer used on the GPU.
		 *
		 *      - FreeDeferralMode::FrameTracker: the implementation must patch the existing IGpuResource
		 *        in place to the new allocation and return 'this'. The allocator will interally free old memory 
		 *		  after the IFrameTracker reports it is no longer being used.
		 *
		 * @p newAllocation is a backend-agnostic GpuAllocation; the consumer downcasts its opaque
		 * IGpuHeap* to the concrete backend heap to access native fields and slot identity.
		 *
		 * Must succeed; backends should not pick candidates whose recreation can fail.
		 */
		virtual IGpuResource* MoveAllocation(render::GpuCommandBuffer& commandBuffer, const GpuAllocation& newAllocation)
		{
			(void)commandBuffer;
			(void)newAllocation;
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

		/** Per-queue in-flight read counts. Guarded by mMutex. */
		u8 mReadUses[kMaximumUniqueQueueCount] = {};

		/** Per-queue in-flight write counts. Guarded by mMutex. */
		u8 mWriteUses[kMaximumUniqueQueueCount] = {};

	private:
		/** Deletes the resource. Caller must ensure resource is not being used on the GPU or bound to a command buffer. */
		void DestroyImmediately();

		bool mDestroyRequested = false;

#if B3D_BUILD_TYPE_DEVELOPMENT
		std::atomic<bool> mIsSupersededByAlias = false;
#endif
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

		/** Returns submission hazards shared by all command buffers using this buffer. Submit thread only. */
		const render::GpuResourceSubmissionState& GetSubmissionState() const { return mSubmissionState; }

		/** Returns the submission state for an in-place update. Submit thread only. */
		render::GpuResourceSubmissionState& GetSubmissionState() { return mSubmissionState; }

		/** Commits submission hazards after native boundary synchronization has been constructed. Submit thread only. */
		void SetSubmissionState(render::GpuResourceSubmissionState&& state) { mSubmissionState = std::move(state); }

	protected:
		IGpuBufferResource() = default;

#if B3D_BUILD_TYPE_DEVELOPMENT
		TInlineArray<SuballocationTrackingState, 2> mSuballocationStates;
		u32 mSuballocationSize = 0;  // Size of each suballocation (for range-to-index conversion)
#endif

	private:
		/** Used for issuing transitions between command buffers. Stores information about last submitted state. Submit thread only. */
		render::GpuResourceSubmissionState mSubmissionState;
	};

	/**
	 * Use counters, submission state and native state of one face × mip × aspect of an image, or of the image's full range. Owned
	 * by the image, which creates and destroys it; not owned by a GpuResourceManager.
	 */
	class B3D_EXPORT GpuImageSubresource final : public IGpuResource
	{
	public:
		GpuImageSubresource() = default;

		/** Submission hazards left by the last submitted command buffers using this subresource. Submit thread only. */
		render::GpuResourceSubmissionState SubmissionState;

		/** Backend-native state committed by the last submitted command buffer using this subresource. Submit thread only. */
		render::GpuImageNativeState NativeState;
	};

	/**
	 * Base for GPU image resources. Stores the full-image subresource range and owns one GpuImageSubresource per
	 * face × mip × aspect, plus one full-range subresource.
	 *
	 * Submission state is either uniform or split. While uniform, the full-range subresource holds the submission and native state
	 * of every subresource. While split, each subresource holds its own. Only single-aspect images can be uniform.
	 * Only uniform images rest.
	 */
	class B3D_EXPORT IGpuImageResource : public IGpuResource
	{
	public:
		/** Constructs a manager-owned image resource of the specified shape, and its subresources. */
		IGpuImageResource(GpuResourceManager* owner, const StringView& name, u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask);

		~IGpuImageResource() override;

		/** Retrieves a subresource range covering all the sub-resources of the image. */
		const GpuTextureSubresourceRange& GetRange() const { return mFullRange; }

		/**
		 * Retrieves a subresource range covering the faces and mip levels described by @p surface, over every aspect of
		 * the image. A zero face or mip count covers everything from the first face or mip onward, and explicit counts
		 * are clamped to the subresources that remain.
		 */
		GpuTextureSubresourceRange GetRange(const TextureSurface& surface) const;

		/** Returns true if @p range covers every subresource of the image. */
		bool IsFullRange(const GpuTextureSubresourceRange& range) const;

		/** Retrieves the subresource holding use counters of partial accesses to one face, mip level and aspect. */
		GpuImageSubresource* GetSubresource(u32 face, u32 mipLevel, GpuTextureAspectFlag aspect) const;

		/** Retrieves the subresource holding use counters of accesses that cover the image's full range. */
		GpuImageSubresource* GetFullRangeSubresource() const { return &mSubresources[GetSubresourceCount()]; }

		/**
		 * Returns true if the full-range subresource holds the submission state of every subresource. Always false for images
		 * with more than one aspect. Submit thread only.
		 */
		bool HasUniformSubmissionState() const { return mHasUniformSubmissionState; }

		/**
		 * Returns the subresource holding the submission and native state of one face, mip level and aspect: the full-range
		 * subresource if the image is uniform, the subresource itself otherwise. Submit thread only.
		 */
		GpuImageSubresource& GetSubmissionStateResource(u32 face, u32 mipLevel, GpuTextureAspectFlag aspect) const;

		/** Copies the full-range state into every subresource and switches the image to per-subresource state. Submit thread only. */
		void SplitSubmissionState();

		/**
		 * Sets the native state of every subresource in @p range. Switches the image to per-subresource state if @p range does not cover
		 * the whole image. Submit thread only.
		 */
		void SetNativeState(const GpuTextureSubresourceRange& range, const render::GpuImageNativeState& nativeState);

		/**
		 * Merges the state of every subresource into the full-range subresource and switches the image to uniform state. Fails and
		 * changes nothing if the image has more than one aspect, if the native states differ, or if the submission states cannot
		 * be represented by one state (see GpuResourceSubmissionState::TryMerge()). Submit thread only.
		 */
		bool TryMergeSubmissionState(u32 frameIndex);

		/** Returns queues using any aspect of the specified face and mip level. */
		GpuQueueMask GetSubresourceUseInfo(u32 face, u32 mipLevel, GpuAccessFlags useFlags) const;

		/** Returns the total bound count across every aspect of the specified face and mip level. */
		u32 GetSubresourceBoundCount(u32 face, u32 mipLevel) const;

		/** Returns the total in-flight use count across every aspect of the specified face and mip level. */
		u32 GetSubresourceUseCount(u32 face, u32 mipLevel) const;

		/**
		 * Returns true if the image can ever be transitioned into a 'rest' state. Resting state implies read-only access and does not require hazard tracking.
		 * It is used primarily as an optimization so we don't need to perform hazard tracking on every single image (large majority of images are read-only sampleable images).
		 */
		bool CanRest() const { return mCanRest; }

	protected:
		/** Constructs an unmanaged image resource (no owner). Reserved for test mocks, see IGpuResource(). */
		IGpuImageResource(u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask);

		/** Sets the native state of every subresource of @p aspects, and of the full-range subresource. Backends call this during construction. */
		void InitializeNativeState(GpuTextureAspectFlags aspects, const render::GpuImageNativeState& state);

		/** Returns the storage index of one face, mip level and aspect. */
		u32 GetSubresourceIndex(u32 face, u32 mipLevel, GpuTextureAspectFlag aspect) const;

		/** Returns the number of face × mip × aspect subresources owned by the image, excluding the full-range subresource. */
		u32 GetSubresourceCount() const { return mFaceCount * mMipLevelCount * mFullRange.GetAspectCount(); }

		u32 mFaceCount = 0;
		u32 mMipLevelCount = 0;
		GpuTextureSubresourceRange mFullRange;

		bool mCanRest = false;

	private:
		/** Creates the subresources and selects the initial submission state mode. */
		void CreateSubresources();

		/** Face × mip × aspect subresources, followed by the full-range subresource. */
		GpuImageSubresource* mSubresources = nullptr;
		bool mHasUniformSubmissionState = false;
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
