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
		/** Native Metal heap handle. Plain C++ translation units see it as void*, which has the same layout. */
		using MetalHeapNativeHandle = id<MTLHeap>;
#else
		using MetalHeapNativeHandle = void*;
#endif

		/** References a Metal memory heap, as returned by MetalHeapBackend. */
		struct MetalGpuHeap : IGpuHeap
		{
			MetalHeapNativeHandle Heap = nullptr; /**< Backing placement heap. Resources are placed at allocator-supplied offsets. */
			u64 Size = 0; /**< Total heap size in bytes. */
			u32 MemoryType = 0; /**< One of the MetalHeapAllocator::kMemoryType* values. */
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
			u32 MemoryType = 0; /**< One of the MetalHeapAllocator::kMemoryType* values. */
		};

		/**
		 * Metal implementation of the GpuHeapBackend trait. Creates placement heaps, so the engine-side allocators decide
		 * resource offsets. Heap hazard tracking follows @c B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION, matching the
		 * resources placed in them.
		 *
		 * @note	Thread safe.
		 */
		class MetalHeapBackend
		{
		public:
			using HeapHandle = IGpuHeap*;
			using HeapCreateInformation = MetalHeapCreateInformation;

			explicit MetalHeapBackend(MetalGpuDevice& device);

			MetalHeapBackend(const MetalHeapBackend&) = delete;
			MetalHeapBackend& operator=(const MetalHeapBackend&) = delete;

			/** @name GpuHeapBackend trait surface.
			 *  @{
			 */

			/**
			 * Allocates a backing heap of @p sizeInBytes bytes according to @p createInformation. Returns a stable
			 * MetalGpuHeap pointer (as IGpuHeap*) from the backend's pool, or null on failure.
			 */
			HeapHandle CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation);

			/** Releases the heap and returns the heap object to the pool. */
			void DestroyHeap(HeapHandle handle);

			/** @} */

		private:
			MetalGpuDevice& mDevice;

			/** Pool of heap objects with stable addresses. Guarded by mHeapPoolMutex, since allocators of different memory types create and destroy heaps concurrently. */
			TPool<MetalGpuHeap> mHeapPool;
			Mutex mHeapPoolMutex;
		};

		B3D_STATIC_ASSERT_HEAP_BACKEND_IS_VALID(MetalHeapBackend);

		/**
		 * Device-level GPU memory manager for the Metal backend. Creates buffers and textures inside placement heaps,
		 * avoiding a driver allocation per resource. Per memory type it owns a persistent TLSF allocator for long-lived
		 * resources, and a page pool shared by the scratch linear allocators it creates for transient resources. Resources
		 * can be allocated from either, or placed at an allocation that already has memory.
		 *
		 * Returned buffers and textures are owned by the caller. Their GpuAllocation must be freed through
		 * its allocator once the resource retires. Only requests the persistent allocators cannot satisfy fall back to a
		 * direct device allocation with an empty allocation. Scratch allocators never fall back, since a direct allocation
		 * would escape their frame retirement.
		 *
		 * @note	Thread safe.
		 */
		class MetalHeapAllocator
		{
		public:
			/** Memory types resources allocate from. */
			static constexpr u32 kMemoryTypePrivate = 0; /**< MTLStorageModePrivate, for GPU-only resources. */
			static constexpr u32 kMemoryTypeShared = 1; /**< MTLStorageModeShared, for CPU-visible resources. */
			static constexpr u32 kMemoryTypeCount = 2;

			explicit MetalHeapAllocator(MetalGpuDevice& device);

			/** Releases every heap. All resources placed in them must have been destroyed beforehand. */
			~MetalHeapAllocator();

			MetalHeapAllocator(const MetalHeapAllocator&) = delete;
			MetalHeapAllocator& operator=(const MetalHeapAllocator&) = delete;

			/** Returns the memory type a buffer described by @p information allocates from. */
			static u32 GetBufferMemoryType(const GpuBufferInformation& information);

			/** Returns the persistent allocator for @p memoryType. */
			IGpuAllocator& GetAllocator(u32 memoryType);

			/**
			 * Creates a scratch linear allocator for @p memoryType, whose pages are retired through @p completionTracker.
			 * Pages are recycled through a pool shared by every scratch allocator of the same memory type. Returns null if
			 * @p memoryType is invalid.
			 */
			TUnique<IGpuAllocator> CreateScratchAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker);

#ifdef __OBJC__
			/**
			 * Returns the memory requirements of a buffer of @p length bytes, allocated from @p memoryType. Memory type is
			 * GpuMemoryRequirements::kUnsupportedMemoryType if the buffer cannot be placed in a heap.
			 */
			GpuMemoryRequirements GetBufferMemoryRequirements(u64 length, u32 memoryType) const;

			/**
			 * Returns the memory requirements of a texture described by @p descriptor. Memory type is
			 * GpuMemoryRequirements::kUnsupportedMemoryType if the texture cannot be placed in a heap.
			 */
			GpuMemoryRequirements GetTextureMemoryRequirements(MTLTextureDescriptor* descriptor) const;

			/**
			 * Creates a buffer of @p length bytes in @p memoryType memory. If @p requestedAllocation already has memory the
			 * buffer is placed there, otherwise it is allocated from its allocator, which must provide heaps of
			 * @p memoryType. On success @p outAllocation receives the memory backing the buffer, or is left empty if the
			 * buffer was allocated directly from the device. Returns nil on failure.
			 */
			id<MTLBuffer> AllocateBuffer(u64 length, u32 memoryType, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation);

			/**
			 * Creates a texture described by @p descriptor, see AllocateBuffer(). The memory type is derived from the
			 * descriptor's storage mode, and storage modes without a memory type are always allocated directly from the
			 * device. The descriptor must carry the configured hazard tracking mode, which direct allocations use in place of
			 * the heap's.
			 */
			id<MTLTexture> AllocateTexture(MTLTextureDescriptor* descriptor, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation);
#endif

		private:
			using MemoryAllocator = TGpuTlsfAllocator<MetalHeapBackend>;
			using LinearPagePool = TGpuLinearPagePool<MetalHeapBackend>;
			using ScratchAllocator = TGpuLinearAllocator<MetalHeapBackend>;

			/** Returns the scratch page pool for @p memoryType, creating it on first use. */
			LinearPagePool& GetOrCreateLinearPagePool(u32 memoryType);

			MetalGpuDevice& mDevice;
			MetalHeapBackend mBackend;
			TUnique<MemoryAllocator> mAllocators[kMemoryTypeCount];
			TUnique<LinearPagePool> mLinearPagePools[kMemoryTypeCount];
			Mutex mLinearPagePoolMutex;
		};

		/** @} */
	} // namespace render
} // namespace b3d
