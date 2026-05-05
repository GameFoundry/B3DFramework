//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d
{
	class GpuResourceManager;
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
	 * Empty marker base for TGpuResourceLocation. Lets non-templated interfaces (e.g. IGpuResource::MoveAllocation) accept any backend's
	 * typed location through a common reference; the consumer downcasts to its concrete TGpuResourceLocation<HeapBackend>.
	 */
	struct GpuResourceLocation {};

	/**
	 * GPU memory allocation as returned by a GPU memory allocator. Used for freeing the allocation, as well
	 * as referencing the underlying memory. Each consumer owns their location and is the sole writer; the
	 * allocator only writes to the consumer's location once during the initial TryAllocate, and then
	 * supplies a fresh replacement location to IGpuResource::MoveAllocation when defragmentation
	 * moves the allocation.
	 *
	 * Inherits from GpuResourceLocation so any backend-specialised location can be passed through
	 * the non-templated IGpuResource::MoveAllocation hook and downcast back to its concrete type.
	 *
	 * @tparam HeapBackend	Backend trait satisfying the GpuHeapBackend contract.
	 */
	template <typename HeapBackend>
	struct TGpuResourceLocation : GpuResourceLocation
	{
		using HeapHandle = typename HeapBackend::HeapHandle;

		HeapHandle Heap{};
		u64 Offset = 0;
		u64 Size = 0;

		/** Type-erased pointer to the allocator strategy that owns this allocation. */
		void* Allocator = nullptr;

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
			Heap = HeapHandle{};
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
		 * Inside this call, the consumer's old TGpuResourceLocation is still intact — the implementation can
		 * read its source heap / offset / size off it. The implementation must:
		 *   1. Record a copy from the source range to the destination range using @p commandBuffer.
		 *   2. Recreate any placed backend object (VkBuffer / VkImage / ...) bound to the new memory range.
		 *   3. Replace the IGpuResource's TGpuResourceLocation with @p newLocation, so the location identifies
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
		 * @p newLocation is a GpuResourceLocation reference; the consumer downcasts to its concrete
		 * TGpuResourceLocation<HeapBackend> to access the typed heap handle and slot identity.
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

	/** @} */
} // namespace b3d
