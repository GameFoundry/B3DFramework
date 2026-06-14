//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuQueue.h"

namespace b3d
{
	class GpuDevice;
	class GpuParameterSetPool;
	class IGpuAllocator;
	class IGpuCompletionTracker;
	class GpuFenceCompletionTracker;
	struct GpuBufferCreateInformation;

	namespace render
	{
		class GpuBuffer;
		class GpuCommandBuffer;
		class GpuCommandBufferPoolRing;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Base interface for objects owned by a GpuWorkContext and scoped to its lifetime. Registered via
	 * GpuWorkContext::AddLocal()/GetOrCreateLocal() and destroyed with the context, on the context's owning
	 * thread - after the context's GPU work has drained, but before the context's pools are torn down, so
	 * locals may release pool-allocated resources (e.g. parameter sets) from their destructors.
	 */
	class B3D_EXPORT IGpuWorkContextLocal
	{
	public:
		virtual ~IGpuWorkContextLocal() = default;
	};

	/**
	 * Owns various GPU state for work on a single thread/fiber, and is used for submitting GPU work from that
	 * thread/fiber. Backed by a GpuDevice; one device can have multiple contexts active at a time.
	 * 
	 * Provides parameter set and command buffer pools, transfer command buffers, transient memory allocations 
	 * and a completion tracker. 
	 * 
	 * It's expected that the renderer will have one of these, and any worker operation will create its own context.
	 *
	 * @note	Not thread safe. A single thread/fiber must own and drive a given context.
	 */
	class B3D_EXPORT GpuWorkContext
	{
		struct PrivatelyConstruct {};

	public:
		/**
		 * Constructs a context. The PrivatelyConstruct tag keeps direct construction to the Create()
		 * factories, which are the canonical way to build a context. See Create().
		 *
		 * @param	device	Device this context performs work against. Must outlive the context.
		 */
		GpuWorkContext(PrivatelyConstruct, GpuDevice& device);

		/**
		 * Constructs a context that borrows an externally-owned completion tracker. The PrivatelyConstruct
		 * tag keeps direct construction to the Create() factories. See Create().
		 *
		 * @param	device	Device this context performs work against. Must outlive the context.
		 * @param	tracker	Externally-owned tracker the context borrows (not owned). Must outlive the context.
		 */
		GpuWorkContext(PrivatelyConstruct, GpuDevice& device, IGpuCompletionTracker& tracker);

		/**
		 * For contexts owning their own fence tracker this settles all outstanding GPU work first, via
		 * WaitAndReclaim(). Contexts borrowing an external tracker must only be destroyed once their GPU
		 * work is known complete (e.g. after a device idle at shutdown). Must run on the owning thread.
		 */
		~GpuWorkContext();

		GpuWorkContext(const GpuWorkContext&) = delete;
		GpuWorkContext& operator=(const GpuWorkContext&) = delete;
		GpuWorkContext(GpuWorkContext&&) = delete;
		GpuWorkContext& operator=(GpuWorkContext&&) = delete;

		/**
		 * Creates a context that owns its own timeline-fence completion tracker. The canonical way
		 * to construct a context for a one-off operation driven by a single thread/fiber.
		 *
		 * @param	device	Device the context performs work against. Must outlive the returned context.
		 */
		static TShared<GpuWorkContext> Create(GpuDevice& device);

		/**
		 * Creates a context that borrows an externally-owned completion tracker instead of owning a
		 * fence. For long-lived, frame-driven contexts whose GPU progress is tracked externally - i.e.
		 * the renderer's render-thread context borrowing the device's frame completion tracker. One-off
		 * worker operations should use Create(GpuDevice&) instead.
		 *
		 * @param	device	Device the context performs work against. Must outlive the returned context.
		 * @param	tracker	Externally-owned tracker the context borrows (not owned). Must outlive the returned context.
		 */
		static TShared<GpuWorkContext> Create(GpuDevice& device, IGpuCompletionTracker& tracker);

		/** Device this context performs its GPU work against. */
		GpuDevice& GetDevice() const { return mDevice; }

		/** Completion tracker this context schedules object reclamation against. */
		IGpuCompletionTracker& GetCompletionTracker() const { return *mTracker; }

		/**
		 * Returns this context's transient (linear/bump) allocator for memory type @p memoryType, lazily
		 * manufacturing it via GpuDevice::CreateTransientAllocator on first use. The allocator draws pages
		 * from the device's shared per-type page pool and retires them against this context's completion
		 * tracker.
		 */
		IGpuAllocator& GetOrCreateTransientAllocator(u32 memoryType);

		/**
		 * Creates a buffer whose backing memory comes from this context's transient (linear/bump) allocator
		 * for the buffer's memory type. Allocation and free are extremely cheap — memory is not freed
		 * per-buffer but reclaimed in bulk by AdvanceFrame() once the GPU work that used it completes — so
		 * the buffer must only be used for short-lived, single-use work (compute scratch, staging) bounded
		 * by the frame/operation that created it, and must not be retained past that point.
		 *
		 * On backends without context transient allocation this falls back to a regular persistent buffer.
		 *
		 * @param	createInformation	Object describing the buffer to create.
		 */
		TShared<render::GpuBuffer> CreateTransientGpuBuffer(const GpuBufferCreateInformation& createInformation);

		/**
		 * Returns this context's pool for allocating GPU parameter sets. Parameter sets allocated from it must not
		 * outlive the context.
		 */
		GpuParameterSetPool& GetParameterSetPool() { return *mParameterSetPool; }

		/**
		 * Returns the context-local object registered under @p key, or null if none is. Context-local objects
		 * let systems cache per-context state (e.g. renderer material instances) with the context's lifetime.
		 * Must be called from the owning thread.
		 */
		IGpuWorkContextLocal* GetLocal(const void* key) const;

