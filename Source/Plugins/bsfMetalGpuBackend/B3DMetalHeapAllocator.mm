//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalHeapAllocator.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/** Returns the MTLStorageMode backing the provided memory type. */
			MTLStorageMode GetMemoryTypeStorageMode(u32 memoryType)
			{
				switch (memoryType)
				{
				case MetalHeapAllocator::kMemoryTypeShared:
					return MTLStorageModeShared;
				case MetalHeapAllocator::kMemoryTypePrivate:
				default:
					return MTLStorageModePrivate;
				}
			}
		}

		MetalHeapBackend::MetalHeapBackend(MetalGpuDevice& device)
			: mDevice(&device)
		{ }

		MetalHeapBackend::HeapHandle MetalHeapBackend::CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
		{
			// Descriptor allocations below are autoreleased internally; drain locally rather than
			// relying on a runloop — there may be none under the engine's fiber scheduler.
			@autoreleasepool
			{
			id<MTLDevice> device = mDevice->GetMetalDevice();
			if (device == nil)
				return nullptr;

			if (createInformation.MemoryType >= MetalHeapAllocator::kMemoryTypeCount)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalHeapBackend: invalid memory type {0}.", createInformation.MemoryType);
				return nullptr;
			}

			MTLHeapDescriptor* heapDescriptor = [[MTLHeapDescriptor alloc] init];
			heapDescriptor.size = sizeInBytes;
			heapDescriptor.storageMode = GetMemoryTypeStorageMode(createInformation.MemoryType);
			heapDescriptor.cpuCacheMode = MTLCPUCacheModeDefaultCache;

			// Placement heaps: the engine-side TLSF/linear allocators own offsets, so resources
			// are created at explicit allocator-chosen offsets (mirroring Vulkan's
			// bind-at-offset model). Automatic heaps cannot honor GpuResourceLocation offsets.
			heapDescriptor.type = MTLHeapTypePlacement;

			// Tracked mode delegates hazards to Metal. Explicit mode uses untracked heaps and
			// relies on the command buffer's barriers, encoder fences, and queue events. Child
			// resource descriptors use the matching policy through MetalUtility.
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			heapDescriptor.hazardTrackingMode = MTLHazardTrackingModeUntracked;
#else
			heapDescriptor.hazardTrackingMode = MTLHazardTrackingModeTracked;
#endif

			id<MTLHeap> heap = [device newHeapWithDescriptor:heapDescriptor];
#if !__has_feature(objc_arc)
			[heapDescriptor release];
#endif

			if (heap == nil)
			{
				B3D_LOG(Error, LogRenderBackend,
					"MetalHeapBackend: newHeapWithDescriptor failed for {0} bytes, memory type {1}.",
					sizeInBytes, createInformation.MemoryType);
				return nullptr;
			}

			heap.label = createInformation.MemoryType == MetalHeapAllocator::kMemoryTypeShared
				? @"Banshee shared placement heap"
				: @"Banshee private placement heap";

			MetalGpuHeap* heapWrapper = nullptr;
			{
				Lock lock(mHeapPoolMutex);
				heapWrapper = mHeapPool.Allocate();
			}

			heapWrapper->Heap = heap;
			heapWrapper->Size = sizeInBytes;
			heapWrapper->MemoryType = createInformation.MemoryType;

			return heapWrapper;
			} // @autoreleasepool
		}

		void MetalHeapBackend::DestroyHeap(HeapHandle handle)
		{
			if (handle == nullptr)
				return;

			MetalGpuHeap& heap = ToMetalGpuHeap(handle);

			// Under MRC the heap was returned with +1 from newHeapWithDescriptor; release it
			// explicitly. Under ARC the nil assignment below drops the strong reference.
#if !__has_feature(objc_arc)
			[heap.Heap release];
#endif
			heap.Heap = nullptr;
			heap.Size = 0;
			heap.MemoryType = 0;

			Lock lock(mHeapPoolMutex);
			mHeapPool.Release(&heap);
		}

		MetalHeapAllocator::MetalHeapAllocator(MetalGpuDevice& device)
			: mDevice(device), mBackend(device)
		{
			for (u32 memoryType = 0; memoryType < kMemoryTypeCount; memoryType++)
			{
				MemoryAllocator::Configuration configuration;

				// Wrappers fully implement the IGpuResource lifecycle (Part A's tracker drives
				// Notify*), and they free their span from the destructor which only runs once
				// the resource has retired — so Free may reclaim immediately, and no
				// completion tracker is required.
				configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

				// Metal placement heaps have no buffer-image granularity constraint analogous
				// to Vulkan's; per-request alignment comes from heap*SizeAndAlign* queries.
				configuration.Granularity = 1;

				// Private resources dominate long-lived scene memory and benefit from larger heaps.
				// Shared resources are generally staging/uniform data; starting those at 16 MiB avoids
				// reserving a 64 MiB heap for the first small CPU-visible buffer.
				if (memoryType == kMemoryTypeShared)
				{
					configuration.InitialHeapSize = 16ull * 1024 * 1024;
					configuration.MaxHeapSize = 64ull * 1024 * 1024;
				}
				else
				{
					configuration.InitialHeapSize = 64ull * 1024 * 1024;
					configuration.MaxHeapSize = 256ull * 1024 * 1024;
				}
				configuration.GrowthFactor = 2;
				configuration.MaxEmptyHeapCount = 1;

				configuration.HeapCreateInfo.MemoryType = memoryType;

				mAllocators[memoryType] = B3DMakeUnique<MemoryAllocator>(&mBackend, nullptr, configuration);
			}
		}

		MetalHeapAllocator::~MetalHeapAllocator()
		{
			// Persistent allocator teardown returns every remaining heap through
			// MetalHeapBackend::DestroyHeap. All resources sub-allocated from these heaps must have
			// been destroyed by this point — the resource manager's debug leak tracking and the
			// allocators' outstanding-allocation counters back that invariant.
			for (u32 memoryType = 0; memoryType < kMemoryTypeCount; memoryType++)
				mAllocators[memoryType].reset();

			for (u32 memoryType = 0; memoryType < kMemoryTypeCount; memoryType++)
				mLinearPagePools[memoryType].reset();
		}

		u32 MetalHeapAllocator::PickBufferMemoryType(const GpuBufferInformation& information)
		{
			return MetalUtility::GetBufferStorageMode(information) == MTLStorageModeShared
				? kMemoryTypeShared
				: kMemoryTypePrivate;
		}

		IGpuAllocator& MetalHeapAllocator::GetAllocator(u32 memoryType)
		{
			B3D_ASSERT(memoryType < kMemoryTypeCount);
			B3D_ASSERT(mAllocators[memoryType] != nullptr);

			return *mAllocators[memoryType];
		}

		MetalHeapAllocator::LinearPagePool& MetalHeapAllocator::GetOrCreateLinearPagePool(u32 memoryType)
		{
			B3D_ASSERT(memoryType < kMemoryTypeCount);

			Lock lock(mLinearPagePoolMutex);
			TUnique<LinearPagePool>& slot = mLinearPagePools[memoryType];
			if (slot != nullptr)
				return *slot;

			LinearPagePool::Configuration configuration;
			configuration.PageSize = 8ull * 1024 * 1024;
			configuration.MaxRetainedPages = 4;
			configuration.HeapCreateInfo.MemoryType = memoryType;

			slot = B3DMakeUnique<LinearPagePool>(&mBackend, configuration);
			return *slot;
		}

		TUnique<IGpuAllocator> MetalHeapAllocator::CreateTransientAllocator(u32 memoryType,
			IGpuCompletionTracker& completionTracker)
		{
			if (memoryType >= kMemoryTypeCount)
				return nullptr;

			LinearPagePool& pool = GetOrCreateLinearPagePool(memoryType);

			TransientAllocator::Configuration configuration;
			configuration.PageSize = pool.GetPageSize();
			configuration.HeapCreateInfo.MemoryType = memoryType;

			return B3DMakeUnique<TransientAllocator>(&mBackend, &completionTracker, configuration, &pool);
		}

		id<MTLBuffer> MetalHeapAllocator::AllocateBuffer(u64 length, u32 memoryType, GpuResourceLocation& outLocation)
		{
			outLocation.Reset();

			if (length == 0 || memoryType >= kMemoryTypeCount)
				return nil;

			return AllocateBufferInternal(length, memoryType, *mAllocators[memoryType], true, outLocation);
		}

		id<MTLBuffer> MetalHeapAllocator::AllocateBuffer(u64 length, u32 memoryType, IGpuAllocator& allocator,
			GpuResourceLocation& outLocation)
		{
			outLocation.Reset();

			if (length == 0 || memoryType >= kMemoryTypeCount)
				return nil;

			return AllocateBufferInternal(length, memoryType, allocator, false, outLocation);
		}

		id<MTLBuffer> MetalHeapAllocator::AllocateBufferInternal(u64 length, u32 memoryType,
			IGpuAllocator& allocator, bool allowDirectFallback, GpuResourceLocation& outLocation)
		{
			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			const MTLResourceOptions options = MetalUtility::GetResourceOptions(GetMemoryTypeStorageMode(memoryType));

			@autoreleasepool
			{
				const MTLSizeAndAlign sizeAndAlign = [device heapBufferSizeAndAlignWithLength:length options:options];

				GpuResourceLocation location;
				if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align,
					GpuResourceKind::Linear, nullptr, location))
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(location.Heap);
					if (heap.MemoryType == memoryType)
					{
						id<MTLBuffer> buffer = [heap.Heap newBufferWithLength:length options:options offset:location.Offset];
						if (buffer != nil)
						{
							outLocation = location;
							return buffer;
						}
					}
					else
					{
						B3D_LOG(Error, LogRenderBackend,
							"Metal buffer allocator returned memory type {0}, expected {1}.",
							heap.MemoryType, memoryType);
					}

					allocator.FreeAndReclaim(location);
				}

				if (!allowDirectFallback)
					return nil;

				// Heap-path miss (allocator out of device memory, or a placed create failed) — fall
				// back to a direct device allocation so the caller still gets a usable buffer. The
				// options mask preserves the configured hazard policy.
				return [device newBufferWithLength:length options:options];
			} // @autoreleasepool
		}

		id<MTLTexture> MetalHeapAllocator::AllocateTexture(MTLTextureDescriptor* descriptor, GpuResourceLocation& outLocation)
		{
			outLocation.Reset();

			if (descriptor == nil)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			// Derive the memory type from the descriptor's storage mode. Anything other than
			// private/shared (managed, memoryless) is not pooled and goes straight to the device.
			const MTLStorageMode storageMode = descriptor.storageMode;
			u32 memoryType = kMemoryTypeCount;
			if (storageMode == MTLStorageModePrivate)
				memoryType = kMemoryTypePrivate;
			else if (storageMode == MTLStorageModeShared)
				memoryType = kMemoryTypeShared;

			@autoreleasepool
			{
			if (memoryType < kMemoryTypeCount)
			{
				// Free layout query, mirrors the buffer path above.
				const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];

				GpuResourceLocation location;
				if (mAllocators[memoryType]->TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, GpuResourceKind::NonLinear, location))
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(location.Heap);
					id<MTLTexture> texture = [heap.Heap newTextureWithDescriptor:descriptor offset:location.Offset];
					if (texture != nil)
					{
						outLocation = location;
						return texture;
					}

					mAllocators[memoryType]->FreeAndReclaim(location);
				}
			}

			return [device newTextureWithDescriptor:descriptor];
			} // @autoreleasepool
		}

	} // namespace render
} // namespace b3d
