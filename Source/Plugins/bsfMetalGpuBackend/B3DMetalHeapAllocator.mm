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
			/** Returns the storage mode backing @p memoryType. */
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

			/** Returns the memory type backing @p storageMode, or MetalHeapAllocator::kMemoryTypeCount if heaps cannot back it. */
			u32 GetStorageModeMemoryType(MTLStorageMode storageMode)
			{
				switch (storageMode)
				{
				case MTLStorageModePrivate:
					return MetalHeapAllocator::kMemoryTypePrivate;
				case MTLStorageModeShared:
					return MetalHeapAllocator::kMemoryTypeShared;
				default:
					return MetalHeapAllocator::kMemoryTypeCount;
				}
			}
		} // namespace

		MetalHeapBackend::MetalHeapBackend(MetalGpuDevice& device)
			: mDevice(device)
		{ }

		MetalHeapBackend::HeapHandle MetalHeapBackend::CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
		{
			// Drained locally since the calling thread may have no run loop
			@autoreleasepool
			{
				id<MTLDevice> device = mDevice.GetMetalDevice();
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
				heapDescriptor.type = MTLHeapTypePlacement;

				// Must match the hazard tracking mode MetalUtility assigns to resources placed in the heap
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
					B3D_LOG(Error, LogRenderBackend, "MetalHeapBackend: newHeapWithDescriptor failed for {0} bytes, memory type {1}.", sizeInBytes, createInformation.MemoryType);
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
			}
		}

		void MetalHeapBackend::DestroyHeap(HeapHandle handle)
		{
			if (handle == nullptr)
				return;

			MetalGpuHeap& heap = ToMetalGpuHeap(handle);

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

				// Resources free their memory only once they retire, so it can be reclaimed immediately
				configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

				// Placement heaps have no buffer-image granularity, alignment comes from the per-resource size queries
				configuration.Granularity = 1;

				// Shared memory mostly holds small staging and uniform data, so it starts with smaller heaps
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
			for (u32 memoryType = 0; memoryType < kMemoryTypeCount; memoryType++)
				mAllocators[memoryType].reset();

			for (u32 memoryType = 0; memoryType < kMemoryTypeCount; memoryType++)
				mLinearPagePools[memoryType].reset();
		}

		u32 MetalHeapAllocator::GetBufferMemoryType(const GpuBufferInformation& information)
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
			TUnique<LinearPagePool>& pagePool = mLinearPagePools[memoryType];
			if (pagePool != nullptr)
				return *pagePool;

			LinearPagePool::Configuration configuration;
			configuration.PageSize = 8ull * 1024 * 1024;
			configuration.MaxRetainedPages = 4;
			configuration.HeapCreateInfo.MemoryType = memoryType;

			pagePool = B3DMakeUnique<LinearPagePool>(&mBackend, configuration);
			return *pagePool;
		}

		TUnique<IGpuAllocator> MetalHeapAllocator::CreateScratchAllocator(u32 memoryType, IGpuCompletionTracker& completionTracker)
		{
			if (memoryType >= kMemoryTypeCount)
				return nullptr;

			LinearPagePool& pagePool = GetOrCreateLinearPagePool(memoryType);

			ScratchAllocator::Configuration configuration;
			configuration.PageSize = pagePool.GetPageSize();
			configuration.HeapCreateInfo.MemoryType = memoryType;

			return B3DMakeUnique<ScratchAllocator>(&mBackend, &completionTracker, configuration, &pagePool);
		}

		GpuMemoryRequirements MetalHeapAllocator::GetBufferMemoryRequirements(u64 length, u32 memoryType) const
		{
			GpuMemoryRequirements output;
			output.MemoryType = memoryType;
			output.Kind = GpuResourceKind::Linear;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil || memoryType >= kMemoryTypeCount)
			{
				output.MemoryType = GpuMemoryRequirements::kUnsupportedMemoryType;
				return output;
			}

			const MTLResourceOptions options = MetalUtility::GetResourceOptions(GetMemoryTypeStorageMode(memoryType));
			const MTLSizeAndAlign sizeAndAlign = [device heapBufferSizeAndAlignWithLength:length options:options];
			output.Size = sizeAndAlign.size;
			output.Alignment = sizeAndAlign.align;

			return output;
		}

		GpuMemoryRequirements MetalHeapAllocator::GetTextureMemoryRequirements(MTLTextureDescriptor* descriptor) const
		{
			GpuMemoryRequirements output;
			output.Kind = GpuResourceKind::NonLinear;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			const u32 memoryType = descriptor != nil ? GetStorageModeMemoryType(descriptor.storageMode) : kMemoryTypeCount;
			if (device == nil || memoryType >= kMemoryTypeCount)
			{
				output.MemoryType = GpuMemoryRequirements::kUnsupportedMemoryType;
				return output;
			}

			const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];
			output.MemoryType = memoryType;
			output.Size = sizeAndAlign.size;
			output.Alignment = sizeAndAlign.align;

			return output;
		}

		id<MTLBuffer> MetalHeapAllocator::AllocateBuffer(u64 length, u32 memoryType, const GpuResourceLocation& location, GpuResourceLocation& outLocation)
		{
			outLocation.Reset();

			if (length == 0 || memoryType >= kMemoryTypeCount)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			const MTLResourceOptions options = MetalUtility::GetResourceOptions(GetMemoryTypeStorageMode(memoryType));

			@autoreleasepool
			{
				if (location.HasMemory())
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(location.Heap);
					B3D_ASSERT(heap.MemoryType == memoryType && "Location's memory type cannot back the buffer.");

					id<MTLBuffer> buffer = [heap.Heap newBufferWithLength:length options:options offset:location.Offset];
					if (buffer != nil)
						outLocation = location;

					return buffer;
				}

				IGpuAllocator& allocator = *location.Allocator;
				const MTLSizeAndAlign sizeAndAlign = [device heapBufferSizeAndAlignWithLength:length options:options];

				GpuResourceLocation allocation;
				if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, GpuResourceKind::Linear, nullptr, allocation))
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(allocation.Heap);
					if (heap.MemoryType == memoryType)
					{
						id<MTLBuffer> buffer = [heap.Heap newBufferWithLength:length options:options offset:allocation.Offset];
						if (buffer != nil)
						{
							outLocation = allocation;
							return buffer;
						}
					}
					else
					{
						B3D_LOG(Error, LogRenderBackend, "Metal buffer allocator returned memory type {0}, expected {1}.", heap.MemoryType, memoryType);
					}

					allocator.FreeAndReclaim(allocation);
				}

				if (&allocator != mAllocators[memoryType].get())
					return nil;

				return [device newBufferWithLength:length options:options];
			}
		}

		id<MTLTexture> MetalHeapAllocator::AllocateTexture(MTLTextureDescriptor* descriptor, const GpuResourceLocation& location, GpuResourceLocation& outLocation)
		{
			outLocation.Reset();

			if (descriptor == nil)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			const u32 memoryType = GetStorageModeMemoryType(descriptor.storageMode);

			@autoreleasepool
			{
				if (location.HasMemory())
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(location.Heap);
					B3D_ASSERT(heap.MemoryType == memoryType && "Location's memory type cannot back the texture.");

					id<MTLTexture> texture = [heap.Heap newTextureWithDescriptor:descriptor offset:location.Offset];
					if (texture != nil)
						outLocation = location;

					return texture;
				}

				if (memoryType >= kMemoryTypeCount)
					return [device newTextureWithDescriptor:descriptor];

				IGpuAllocator& allocator = *location.Allocator;
				const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];

				GpuResourceLocation allocation;
				if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, GpuResourceKind::NonLinear, nullptr, allocation))
				{
					MetalGpuHeap& heap = ToMetalGpuHeap(allocation.Heap);
					id<MTLTexture> texture = [heap.Heap newTextureWithDescriptor:descriptor offset:allocation.Offset];
					if (texture != nil)
					{
						outLocation = allocation;
						return texture;
					}

					allocator.FreeAndReclaim(allocation);
				}

				if (&allocator != mAllocators[memoryType].get())
					return nil;

				return [device newTextureWithDescriptor:descriptor];
			}
		}
	} // namespace render
} // namespace b3d