		/**
		 * Registers @p object as a context-local object under @p key, transferring its ownership to the
		 * context. No object must already be registered under the key. The object is destroyed with the
		 * context, on the owning thread, after the context's GPU work drains but before its pools are torn
		 * down. Must be called from the owning thread.
		 */
		IGpuWorkContextLocal& AddLocal(const void* key, TUnique<IGpuWorkContextLocal> object);

		/**
		 * Returns the context-local object registered under @p key, creating it on first use via @p factory
		 * (a callable returning TUnique<IGpuWorkContextLocal>). The object is cast to @p T, so a key must
		 * consistently map to a single type. See AddLocal().
		 */
		template <class T, class FactoryType>
		T& GetOrCreateLocal(const void* key, FactoryType&& factory)
		{
			if (IGpuWorkContextLocal* const existing = GetLocal(key))
				return static_cast<T&>(*existing);

			return static_cast<T&>(AddLocal(key, factory()));
		}

		/**
		 * Returns a command buffer for transfer (copy/upload) operations, lazily creating the context's
		 * transfer pool ring and the active command buffer if needed. The returned buffer is submitted by
		 * SubmitTransferCommandBuffers(), or recycled at the next AdvanceFrame().
		 */
		const TShared<render::GpuCommandBuffer>& GetTransferCommandBuffer();

		/**
		 * Submits a command buffer through this context. Any pending transfer command buffer owned by this
		 * context is flushed first (non-blocking), so transfers recorded against this context are visible to
		 * the submitted work, then the command buffer is submitted to the queue resolved from its queue type
		 * and @p queueIndex.
		 *
		 * @param	information	Command buffer + sync mask + signal fences to submit.
		 * @param	queueIndex	Index of the queue (of the command buffer's type) to submit on.
		 */
		void SubmitCommandBuffer(const GpuSubmissionInformation& information, u32 queueIndex = 0);

		/**
		 * Convenience overload of SubmitCommandBuffer that submits @p commandBuffer with the provided sync
		 * mask. See SubmitCommandBuffer(const GpuSubmissionInformation&, u32).
		 */
		void SubmitCommandBuffer(const TShared<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask = GpuQueueMask::kAll, u32 queueIndex = 0);

		/**
		 * Ends and submits the active transfer command buffer (if any) to the transfer queue. After this a
		 * fresh buffer is created on the next GetTransferCommandBuffer() call. SubmitCommandBuffer() flushes
		 * pending transfers automatically; call this directly when transfers must execute without follow-up
		 * work (e.g. before reading back a resource the transfers write).
		 *
		 * @param	wait	If true, blocks until the transfer queue finishes executing the submitted work.
		 */
		void SubmitTransferCommandBuffers(bool wait = false);

		/**
		 * Advances to the next frame at the frame boundary: recycles the transfer pool ring (clearing the
		 * active transfer command buffer) and reclaims transient memory the GPU has finished with - retiring
		 * each transient allocator's open page and draining everything whose completion marker has signaled
		 * (non-blocking; drains only what is already complete). Call once per frame after the prior frame's
		 * work is known safe to reuse, and before the frame index advances so retired pages are stamped with
		 * the correct frame.
		 */
		void AdvanceFrame();

		/**
		 * Blocking teardown of all outstanding GPU work owned by this context: flushes any pending
		 * transfer command buffer, blocks (yieldably) until the GPU drains this context's last
		 * submission, ensures the completion callbacks of the finished work have run (releasing any
		 * transient buffers they hold), then retires and force-drains all transient memory back to the
		 * device's shared page pool. Called automatically by the destructor for contexts that own their
		 * fence tracker. Must run on the owning thread.
		 */
		void WaitAndReclaim();

	private:
		/**
		 * Same as GetOrCreateTransientAllocator, but returns null when the backend does not support
		 * context transient allocation, instead of asserting.
		 */
		IGpuAllocator* TryGetOrCreateTransientAllocator(u32 memoryType);

#if B3D_DEBUG
		/**
		 * Asserts no transient allocation produced by this context is still alive — catching transient
		 * buffers that outlived their context. Only meaningful once the completion callbacks of the
		 * context's finished GPU work have run (they hold the last references to transient buffers), so
		 * call after the WaitAndReclaim() wait, or at destruction.
		 */
		void AssertNoOutstandingTransientAllocations() const;
#endif

		GpuDevice& mDevice; /**< Non-owning back-ref to the device. */
		IGpuCompletionTracker* mTracker; /**< Owned or borrowed. */
		TUnique<GpuFenceCompletionTracker> mOwnedTracker; /**< Non-null only owning a tracker. */

		/**
		 * Per-memory-type transient (linear) allocators this context owns and drives. Lazily populated by
		 * GetOrCreateTransientAllocator; each draws from the device's shared page pool for its type. Owned
		 * through IGpuAllocator (whose destructor is virtual), and destroyed with the context.
		 */
		Map<u32, TUnique<IGpuAllocator>> mTransientAllocators;

		/** Pool for GPU parameter sets allocated through this context. */
		TUnique<GpuParameterSetPool> mParameterSetPool;

		/**
		 * Context-local objects, keyed by their registering system. Declared after mParameterSetPool (and
		 * explicitly cleared first in the destructor) so locals are destroyed before the pools they may hold
		 * resources from.
		 */
		Map<const void*, TUnique<IGpuWorkContextLocal>> mLocalObjects;

		// Transfer command-buffers
		TUnique<render::GpuCommandBufferPoolRing> mTransferPoolRing;
		TShared<render::GpuCommandBuffer> mTransferCommandBuffer;
	};

	/** @} */

} // namespace b3d
