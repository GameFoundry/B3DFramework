//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuTransientResourceAllocator.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d
{
	TConfigVariable<bool> gGpuTransientResources("gpu.TransientResources",
		"If disabled, transient resources are created in persistent memory, and never share memory with other resources.", true);
	TConfigVariable<u32> gGpuTransientMaxCachedTextures("gpu.TransientMaxCachedTextures",
		"Maximum number of textures each transient resource allocator caches.", 256);
	TConfigVariable<u32> gGpuTransientMaxCachedBuffers("gpu.TransientMaxCachedBuffers",
		"Maximum number of buffers each transient resource allocator caches.", 256);
} // namespace b3d

using namespace b3d;

GpuTransientTexture GpuTransientScope::AllocateTexture(const TextureCreateInformation& createInformation, u32 firstSubmission)
{
	GpuTransientResourceAllocator& allocator = mAllocator;
	B3D_ASSERT(allocator.mTimeline != nullptr && "Transient resources can only be allocated within a scope.");

	GpuTransientTexture output;
	const bool isGpuOnly = !createInformation.Usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess) && createInformation.InitialData == nullptr;
	if(!B3D_ENSURE_LOG(isGpuOnly, "Transient texture '{0}' must only be accessed by the GPU.", createInformation.Name))
		return output;

	// D3D12 copy queues cannot transition texture layouts, which the alias acquire before the first use requires
	if(!B3D_ENSURE_LOG(allocator.mTimeline->GetQueue(firstSubmission).GetType() != GQT_TRANSFER, "The first use of transient texture '{0}' cannot be on a transfer queue.", createInformation.Name))
		return output;

	auto fnIsSame = [&createInformation](const GpuTransientResourceAllocator::Resource& resource)
	{
		return GpuTransientResourceAllocator::IsSameTexture(resource.TextureDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	bool hasMemoryRequirements = false;
	for(const GpuTransientResourceAllocator::Resource* cachedTexture : allocator.mCachedTextures)
	{
		if(fnIsSame(*cachedTexture))
		{
			memoryRequirements = cachedTexture->MemoryRequirements;
			hasMemoryRequirements = true;
			break;
		}
	}

	if(!hasMemoryRequirements)
		memoryRequirements = allocator.mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return output;

	auto fnCreate = [&allocator, &createInformation](GpuTransientResourceAllocator::Resource& resource, const GpuAllocation* location)
	{
		resource.Texture = location != nullptr ? allocator.mDevice.CreateTexture(createInformation, *location, GpuObjectCreateFlag::Aliased) : allocator.mDevice.CreateTexture(createInformation);
		resource.TextureDescription = createInformation;
		return resource.Texture != nullptr;
	};

	const GpuTransientResourceAllocator::Resource* resource = allocator.AllocateResource(allocator.mCachedTextures, memoryRequirements, firstSubmission, output.Acquire, fnCreate, fnIsSame);
	if(resource == nullptr)
		return GpuTransientTexture();

#if B3D_BUILD_TYPE_DEVELOPMENT
	resource->Texture->SetName(createInformation.Name);
#endif

	output.Texture = resource->Texture;
	return output;
}

GpuTransientBuffer GpuTransientScope::AllocateBuffer(const GpuBufferCreateInformation& createInformation, u32 firstSubmission)
{
	GpuTransientResourceAllocator& allocator = mAllocator;
	B3D_ASSERT(allocator.mTimeline != nullptr && "Transient resources can only be allocated within a scope.");

	GpuTransientBuffer output;
	const bool isStaging = createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite;
	const bool isGpuOnly = !isStaging && !createInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess);
	if(!B3D_ENSURE_LOG(isGpuOnly, "Transient buffers must only be accessed by the GPU."))
		return output;

	auto fnIsSame = [&createInformation](const GpuTransientResourceAllocator::Resource& resource)
	{
		return GpuTransientResourceAllocator::IsSameBuffer(resource.BufferDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	bool hasMemoryRequirements = false;
	for(const GpuTransientResourceAllocator::Resource* cachedBuffer : allocator.mCachedBuffers)
	{
		if(fnIsSame(*cachedBuffer))
		{
			memoryRequirements = cachedBuffer->MemoryRequirements;
			hasMemoryRequirements = true;
			break;
		}
	}

	if(!hasMemoryRequirements)
		memoryRequirements = allocator.mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return output;

	auto fnCreate = [&allocator, &createInformation](GpuTransientResourceAllocator::Resource& resource, const GpuAllocation* location)
	{
		resource.Buffer = location != nullptr ? allocator.mDevice.CreateGpuBuffer(createInformation, *location, GpuObjectCreateFlag::Aliased) : allocator.mDevice.CreateGpuBuffer(createInformation);
		resource.BufferDescription = createInformation;
		return resource.Buffer != nullptr;
	};

	const GpuTransientResourceAllocator::Resource* resource = allocator.AllocateResource(allocator.mCachedBuffers, memoryRequirements, firstSubmission, output.Acquire, fnCreate, fnIsSame);
	if(resource == nullptr)
		return GpuTransientBuffer();

	output.Buffer = resource->Buffer;
	return output;
}

void GpuTransientScope::Release(const render::Texture& texture, TArrayView<const GpuTransientLastUse> lastUses)
{
	mAllocator.ReleaseResource(&texture, lastUses);
}

void GpuTransientScope::Release(const render::GpuBuffer& buffer, TArrayView<const GpuTransientLastUse> lastUses)
{
	mAllocator.ReleaseResource(&buffer, lastUses);
}

GpuTransientResourceAllocator::GpuTransientResourceAllocator(GpuDevice& device)
	: mDevice(device), mScope(*this)
{ }

GpuTransientResourceAllocator::~GpuTransientResourceAllocator()
{
	B3D_ASSERT(mTimeline == nullptr && "Transient resource allocator destroyed while a scope is open.");

	for(Resource* resource : mCachedTextures)
		EvictResource(resource);

	for(Resource* resource : mCachedBuffers)
		EvictResource(resource);
}

GpuTransientScope& GpuTransientResourceAllocator::BeginScope(const GpuSubmissionTimeline& timeline)
{
	B3D_ASSERT(mTimeline == nullptr && "Only one transient resource scope may be open at a time.");

	// Release cached resources that went unused for too long, along with their heap references
	const u64 frameIndex = mDevice.GetFrameIndex();
	const u32 idleFrames = gGpuTransientIdleFrames;
	auto fnEvictIdleResources = [frameIndex, idleFrames](TArray<Resource*>& cachedResources)
	{
		u32 keptCount = 0;
		for(u32 resourceIndex = 0; resourceIndex < cachedResources.Size(); resourceIndex++)
		{
			Resource* resource = cachedResources[resourceIndex];
			if(frameIndex - resource->AllocationFrame >= idleFrames)
				EvictResource(resource);
			else
				cachedResources[keptCount++] = resource;
		}

		cachedResources.Erase(cachedResources.Begin() + keptCount, cachedResources.End());
	};

	fnEvictIdleResources(mCachedTextures);
	fnEvictIdleResources(mCachedBuffers);

	mTimeline = &timeline;
	mScopeIndex++;
	mAllocationCount = 0;
	mCacheHits = 0;
	mCacheMisses = 0;
	mAliasingAllocator.BeginScope(timeline);

	return mScope;
}

void GpuTransientResourceAllocator::EndScope()
{
	B3D_ASSERT(mTimeline != nullptr && "No transient resource scope is open.");

	// The scope's work is synchronized once the scope ends, so a resource that was not released no longer uses its memory
	for(const auto& entry : mAllocatedResources)
	{
		Resource* resource = entry.second;
		B3D_ENSURE_LOG(false, "A transient resource was not released before the end of the scope that allocated it.");

		if(resource->Pool == nullptr)
			B3DDelete(resource);
	}

	mAllocatedResources.clear();

#if B3D_BUILD_TYPE_DEVELOPMENT
	for(const Resource* resource : mScopeReleasedResources)
	{
		const IGpuResource* gpuResource = GetGpuResource(*resource);
		if(resource->IsReleasedWithUses && gpuResource != nullptr)
			B3D_ENSURE_LOG(gpuResource->GetAliasAcquireCount() != resource->AliasAcquireCount, "A transient resource was used without an alias acquire before its first use.");
	}

	mScopeReleasedResources.Clear();
#endif

	mLastScopeStatistics.LastScope = mAliasingAllocator.GetStatistics();
	mLastScopeStatistics.AllocationCount = mAllocationCount;
	mLastScopeStatistics.CacheHits = mCacheHits;
	mLastScopeStatistics.CacheMisses = mCacheMisses;

	mAliasingAllocator.EndScope();
	mTimeline = nullptr;

	// Release the least recently used resources beyond the cache size
	auto fnEvictExcessResources = [](TArray<Resource*>& cachedResources, u32 maximumCount)
	{
		if(cachedResources.Size() <= maximumCount)
			return;

		std::stable_sort(cachedResources.Begin(), cachedResources.End(), [](const Resource* a, const Resource* b)
		{
			if(a->AllocationFrame != b->AllocationFrame)
				return a->AllocationFrame < b->AllocationFrame;

			return a->AllocationScope < b->AllocationScope;
		});

		const u32 excessCount = (u32)cachedResources.Size() - maximumCount;
		for(u32 resourceIndex = 0; resourceIndex < excessCount; resourceIndex++)
			EvictResource(cachedResources[resourceIndex]);

		cachedResources.Erase(cachedResources.Begin(), cachedResources.Begin() + excessCount);
	};

	fnEvictExcessResources(mCachedTextures, gGpuTransientMaxCachedTextures);
	fnEvictExcessResources(mCachedBuffers, gGpuTransientMaxCachedBuffers);
}

GpuTransientStatistics GpuTransientResourceAllocator::GetStatistics() const
{
	GpuTransientStatistics output = mLastScopeStatistics;
	output.CachedTextures = (u32)mCachedTextures.Size();
	output.CachedBuffers = (u32)mCachedBuffers.Size();

	return output;
}

template<class CreateFunction, class IsSameFunction>
GpuTransientResourceAllocator::Resource* GpuTransientResourceAllocator::AllocateResource(TArray<Resource*>& cachedResources, const GpuMemoryRequirements& memoryRequirements,
	u32 firstSubmission, render::GpuAliasAcquire& outAcquire, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame)
{
	mAllocationCount++;

	// Memory types the device cannot alias use persistent memory instead
	IGpuTransientHeapPool* pool = gGpuTransientResources ? mDevice.GetTransientHeapPool(memoryRequirements.MemoryType) : nullptr;

	// A cached resource is reused if the memory it was created at is free in the scope. Each cached resource is handed out at
	// most once per scope, so every allocation of a scope is a separate resource.
	Resource* resource = nullptr;
	GpuAllocation allocation;
	if(pool != nullptr)
	{
		// TODO - Linear scan over the whole cache, with a full description compare per entry. It is done twice per allocation,
		// here and in GpuTransientScope::AllocateTexture()/AllocateBuffer() to find the memory requirements. Look up cached
		// resources by a description hash instead.
		for(Resource* cachedResource : cachedResources)
		{
			if(cachedResource->AllocationScope == mScopeIndex || cachedResource->Pool != pool || !fnIsSame(*cachedResource))
				continue;

			if(mAliasingAllocator.TryAllocateAt(*pool, cachedResource->Allocation, firstSubmission, allocation, outAcquire))
			{
				resource = cachedResource;
				break;
			}
		}

		if(resource == nullptr && !mAliasingAllocator.TryAllocate(*pool, memoryRequirements.Size, memoryRequirements.Alignment, firstSubmission, allocation, outAcquire))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate {0} bytes of transient memory of type {1}. Using persistent memory instead.", memoryRequirements.Size, memoryRequirements.MemoryType);
			pool = nullptr;
		}
	}

	if(resource != nullptr)
		mCacheHits++;
	else
	{
		resource = B3DNew<Resource>();
		resource->MemoryRequirements = memoryRequirements;
		resource->Pool = pool;

		if(!fnCreate(*resource, pool != nullptr ? &allocation : nullptr))
		{
			// The memory was never used, so it keeps the uses of the resources released on it before
			if(pool != nullptr)
				mAliasingAllocator.Release(allocation, TArrayView<const GpuTransientLastUse>(), nullptr);

			B3DDelete(resource);
			return nullptr;
		}

		if(pool != nullptr)
		{
			pool->AddHeapReference(allocation.Heap);
			cachedResources.Add(resource);
			mCacheMisses++;
		}
	}

	resource->Allocation = allocation;
	resource->AllocationScope = mScopeIndex;
	resource->AllocationFrame = mDevice.GetFrameIndex();

	const void* resourceKey = resource->Texture != nullptr ? (const void*)resource->Texture.get() : (const void*)resource->Buffer.get();
	mAllocatedResources[resourceKey] = resource;

#if B3D_BUILD_TYPE_DEVELOPMENT
	// A cached resource may be placed over memory it released itself, and an acquire must not supersede the resource it acquires
	IGpuResource* gpuResource = GetGpuResource(*resource);

	resource->Predecessors.Clear();
	for(IGpuResource* predecessor : outAcquire.Predecessors)
	{
		if(predecessor != gpuResource)
			resource->Predecessors.Add(predecessor);
	}

	outAcquire.Predecessors = TArrayView<IGpuResource* const>(resource->Predecessors.Data(), resource->Predecessors.Size());
	resource->AliasAcquireCount = gpuResource != nullptr ? gpuResource->GetAliasAcquireCount() : 0;
	resource->IsReleasedWithUses = false;
#endif

	return resource;
}

void GpuTransientResourceAllocator::ReleaseResource(const void* resourceKey, TArrayView<const GpuTransientLastUse> lastUses)
{
	B3D_ASSERT(mTimeline != nullptr && "Transient resources can only be released within a scope.");

	const auto found = mAllocatedResources.find(resourceKey);
	if(!B3D_ENSURE_LOG(found != mAllocatedResources.end(), "Only a transient resource allocated in the open scope can be released."))
		return;

	Resource* resource = found->second;
	mAllocatedResources.erase(found);

	if(resource->Pool == nullptr)
	{
		B3DDelete(resource);
		return;
	}

	mAliasingAllocator.Release(resource->Allocation, lastUses, GetGpuResource(*resource));

#if B3D_BUILD_TYPE_DEVELOPMENT
	resource->IsReleasedWithUses = !lastUses.IsEmpty();
	mScopeReleasedResources.Add(resource);
#endif
}

void GpuTransientResourceAllocator::EvictResource(Resource* resource)
{
	// The resource's native destruction waits for the GPU to finish using it. The heap is destroyed even later, once it went
	// unused for gGpuTransientIdleFrames frames and the GPU finished the frame that released it.
	resource->Pool->ReleaseHeap(resource->Allocation.Heap);
	B3DDelete(resource);
}

IGpuResource* GpuTransientResourceAllocator::GetGpuResource(const Resource& resource)
{
	return resource.Texture != nullptr ? resource.Texture->GetGpuResource() : resource.Buffer->GetGpuResource();
}

bool GpuTransientResourceAllocator::IsSameTexture(const TextureInformation& a, const TextureInformation& b)
{
	return a.Type == b.Type && a.Format == b.Format && a.Width == b.Width && a.Height == b.Height && a.Depth == b.Depth && a.MipMapCount == b.MipMapCount &&
		a.Usage == b.Usage && a.UseHardwareSRGB == b.UseHardwareSRGB && std::max(a.SampleCount, 1u) == std::max(b.SampleCount, 1u) && a.ArraySliceCount == b.ArraySliceCount &&
		a.ClearColor == b.ClearColor && a.ClearDepth == b.ClearDepth && a.ClearStencil == b.ClearStencil;
}

bool GpuTransientResourceAllocator::IsSameBuffer(const GpuBufferInformation& a, const GpuBufferInformation& b)
{
	if(a.Type != b.Type || a.Flags != b.Flags || a.SuballocationCount != b.SuballocationCount)
		return false;

	switch(a.Type)
	{
	case GpuBufferType::Vertex:
		return a.Vertex.ElementSize == b.Vertex.ElementSize && a.Vertex.Count == b.Vertex.Count;
	case GpuBufferType::Index:
		return a.Index.Type == b.Index.Type && a.Index.Count == b.Index.Count;
	case GpuBufferType::Uniform:
		return a.Uniform.Size == b.Uniform.Size;
	case GpuBufferType::SimpleStorage:
		return a.SimpleStorage.Count == b.SimpleStorage.Count && a.SimpleStorage.Format == b.SimpleStorage.Format;
	case GpuBufferType::StructuredStorage:
		return a.StructuredStorage.Count == b.StructuredStorage.Count && a.StructuredStorage.ElementSize == b.StructuredStorage.ElementSize;
	default:
		return a.Staging.Size == b.Staging.Size;
	}
}
