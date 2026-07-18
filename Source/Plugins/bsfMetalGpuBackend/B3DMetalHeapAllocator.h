//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/Allocators/B3DGpuAllocator.h"
#include "GpuBackend/Allocators/B3DGpuLinearAllocator.h"
#include "GpuBackend/Allocators/B3DGpuTlsfAllocator.h"
#include "Utility/B3DPool.h"
#include "Threading/B3DThreading.h"

namespace b3d
{
	struct GpuBufferInformation;

	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

#ifdef __OBJC__
		/** Native Metal heap handle. Aliased to void* in plain C++ TUs so class layouts stay identical (id is a pointer). */
		using MetalHeapNativeHandle = id<MTLHeap>;
#else
		using MetalHeapNativeHandle = void*;
#endif

		/** References a Metal memory heap, as returned by MetalHeapBackend. */
		struct MetalGpuHeap : IGpuHeap
		{
			/** Backing placement MTLHeap. Its hazard mode follows the backend synchronization toggle. */
			MetalHeapNativeHandle Heap = nullptr;

			u64 Size = 0; /**< Total heap size in bytes. */
			u32 MemoryType = 0; /**< Memory type the heap was created for. See MetalHeapAllocator::kMemoryType*. */
		};

		/** Downcasts an opaque engine heap handle to the concrete Metal heap it must refer to. */
		inline MetalGpuHeap& ToMetalGpuHeap(IGpuHeap* heap)
		{
			B3D_ASSERT(heap != nullptr);
			return *static_cast<MetalGpuHeap*>(heap);
		}

		/** Initializer struct for MetalHeapBackend::CreateHeap. */
		struct MetalHeapCreateInformation
		{
			u32 MemoryType = 0; /**< Memory type (storage-mode bucket) the heap serves. See MetalHeapAllocator::kMemoryType*. */
		};

		/**
		 * Metal implementation of the GpuHeapBackend trait. Creates placement-type MTLHeaps so the
		 * engine-side allocator strategies (TLSF / linear) own sub-allocation offsets, mirroring the
		 * Vulkan backend's bind-at-offset model. Heap hazard mode follows
		 * @c B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION and always matches its child resources.
		 *
		 * Requires placement-heap support (macOS 10.15). Apple Silicon Macs satisfy this requirement.
		 *
		 * @note Thread safe.
		 */
		class MetalHeapBackend
		{
		public:
			using HeapHandle = IGpuHeap*;
			using HeapCreateInformation = MetalHeapCreateInformation;

			explicit MetalHeapBackend(MetalGpuDevice& device);
			~MetalHeapBackend() = default;

			MetalHeapBackend(const MetalHeapBackend&) = delete;
			MetalHeapBackend& operator=(const MetalHeapBackend&) = delete;

			/** @name GpuHeapBackend trait surface.
			 *  @{
			 */

			/**
			 * Allocates a backing MTLHeap of @p sizeInBytes bytes according to @p createInformation.
			 * Returns a stable MetalGpuHeap pointer (as IGpuHeap*) minted from the backend's pool,
			 * or nullptr on failure (device lost, OS too old, or out of memory).
			 */
			HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

			/** Releases the MTLHeap and returns the heap object to the pool. */
			void DestroyHeap(HeapHandle handle);

			/** @} */

		private:
			MetalGpuDevice* mDevice = nullptr;

			/** Pool of heap objects with stable addresses. Guarded by mHeapPoolMutex — CreateHeap/DestroyHeap are called concurrently from per-memory-type allocators. */
			TPool<MetalGpuHeap> mHeapPool;
			Mutex mHeapPoolMutex;
		};

