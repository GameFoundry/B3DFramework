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

namespace
{
	/** Returns true if a texture described by @p createInformation is only accessed by the GPU. */
	bool IsGpuOnly(const TextureCreateInformation& createInformation)
	{
		return !createInformation.Usage.IsSet(TextureUsageFlag::StoreOnCPUWithGPUAccess) && createInformation.InitialData == nullptr;
	}

	/** Returns true if a buffer described by @p createInformation is only accessed by the GPU. */
	bool IsGpuOnly(const GpuBufferCreateInformation& createInformation)
	{
		const bool isStaging = createInformation.Type == GpuBufferType::StagingRead || createInformation.Type == GpuBufferType::StagingWrite;
		return !isStaging && !createInformation.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess);
	}
}

GpuTransientTexture GpuTransientScope::AllocateTexture(const TextureCreateInformation& createInformation, u32 firstSubmission)
{
	GpuTransientResourceAllocator& allocator = mAllocator;
	B3D_ASSERT(allocator.mTimeline != nullptr && "Transient resources can only be allocated within a scope.");

	GpuTransientTexture output;
	if(!B3D_ENSURE_LOG(IsGpuOnly(createInformation), "Transient texture '{0}' must only be accessed by the GPU.", createInformation.Name))
		return output;

	// D3D12 copy queues cannot transition texture layouts, which the alias acquire before the first use requires
	if(!B3D_ENSURE_LOG(allocator.mTimeline->GetQueue(firstSubmission).GetType() != GQT_TRANSFER, "The first use of transient texture '{0}' cannot be on a transfer queue.", createInformation.Name))
		return output;

	auto fnIsSame = [&createInformation](const GpuTransientResourceAllocator::Resource& resource)
	{
		return GpuTransientResourceAllocator::IsSameTexture(resource.TextureDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	if(!GpuTransientResourceAllocator::FindCachedMemoryRequirements(allocator.mCachedTextures, fnIsSame, memoryRequirements))
		memoryRequirements = allocator.mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return output;

	auto fnCreate = [&allocator, &createInformation](GpuTransientResourceAllocator::Resource& resource, const GpuAllocation* location)
	{
		resource.Texture = location != nullptr ? allocator.mDevice.CreateTexture(createInformation, *location, GpuObjectCreateFlag::Aliased) : allocator.mDevice.CreateTexture(createInformation);
		resource.TextureDescription = createInformation;
		return resource.Texture != nullptr;
	};

	const GpuTransientResourceAllocator::Resource* resource = allocator.AllocateResource(allocator.mCachedTextures, memoryRequirements, firstSubmission, output.Acquire, output.LatestSourcePass, fnCreate, fnIsSame);
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
	if(!B3D_ENSURE_LOG(IsGpuOnly(createInformation), "Transient buffers must only be accessed by the GPU."))
		return output;

	auto fnIsSame = [&createInformation](const GpuTransientResourceAllocator::Resource& resource)
	{
		return GpuTransientResourceAllocator::IsSameBuffer(resource.BufferDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	if(!GpuTransientResourceAllocator::FindCachedMemoryRequirements(allocator.mCachedBuffers, fnIsSame, memoryRequirements))
		memoryRequirements = allocator.mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return output;

	auto fnCreate = [&allocator, &createInformation](GpuTransientResourceAllocator::Resource& resource, const GpuAllocation* location)
	{
		resource.Buffer = location != nullptr ? allocator.mDevice.CreateGpuBuffer(createInformation, *location, GpuObjectCreateFlag::Aliased) : allocator.mDevice.CreateGpuBuffer(createInformation);
		resource.BufferDescription = createInformation;
		return resource.Buffer != nullptr;
	};

	const GpuTransientResourceAllocator::Resource* resource = allocator.AllocateResource(allocator.mCachedBuffers, memoryRequirements, firstSubmission, output.Acquire, output.LatestSourcePass, fnCreate, fnIsSame);
	if(resource == nullptr)
		return GpuTransientBuffer();

	output.Buffer = resource->Buffer;
	return output;
}

GpuTransientPlacement GpuTransientScope::PlaceTexture(const render::Texture& texture, u32 firstSubmission)
{
	GpuTransientResourceAllocator& allocator = mAllocator;
	B3D_ASSERT(allocator.mTimeline != nullptr && "Transient resources can only be placed within a scope.");

	// D3D12 copy queues cannot transition texture layouts, which the alias acquire before the first use requires
	if(!B3D_ENSURE_LOG(allocator.mTimeline->GetQueue(firstSubmission).GetType() != GQT_TRANSFER, "The first use of transient texture '{0}' cannot be on a transfer queue.", texture.GetName()))
		return GpuTransientPlacement();

	return allocator.PlaceResource(allocator.mCachedTextures, &texture, firstSubmission);
}

GpuTransientPlacement GpuTransientScope::PlaceBuffer(const render::GpuBuffer& buffer, u32 firstSubmission)
{
	B3D_ASSERT(mAllocator.mTimeline != nullptr && "Transient resources can only be placed within a scope.");

	return mAllocator.PlaceResource(mAllocator.mCachedBuffers, &buffer, firstSubmission);
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

	// Unplaced resources that were never cached have no memory to release
	for(const auto& entry : mUnplacedResources)
	{
		if(entry.second->Pool == nullptr)
			mResourcePool.Release(entry.second);
	}

	for(Resource* resource : mCachedTextures)
		EvictResource(resource);

	for(Resource* resource : mCachedBuffers)
		EvictResource(resource);
}

TShared<render::Texture> GpuTransientResourceAllocator::GetOrCreateTexture(const TextureCreateInformation& createInformation)
{
	if(!B3D_ENSURE_LOG(IsGpuOnly(createInformation), "Transient texture '{0}' must only be accessed by the GPU.", createInformation.Name))
		return nullptr;

	auto fnIsSame = [&createInformation](const Resource& resource)
	{
		return IsSameTexture(resource.TextureDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	if(!FindCachedMemoryRequirements(mCachedTextures, fnIsSame, memoryRequirements))
		memoryRequirements = mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	auto fnCreate = [this, &createInformation](Resource& resource)
	{
		resource.Texture = mDevice.CreateTexture(createInformation, GpuObjectCreateFlag::DeferredInitialize);
		resource.TextureDescription = createInformation;
		return resource.Texture != nullptr;
	};

	const Resource* resource = GetOrCreateResource(mCachedTextures, memoryRequirements, fnCreate, fnIsSame);
	if(resource == nullptr)
		return nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
	resource->Texture->SetName(createInformation.Name);
#endif

	return resource->Texture;
}

TShared<render::GpuBuffer> GpuTransientResourceAllocator::GetOrCreateBuffer(const GpuBufferCreateInformation& createInformation)
{
	if(!B3D_ENSURE_LOG(IsGpuOnly(createInformation), "Transient buffers must only be accessed by the GPU."))
		return nullptr;

	auto fnIsSame = [&createInformation](const Resource& resource)
	{
		return IsSameBuffer(resource.BufferDescription, createInformation);
	};

	GpuMemoryRequirements memoryRequirements;
	if(!FindCachedMemoryRequirements(mCachedBuffers, fnIsSame, memoryRequirements))
		memoryRequirements = mDevice.GetMemoryRequirements(createInformation);

	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	auto fnCreate = [this, &createInformation](Resource& resource)
	{
		resource.Buffer = mDevice.CreateGpuBuffer(createInformation, GpuObjectCreateFlag::DeferredInitialize);
		resource.BufferDescription = createInformation;
		return resource.Buffer != nullptr;
	};

	const Resource* resource = GetOrCreateResource(mCachedBuffers, memoryRequirements, fnCreate, fnIsSame);
	return resource != nullptr ? resource->Buffer : nullptr;
}

void GpuTransientResourceAllocator::Return(const render::Texture& texture)
{
	ReturnResource(mCachedTextures, &texture, false);
}

void GpuTransientResourceAllocator::Return(const render::GpuBuffer& buffer)
{
	ReturnResource(mCachedBuffers, &buffer, false);
}

void GpuTransientResourceAllocator::Detach(const render::Texture& texture)
{
	ReturnResource(mCachedTextures, &texture, true);
}

void GpuTransientResourceAllocator::Detach(const render::GpuBuffer& buffer)
{
	ReturnResource(mCachedBuffers, &buffer, true);
}

GpuTransientScope& GpuTransientResourceAllocator::BeginScope(const GpuSubmissionTimeline& timeline)
{
	B3D_ASSERT(mTimeline == nullptr && "Only one transient resource scope may be open at a time.");

	// Release cached resources that went unused for too long, along with their heap references
	const u64 frameIndex = mDevice.GetFrameIndex();
	const u32 idleFrames = gGpuTransientIdleFrames;
	auto fnEvictIdleResources = [this, frameIndex, idleFrames](TArray<Resource*>& cachedResources)
	{
		u32 keptCount = 0;
		for(u32 resourceIndex = 0; resourceIndex < cachedResources.Size(); resourceIndex++)
		{
			Resource* resource = cachedResources[resourceIndex];
			if(!resource->IsHeld && frameIndex - resource->LastRequestFrame >= idleFrames)
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
	mPlacementCount = 0;
	mCacheHits = 0;
	mCacheMisses = 0;
	mRelocations = 0;
	mAliasingAllocator.BeginScope(timeline);

	return mScope;
}

void GpuTransientResourceAllocator::EndScope()
{
	B3D_ASSERT(mTimeline != nullptr && "No transient resource scope is open.");

	// The scope's work is synchronized once the scope ends, so a resource that was not released no longer uses its memory
	for(const auto& entry : mPlacedResources)
	{
		Resource* resource = entry.second;
		B3D_ENSURE_LOG(false, "A transient resource was not released before the end of the scope that placed it.");

		if(resource->Pool == nullptr)
			mResourcePool.Release(resource);
	}

	mPlacedResources.clear();

	for(Resource* resource : mScopeHeldResources)
		resource->IsHeld = false;

	mScopeHeldResources.Clear();

	mLastScopeStatistics.LastScope = mAliasingAllocator.GetStatistics();
	mLastScopeStatistics.PlacementCount = mPlacementCount;
	mLastScopeStatistics.CacheHits = mCacheHits;
	mLastScopeStatistics.CacheMisses = mCacheMisses;
	mLastScopeStatistics.Relocations = mRelocations;

	mAliasingAllocator.EndScope();
	mTimeline = nullptr;

	// Release the least recently used resources beyond the cache size. The cache keeps its order, which decides the resources
	// GetOrCreateTexture() and GetOrCreateBuffer() return.
	auto fnEvictExcessResources = [this](TArray<Resource*>& cachedResources, u32 maximumCount)
	{
		// Each pass evicts the least recently used resource that is not held. Ties go to the one cached first.
		while(cachedResources.Size() > maximumCount)
		{
			auto evicted = cachedResources.End();
			for(auto iterator = cachedResources.Begin(); iterator != cachedResources.End(); ++iterator)
			{
				const Resource* resource = *iterator;
				if(resource->IsHeld)
					continue;

				if(evicted == cachedResources.End() || resource->LastRequestFrame < (*evicted)->LastRequestFrame ||
					(resource->LastRequestFrame == (*evicted)->LastRequestFrame && resource->PlacementScope < (*evicted)->PlacementScope))
				{
					evicted = iterator;
				}
			}

			if(evicted == cachedResources.End())
				break;

			Resource* resource = *evicted;
			cachedResources.Erase(evicted);
			EvictResource(resource);
		}
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
	u32 firstSubmission, render::GpuAliasAcquire& outAcquire, TOptional<u32>& outLatestSourcePass, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame)
{
	mPlacementCount++;

	// Memory types the device cannot alias use persistent memory instead
	IGpuTransientHeapPool* pool = gGpuTransientResources ? mDevice.GetTransientHeapPool(memoryRequirements.MemoryType) : nullptr;

	// A cached resource is reused if the memory it was created at is free in the scope. Held resources are excluded, so every
	// placement of a scope is a separate resource.
	Resource* resource = nullptr;
	GpuAllocation allocation;
	if(pool != nullptr)
	{
		// TODO - Linear scan over the whole cache, with a full description compare per entry. It is done twice per allocation,
		// here and in GpuTransientScope::AllocateTexture()/AllocateBuffer() to find the memory requirements. Look up cached
		// resources by a description hash instead.
		for(Resource* cachedResource : cachedResources)
		{
			if(cachedResource->IsHeld || cachedResource->Pool != pool || !fnIsSame(*cachedResource))
				continue;

			if(mAliasingAllocator.TryAllocateAt(*pool, cachedResource->Allocation, firstSubmission, allocation, outAcquire, &outLatestSourcePass))
			{
				resource = cachedResource;
				break;
			}
		}

		if(resource == nullptr && !mAliasingAllocator.TryAllocate(*pool, memoryRequirements.Size, memoryRequirements.Alignment, firstSubmission, allocation, outAcquire, &outLatestSourcePass))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate {0} bytes of transient memory of type {1}. Using persistent memory instead.", memoryRequirements.Size, memoryRequirements.MemoryType);
			pool = nullptr;
		}
	}

	if(resource != nullptr)
		mCacheHits++;
	else
	{
		resource = mResourcePool.Allocate();
		resource->MemoryRequirements = memoryRequirements;
		resource->Pool = pool;

		if(!fnCreate(*resource, pool != nullptr ? &allocation : nullptr))
		{
			// The memory was never used, so it keeps the uses of the resources released on it before
			if(pool != nullptr)
				mAliasingAllocator.Release(allocation, TArrayView<const GpuTransientLastUse>(), nullptr);

			mResourcePool.Release(resource);
			return nullptr;
		}

		if(pool != nullptr)
		{
			pool->AddHeapReference(allocation.Heap);
			cachedResources.Add(resource);
			mCacheMisses++;
		}
	}

	NotifyResourcePlaced(*resource, allocation, outAcquire);
	return resource;
}

template<class CreateFunction, class IsSameFunction>
GpuTransientResourceAllocator::Resource* GpuTransientResourceAllocator::GetOrCreateResource(TArray<Resource*>& cachedResources, const GpuMemoryRequirements& memoryRequirements, CreateFunction&& fnCreate, IsSameFunction&& fnIsSame)
{
	// The cache is in the order resources were cached, so a sequence of requests that repeats gets the same resources every time
	Resource* resource = nullptr;
	for(Resource* cachedResource : cachedResources)
	{
		if(!cachedResource->IsHeld && fnIsSame(*cachedResource))
		{
			resource = cachedResource;
			break;
		}
	}

	// A new resource is created once its memory is placed
	if(resource == nullptr)
	{
		resource = mResourcePool.Allocate();
		resource->MemoryRequirements = memoryRequirements;

		if(!fnCreate(*resource))
		{
			mResourcePool.Release(resource);
			return nullptr;
		}
	}

	resource->IsHeld = true;
	resource->LastRequestFrame = mDevice.GetFrameIndex();
	mUnplacedResources[GetResourceKey(*resource)] = resource;

#if B3D_BUILD_TYPE_DEVELOPMENT
	SetUnplaced(*resource, true);
#endif

	return resource;
}

GpuTransientPlacement GpuTransientResourceAllocator::PlaceResource(TArray<Resource*>& cachedResources, const void* resourceKey, u32 firstSubmission)
{
	GpuTransientPlacement output;

	const auto found = mUnplacedResources.find(resourceKey);
	if(!B3D_ENSURE_LOG(found != mUnplacedResources.end(), "Only an unplaced transient resource can be placed."))
		return output;

	Resource& resource = *found->second;
	mUnplacedResources.erase(found);
	mPlacementCount++;

	const GpuMemoryRequirements& memoryRequirements = resource.MemoryRequirements;
	const auto fnSetAllocation = [&resource](const GpuAllocation& allocation, GpuObjectCreateFlags flags)
	{
		if(resource.Texture != nullptr)
			resource.Texture->SetAllocation(allocation, flags);
		else
			resource.Buffer->SetAllocation(allocation, flags);
	};

	// Memory types the device cannot alias use persistent memory instead
	IGpuTransientHeapPool* const pool = gGpuTransientResources ? mDevice.GetTransientHeapPool(memoryRequirements.MemoryType) : nullptr;

	// A cached resource keeps its native resource if the memory it was created at is free in the scope. The caller already uses
	// the resource, so another cached resource at free memory cannot replace it.
	const bool isCached = resource.Pool != nullptr;
	bool isAtCachedMemory = false;
	bool hasMemory = false;
	GpuAllocation allocation;
	if(pool != nullptr)
	{
		isAtCachedMemory = resource.Pool == pool && mAliasingAllocator.TryAllocateAt(*pool, resource.Allocation, firstSubmission, allocation, output.Acquire, &output.LatestSourcePass);
		hasMemory = isAtCachedMemory || mAliasingAllocator.TryAllocate(*pool, memoryRequirements.Size, memoryRequirements.Alignment, firstSubmission, allocation, output.Acquire, &output.LatestSourcePass);

		if(!hasMemory)
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate {0} bytes of transient memory of type {1}. Using persistent memory instead.", memoryRequirements.Size, memoryRequirements.MemoryType);
	}

	if(isAtCachedMemory)
		mCacheHits++;
	else if(hasMemory)
	{
		// Reference the new heap before releasing the old one, which may be the same heap
		pool->AddHeapReference(allocation.Heap);
		if(isCached)
		{
			resource.Pool->ReleaseHeap(resource.Allocation.Heap);
			mRelocations++;
		}
		else
		{
			cachedResources.Add(&resource);
			mCacheMisses++;
		}

		resource.Pool = pool;
		fnSetAllocation(allocation, GpuObjectCreateFlag::Aliased);
	}
	else if(isCached)
	{
		cachedResources.Erase(std::find(cachedResources.Begin(), cachedResources.End(), &resource));
		resource.Pool->ReleaseHeap(resource.Allocation.Heap);
		resource.Pool = nullptr;

		fnSetAllocation(GpuAllocation::CreatePending(mDevice.GetPersistentAllocator(memoryRequirements.MemoryType)), GpuObjectCreateFlag::None);
	}

	output.Recreated = isCached && !isAtCachedMemory;

	// A resource that was not cached has no native resource yet
	if(!isCached)
	{
		if(resource.Texture != nullptr)
			resource.Texture->Initialize();
		else
			resource.Buffer->Initialize();
	}

	NotifyResourcePlaced(resource, allocation, output.Acquire);
	return output;
}

void GpuTransientResourceAllocator::NotifyResourcePlaced(Resource& resource, const GpuAllocation& allocation, render::GpuAliasAcquire& acquire)
{
	resource.Allocation = allocation;
	resource.PlacementScope = mScopeIndex;
	resource.LastRequestFrame = mDevice.GetFrameIndex();
	resource.IsHeld = true;
	mPlacedResources[GetResourceKey(resource)] = &resource;

	if(resource.Pool != nullptr)
		mScopeHeldResources.Add(&resource);

#if B3D_BUILD_TYPE_DEVELOPMENT
	SetUnplaced(resource, false);
#endif

	// A cached resource may be placed over memory it released itself, and an acquire must not supersede the resource it acquires
	IGpuResource* gpuResource = GetGpuResource(resource);

	resource.Predecessors.Clear();
	for(IGpuResource* predecessor : acquire.Predecessors)
	{
		if(predecessor != gpuResource)
			resource.Predecessors.Add(predecessor);
	}

	acquire.Predecessors = TArrayView<IGpuResource* const>(resource.Predecessors.Data(), resource.Predecessors.Size());
}

void GpuTransientResourceAllocator::ReleaseResource(const void* resourceKey, TArrayView<const GpuTransientLastUse> lastUses)
{
	B3D_ASSERT(mTimeline != nullptr && "Transient resources can only be released within a scope.");

	const auto found = mPlacedResources.find(resourceKey);
	if(!B3D_ENSURE_LOG(found != mPlacedResources.end(), "Only a transient resource placed in the open scope can be released."))
		return;

	Resource* resource = found->second;
	mPlacedResources.erase(found);

	if(resource->Pool == nullptr)
	{
		mResourcePool.Release(resource);
		return;
	}

	mAliasingAllocator.Release(resource->Allocation, lastUses, GetGpuResource(*resource));
}

void GpuTransientResourceAllocator::ReturnResource(TArray<Resource*>& cachedResources, const void* resourceKey, bool detach)
{
	const auto found = mUnplacedResources.find(resourceKey);
	if(!B3D_ENSURE_LOG(found != mUnplacedResources.end(), "Only an unplaced transient resource can be returned or detached."))
		return;

	Resource* resource = found->second;
	mUnplacedResources.erase(found);

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(detach)
		SetUnplaced(*resource, false);
#endif

	// A resource that was not cached has no memory, and nothing the allocator keeps
	if(resource->Pool == nullptr)
	{
		mResourcePool.Release(resource);
		return;
	}

	if(!detach)
	{
		resource->IsHeld = false;
		return;
	}

	// The caller moves the resource off the memory before the next scope can place other resources on it
	cachedResources.Erase(std::find(cachedResources.Begin(), cachedResources.End(), resource));
	EvictResource(resource);
}

void GpuTransientResourceAllocator::EvictResource(Resource* resource)
{
	// The resource's native destruction waits for the GPU to finish using it. The heap is destroyed even later, once it went
	// unused for gGpuTransientIdleFrames frames and the GPU finished the frame that released it.
	resource->Pool->ReleaseHeap(resource->Allocation.Heap);
	mResourcePool.Release(resource);
}

template<class IsSameFunction>
bool GpuTransientResourceAllocator::FindCachedMemoryRequirements(const TArray<Resource*>& cachedResources, IsSameFunction&& fnIsSame, GpuMemoryRequirements& outMemoryRequirements)
{
	for(const Resource* cachedResource : cachedResources)
	{
		if(fnIsSame(*cachedResource))
		{
			outMemoryRequirements = cachedResource->MemoryRequirements;
			return true;
		}
	}

	return false;
}

IGpuResource* GpuTransientResourceAllocator::GetGpuResource(const Resource& resource)
{
	return resource.Texture != nullptr ? resource.Texture->GetGpuResource() : resource.Buffer->GetGpuResource();
}

const void* GpuTransientResourceAllocator::GetResourceKey(const Resource& resource)
{
	return resource.Texture != nullptr ? (const void*)resource.Texture.get() : (const void*)resource.Buffer.get();
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void GpuTransientResourceAllocator::SetUnplaced(const Resource& resource, bool unplaced)
{
	if(resource.Texture != nullptr)
		resource.Texture->SetUnplaced(unplaced);
	else
		resource.Buffer->SetUnplaced(unplaced);
}
#endif

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
