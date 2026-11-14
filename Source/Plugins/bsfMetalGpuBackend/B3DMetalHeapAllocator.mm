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
			MTLStorageMode GetMemoryTypeStorageMode(MetalMemoryType memoryType)
			{
				switch (memoryType)
				{
				case MetalMemoryType::Shared:
					return MTLStorageModeShared;
				case MetalMemoryType::Private:
				default:
					return MTLStorageModePrivate;
				}
			}

			/** Returns the memory type backing @p storageMode, or MetalMemoryType::Count if heaps cannot back it. */
			MetalMemoryType GetStorageModeMemoryType(MTLStorageMode storageMode)
			{
				switch (storageMode)
				{
				case MTLStorageModePrivate:
					return MetalMemoryType::Private;
				case MTLStorageModeShared:
					return MetalMemoryType::Shared;
				default:
					return MetalMemoryType::Count;
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

			if (createInformation.MemoryType >= MetalMemoryType::Count)
			{
				B3D_LOG(Error, LogRenderBackend, "MetalHeapBackend: invalid memory type {0}.", (u32)createInformation.MemoryType);
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
				B3D_LOG(Error, LogRenderBackend, "MetalHeapBackend: newHeapWithDescriptor failed for {0} bytes, memory type {1}.", sizeInBytes, (u32)createInformation.MemoryType);
				return nullptr;
			}

			heap.label = createInformation.MemoryType == MetalMemoryType::Shared
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
			heap.MemoryType = MetalMemoryType::Private;

			Lock lock(mHeapPoolMutex);
			mHeapPool.Release(&heap);
		}

		MetalHeapAllocator::MetalHeapAllocator(MetalGpuDevice& device)
			: mDevice(device), mResidencySet(device.GetMetalDevice()), mBackend(device, mResidencySet)
		{
			for (u32 memoryTypeIndex = 0; memoryTypeIndex < (u32)MetalMemoryType::Count; memoryTypeIndex++)
			{
				const MetalMemoryType memoryType = (MetalMemoryType)memoryTypeIndex;
				MemoryAllocator::Configuration configuration;

				// Resources free their memory only once they retire, so it can be reclaimed immediately
				configuration.DeferralMode = GpuAllocatorFreeDeferralMode::ResourceLifecycle;

				// Shared memory mostly holds small staging and uniform data, so it starts with smaller heaps
				if (memoryType == MetalMemoryType::Shared)
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

				mAllocators[(u32)memoryType] = B3DMakeUnique<MemoryAllocator>(&mBackend, nullptr, configuration);
			}
		}

		MetalHeapAllocator::~MetalHeapAllocator()
		{
			for (TUnique<MemoryAllocator>& allocator : mAllocators)
				allocator.reset();

			for (TUnique<LinearPagePool>& pagePool : mLinearPagePools)
				pagePool.reset();
		}

		MetalMemoryType MetalHeapAllocator::GetBufferMemoryType(const GpuBufferInformation& information)
		{
			return MetalUtility::GetBufferStorageMode(information) == MTLStorageModeShared
				? MetalMemoryType::Shared
				: MetalMemoryType::Private;
		}

		IGpuAllocator& MetalHeapAllocator::GetGpuMemoryAllocator(MetalMemoryType memoryType)
		{
			B3D_ASSERT(memoryType < MetalMemoryType::Count);
			B3D_ASSERT(mAllocators[(u32)memoryType] != nullptr);

			return *mAllocators[(u32)memoryType];
		}

		MetalHeapAllocator::LinearPagePool& MetalHeapAllocator::GetOrCreateLinearPagePool(MetalMemoryType memoryType)
		{
			B3D_ASSERT(memoryType < MetalMemoryType::Count);

			Lock lock(mLinearPagePoolMutex);
			TUnique<LinearPagePool>& pagePool = mLinearPagePools[(u32)memoryType];
			if (pagePool != nullptr)
				return *pagePool;

			LinearPagePool::Configuration configuration;
			configuration.PageSize = 8ull * 1024 * 1024;
			configuration.MaxRetainedPages = 4;
			configuration.HeapCreateInfo.MemoryType = memoryType;

			pagePool = B3DMakeUnique<LinearPagePool>(&mBackend, configuration);
			return *pagePool;
		}

		TUnique<IGpuAllocator> MetalHeapAllocator::CreateScratchAllocator(MetalMemoryType memoryType, IGpuCompletionTracker& completionTracker)
		{
			if (memoryType >= MetalMemoryType::Count)
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
			configuration.HeapCreateInformation.MemoryType = MetalMemoryType::Private;
			configuration.CompletionTracker = &completionTracker;

			return B3DMakeUnique<TGpuTransientHeapPool<MetalHeapBackend>>(&mBackend, configuration);
		}

		GpuMemoryRequirements MetalHeapAllocator::GetBufferMemoryRequirements(u64 length, MetalMemoryType memoryType) const
		{
			GpuMemoryRequirements output;
			output.MemoryType = (u32)memoryType;

			id<MTLDevice> device = mDevice.GetMetalDevice();
			if (device == nil || memoryType >= MetalMemoryType::Count)
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

			id<MTLDevice> device = mDevice.GetMetalDevice();
			const MetalMemoryType memoryType = descriptor != nil ? GetStorageModeMemoryType(descriptor.storageMode) : MetalMemoryType::Count;
			if (device == nil || memoryType >= MetalMemoryType::Count)
			{
				output.MemoryType = GpuMemoryRequirements::kUnsupportedMemoryType;
				return output;
			}

			const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];
			output.MemoryType = (u32)memoryType;
			output.Size = sizeAndAlign.size;
			output.Alignment = sizeAndAlign.align;

			return output;
		}

		id<MTLBuffer> MetalHeapAllocator::AllocateBuffer(u64 length, MetalMemoryType memoryType, const GpuAllocation& requestedAllocation, GpuAllocation& outAllocation)
		{
			outAllocation.Reset();

			if (length == 0 || memoryType >= MetalMemoryType::Count)
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
			if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, allocation))
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
					B3D_LOG(Error, LogRenderBackend, "Metal buffer allocator returned memory type {0}, expected {1}.", (u32)heap.MemoryType, (u32)memoryType);
				}

				allocator.FreeAndReclaim(allocation);
			}

			if (&allocator != mAllocators[(u32)memoryType].get())
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

			const MetalMemoryType memoryType = GetStorageModeMemoryType(descriptor.storageMode);

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

			if (memoryType >= MetalMemoryType::Count)
			{
				id<MTLTexture> texture = [device newTextureWithDescriptor:descriptor];
				mResidencySet.Add(texture);

				return texture;
			}

			IGpuAllocator& allocator = *requestedAllocation.Allocator;
			const MTLSizeAndAlign sizeAndAlign = [device heapTextureSizeAndAlignWithDescriptor:descriptor];

			GpuAllocation allocation;
			if (allocator.TryAllocate(sizeAndAlign.size, (u32)sizeAndAlign.align, allocation))
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

			if (&allocator != mAllocators[(u32)memoryType].get())
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