		B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MetalHeapBackend);

		/**
		 * Device-level GPU memory manager for the Metal backend. Owns the MetalHeapBackend and one
		 * persistent thread-safe TLSF allocator per memory type (private / shared storage), and
		 * mints placed MTLBuffer / MTLTexture objects at allocator-chosen offsets inside pooled
		 * placement heaps. This replaces the per-resource newBufferWithLength: /
		 * newTextureWithDescriptor: driver allocations, which are the single largest per-resource
		 * cost on Apple Silicon for resource-heavy scenes.
		 *
		 * Persistent requests the TLSF path cannot serve fall back to direct device allocations with
		 * an invalid GpuResourceLocation. Explicit-allocator requests never fall back because doing so
		 * would silently escape the transient allocator's frame-retirement contract.
		 *
		 * Ownership/lifetime: returned native handles are +1 references the caller owns (MRC).
		 * The paired GpuResourceLocation must be freed via its stamped allocator
		 * (location.Allocator->Free) once the resource's IGpuResource lifecycle reports it retired.
		 * Persistent TLSF allocators reclaim immediately under ResourceLifecycle deferral; transient
		 * linear allocators recycle their whole page after the completion tracker signals.
		 *
		 * @note Thread safe.
		 */
		class MetalHeapAllocator
		{
		public:
			/** Memory types resources allocate from. One TLSF allocator exists per type. */
			static constexpr u32 kMemoryTypePrivate = 0; /**< MTLStorageModePrivate — GPU-only resources. */
			static constexpr u32 kMemoryTypeShared = 1;  /**< MTLStorageModeShared — CPU-visible buffers. */
			static constexpr u32 kMemoryTypeCount = 2;

			MetalHeapAllocator(MetalGpuDevice& device);
			~MetalHeapAllocator();

			MetalHeapAllocator(const MetalHeapAllocator&) = delete;
			MetalHeapAllocator& operator=(const MetalHeapAllocator&) = delete;

			/**
			 * Determines the memory type a buffer described by @p information allocates from. A
			 * buffer's memory type is a pure function of its create information, fixed for its
			 * lifetime. Thread safe.
			 */
			static u32 PickBufferMemoryType(const GpuBufferInformation& information);

			/**
			 * Returns the persistent allocator for @p memoryType. Only valid when placement heaps
			 * are supported; intended for device-level wiring (PickBufferMemoryType /
			 * CreateTransientAllocator overrides) and diagnostics.
			 */
			IGpuAllocator& GetAllocator(u32 memoryType);

			/**
			 * Creates a context-owned transient linear allocator for @p memoryType. Normal pages are
			 * obtained from a device-owned per-memory-type pool and retired through
			 * @p completionTracker. Returns null when placement heaps are unavailable.
			 */
			TUnique<IGpuAllocator> CreateTransientAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker);

			/** True when placement-heap sub-allocation is available on this device/OS. */
			bool IsHeapSubAllocationSupported() const { return mPlacementHeapsSupported; }

#ifdef __OBJC__
			/**
			 * Allocates an MTLBuffer of @p length bytes from the persistent allocator for
			 * @p memoryType, placing it at the allocator-chosen heap offset. On success
			 * @p outLocation holds the backing span (free it via outLocation.Allocator->Free once
			 * the resource retires). On any heap-path miss the buffer is allocated directly on the
			 * device and @p outLocation is left invalid. Returns nil only on hard failure.
			 */
			id<MTLBuffer> AllocateBuffer(u64 length, u32 memoryType, GpuResourceLocation& outLocation);

			/**
			 * Allocates a placed buffer through an explicitly supplied allocator. The allocator must
			 * produce Metal heaps of @p memoryType. Unlike the persistent overload, this method does
			 * not fall back to a direct allocation when the allocator cannot satisfy the request.
			 */
			id<MTLBuffer> AllocateBuffer(u64 length, u32 memoryType, IGpuAllocator& allocator, GpuResourceLocation& outLocation);

			/**
			 * Counterpart of AllocateBuffer for textures. The memory type is derived from
			 * @p descriptor.storageMode; non-poolable storage modes fall through to a direct device
			 * allocation. The descriptor must carry the configured hazard mode for the direct path;
			 * heap-placed resources inherit the heap's matching mode.
			 */
			id<MTLTexture> AllocateTexture(MTLTextureDescriptor* descriptor, GpuResourceLocation& outLocation);
#endif

			/**
			 * Destroys the persistent allocators and transient page pools, releasing every pooled
			 * MTLHeap through the backend. Every resource sub-allocated from these heaps must have been destroyed
			 * beforehand — the resource manager's leak tracking asserts that in debug builds.
			 * Invoked from the device's destructor after WaitUntilIdle and resource teardown.
			 */
			void Shutdown();

		private:
			using MemoryAllocator = TGpuTlsfAllocator<MetalHeapBackend>;
			using LinearPagePool = TGpuLinearPagePool<MetalHeapBackend>;
			using TransientAllocator = TGpuLinearAllocator<MetalHeapBackend>;

			/** Returns the lazily-created shared transient-page pool for @p memoryType. */
			LinearPagePool& GetOrCreateLinearPagePool(u32 memoryType);

#ifdef __OBJC__
			/** Shared buffer allocation implementation for persistent and explicit allocator paths. */
			id<MTLBuffer> AllocateBufferInternal(u64 length, u32 memoryType, IGpuAllocator& allocator,
				bool allowDirectFallback, GpuResourceLocation& outLocation);
#endif

			MetalGpuDevice& mDevice;
			MetalHeapBackend mBackend;
			TUnique<MemoryAllocator> mAllocators[kMemoryTypeCount];
			TUnique<LinearPagePool> mLinearPagePools[kMemoryTypeCount];
			Mutex mLinearPagePoolMutex;

			bool mPlacementHeapsSupported = false; /**< Placement heaps need macOS 10.15. */
			bool mSharedHeapsSupported = false;    /**< Shared-storage heaps require unified memory. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
