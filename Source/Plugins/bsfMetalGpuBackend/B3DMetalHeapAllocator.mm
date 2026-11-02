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

			/** Returns true if @p texture can be bound as a render pass attachment. */
			bool IsAttachment(id<MTLTexture> texture)
			{
				return ([texture usage] & MTLTextureUsageRenderTarget) != 0;
			}
		} // namespace

		MetalHeapBackend::MetalHeapBackend(MetalGpuDevice& device, MetalResidencySet& residencySet)
			: mDevice(device), mResidencySet(residencySet)
		{ }

		MetalHeapBackend::HeapHandle MetalHeapBackend::CreateHeap(u64 sizeInBytes, const HeapCreateInformation& createInformation)
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
			heapDescriptor.hazardTrackingMode = MTLHazardTrackingModeUntracked;

			id<MTLHeap> heap = [device newHeapWithDescriptor:heapDescriptor];

			if (heap == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalHeapBackend: newHeapWithDescriptor failed for {0} bytes, memory type {1}.", sizeInBytes, createInformation.MemoryType);
				return nullptr;
			}

			heap.label = createInformation.MemoryType == MetalHeapAllocator::kMemoryTypeShared
				? @"Banshee shared placement heap"
				: @"Banshee private placement heap";

			// Covers every resource placed in the heap
			mResidencySet.Add(heap);

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

		void MetalHeapBackend::DestroyHeap(HeapHandle handle)
		{
			if (handle == nullptr)
				return;

			MetalGpuHeap& heap = ToMetalGpuHeap(handle);
			mResidencySet.Remove(heap.Heap);

			heap.Heap = nullptr;
			heap.Size = 0;
			heap.MemoryType = 0;

			Lock lock(mHeapPoolMutex);
			mHeapPool.Release(&heap);
		}

		MetalHeapAllocator::MetalHeapAllocator(MetalGpuDevice& device)
			: mDevice(device), mResidencySet(device.GetMetalDevice()), mBackend(device, mResidencySet)
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

		TUnique<IGpuTransientHeapPool> MetalHeapAllocator::CreateTransientHeapPool(IGpuCompletionTracker& completionTracker)
		{
			TGpuTransientHeapPool<MetalHeapBackend>::Configuration configuration;
			configuration.HeapCreateInformation.MemoryType = kMemoryTypePrivate;
			configuration.CompletionTracker = &completionTracker;

			return B3DMakeUnique<TGpuTransientHeapPool<MetalHeapBackend>>(&mBackend, configuration);
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

		id<MTLBuffer> MetalHeapAllocator::AllocateBuffer(u64 length, u32 memoryType, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation)
		{
			outAllocation.Reset();

			if (length == 0 || memoryType >= kMemoryTypeCount)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			const MTLResourceOptions options = MetalUtility::GetResourceOptions(GetMemoryTypeStorageMode(memoryType));

			if (requestedAllocation.HasMemory())
			{
				MetalGpuHeap& heap = ToMetalGpuHeap(requestedAllocation.Heap);
				B3D_ASSERT(heap.MemoryType == memoryType && "Allocation's memory type cannot back the buffer.");

				id<MTLBuffer> buffer = [heap.Heap newBufferWithLength:length options:options offset:requestedAllocation.Offset];
				if (buffer != nil)
					outAllocation = requestedAllocation;

				return buffer;
			}

			IGpuAllocator& allocator = *requestedAllocation.Allocator;
			const MTLSizeAndAlign sizeAndAlign = [device heapBufferSizeAndAlignWithLength:length options:options];

			GpuAllocation allocation;
			if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, GpuResourceKind::Linear, allocation))
			{
				MetalGpuHeap& heap = ToMetalGpuHeap(allocation.Heap);
				if (heap.MemoryType == memoryType)
				{
					id<MTLBuffer> buffer = [heap.Heap newBufferWithLength:length options:options offset:allocation.Offset];
					if (buffer != nil)
					{
						outAllocation = allocation;
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

			id<MTLBuffer> buffer = [device newBufferWithLength:length options:options];
			mResidencySet.Add(buffer);

			return buffer;
		}

		id<MTLTexture> MetalHeapAllocator::AllocateTexture(MTLTextureDescriptor* descriptor, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation)
		{
			outAllocation.Reset();

			if (descriptor == nil)
				return nil;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			const u32 memoryType = GetStorageModeMemoryType(descriptor.storageMode);

			if (requestedAllocation.HasMemory())
			{
				MetalGpuHeap& heap = ToMetalGpuHeap(requestedAllocation.Heap);
				B3D_ASSERT(heap.MemoryType == memoryType && "Allocation's memory type cannot back the texture.");

				id<MTLTexture> texture = [heap.Heap newTextureWithDescriptor:descriptor offset:requestedAllocation.Offset];
				if (texture != nil)
				{
					outAllocation = requestedAllocation;
					AddHeapAttachmentResidency(texture);
				}

				return texture;
			}

			if (memoryType >= kMemoryTypeCount)
			{
				id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
				mResidencySet.Add(texture);

				return texture;
			}

			IGpuAllocator& allocator = *requestedAllocation.Allocator;
			const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];

			GpuAllocation allocation;
			if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, GpuResourceKind::NonLinear, allocation))
			{
				MetalGpuHeap& heap = ToMetalGpuHeap(allocation.Heap);
				id<MTLTexture> texture = [heap.Heap newTextureWithDescriptor:descriptor offset:allocation.Offset];
				if (texture != nil)
				{
					outAllocation = allocation;
					AddHeapAttachmentResidency(texture);
					return texture;
				}

				allocator.FreeAndReclaim(allocation);
			}

			if (&allocator != mAllocators[memoryType].get())
				return nil;

			id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
			mResidencySet.Add(texture);
			
			return texture;
		}

		void MetalHeapAllocator::Free(id<MTLResource> resource, GpuAllocation& allocation)
		{
			if (!allocation.HasMemory())
			{
				mResidencySet.Remove(resource);
				return;
			}

			if ([resource conformsToProtocol:@protocol(MTLTexture)] && IsAttachment((id<MTLTexture>)resource))
				mResidencySet.Remove(resource);

			if (allocation.IsOwned())
				allocation.Allocator->Free(allocation);
		}

		void MetalHeapAllocator::AddHeapAttachmentResidency(id<MTLTexture> texture)
		{
			// The heap's residency covers the texture, but the validation layer warns about attachments that are not resident
			// themselves
			if (IsAttachment(texture))
				mResidencySet.Add(texture);
		}
	} // namespace render
} // namespace b3d
