//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuSubmitThread.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "Material/B3DShaderCompiler.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

GpuDevice::GpuDevice()
	: mVertexInputManager(B3DMakeUnique<render::GpuVertexInputManager>(*this)), mMemoryDefragmenter(*this)
{ }

GpuDevice::~GpuDevice()
{
	B3D_ASSERT(mTransientHeapPools.Empty() && "The backend must destroy its transient heap pools before its heap backends.");
	B3D_ASSERT(mVertexInputManager == nullptr && "The backend must destroy its vertex input manager before its resource manager.");
	B3D_ASSERT(mBindlessHeaps == nullptr && "The backend must destroy the bindless heaps before its native heaps.");
	B3D_ASSERT(mPendingUnpins.Empty() && "The backend must release the unpinned resources before its resource manager.");
}

TShared<GpuProgramBytecode> GpuDevice::CompileGpuProgramBytecode(const GpuProgramCreateInformation& createInformation) const
{
	if(!IsGpuProgramLanguageSupported(createInformation.Language))
		return nullptr;

	const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(createInformation.Language);
	if(bytecodeCompiler == nullptr)
		return nullptr;

	return bytecodeCompiler->CompileBytecode(createInformation);
}

TUnique<IGpuAllocator> GpuDevice::CreateScratchAllocator(u32 /*memoryType*/, IGpuCompletionTracker& /*completionTracker*/)
{
	// Default: context-owned scratch allocation is unsupported. Backends that support it override this.
	return nullptr;
}

IGpuTransientHeapPool* GpuDevice::GetTransientHeapPool(u32 /*memoryType*/)
{
	// Default: transient resources use persistent memory. Backends that support aliasing override this.
	return nullptr;
}

TArrayView<IGpuTransientHeapPool* const> GpuDevice::GetTransientHeapPools() const
{
	return TArrayView<IGpuTransientHeapPool* const>(mTransientHeapPools.Data(), mTransientHeapPools.Size());
}

void GpuDevice::EndFrame()
{
	ASSERT_IF_NOT_RENDER_THREAD

	// Before the frame boundary, so the next frame's work on every queue waits for heap patches through the frame fence
	if(mBindlessHeaps != nullptr)
		mBindlessHeaps->ApplyPendingChanges();

	// Signal end-of-frame to the submit thread. This blocks until the previous frame's resources are safe to reuse.
	if(mSubmitThread != nullptr)
		mSubmitThread->QueueEndFrameAndWaitForPreviousFrame();

	for(IGpuTransientHeapPool* pool : mTransientHeapPools)
		pool->ReclaimUnused();

	mFrameCompletionTracker.AdvanceFrame();

	if(mBindlessHeaps != nullptr)
		mBindlessHeaps->ProcessReleases();

	ReleaseUnpinnedResources(false);
}

void GpuDevice::PinResource(IGpuResource& resource)
{
	resource.NotifyPinned();
}

void GpuDevice::UnpinResource(IGpuResource& resource)
{
	Lock lock(mPendingUnpinMutex);
	mPendingUnpins.Add(PendingUnpin(mFrameCompletionTracker.GetCurrentMarker(), &resource));
}

void GpuDevice::ReleaseUnpinnedResources(bool isGpuIdle)
{
	FrameAllocatorScope frameScope;
	FrameVector<IGpuResource*> unpinnedResources;
	{
		Lock lock(mPendingUnpinMutex);

		u32 completeUnpinCount = 0;
		for(const PendingUnpin& unpin : mPendingUnpins)
		{
			if(!isGpuIdle && !mFrameCompletionTracker.IsMarkerComplete(unpin.FrameIndex))
				break;

			completeUnpinCount++;
		}

		unpinnedResources.reserve(completeUnpinCount);
		for(u32 unpinIndex = 0; unpinIndex < completeUnpinCount; unpinIndex++)
			unpinnedResources.push_back(mPendingUnpins[unpinIndex].Resource);

		mPendingUnpins.Erase(mPendingUnpins.begin(), mPendingUnpins.begin() + completeUnpinCount);
	}

	// Unpinning may destroy the resource, so it runs without the lock held
	for(IGpuResource* resource : unpinnedResources)
		resource->NotifyUnpinned();
}

bool GpuDevice::CreateBindlessHeaps(TUnique<GpuBindlessHeaps> heaps)
{
	B3D_ASSERT(mBindlessHeaps == nullptr);

	mBindlessHeaps = std::move(heaps);
	if(!mBindlessHeaps->Initialize())
	{
		mBindlessHeaps = nullptr;
		return false;
	}

	// Creating the global samplers allocates their slots through GetBindlessHeaps()
	mBindlessHeaps->CreateGlobalSamplers();
	return true;
}

void GpuDevice::GetBindlessResourceCapacities(u32 deviceMaximumCapacity, u32& outCapacity, u32& outMaximumCapacity)
{
	outMaximumCapacity = std::min(gGpuBindlessResourceMaximumCapacity.Get(), deviceMaximumCapacity);
	outCapacity = std::min(gGpuBindlessResourceCapacity.Get(), outMaximumCapacity);
}

u32 GpuDevice::EvictUnusedSamplerStates()
{
	// Destroying sampler states releases their bindless slots, which must happen without the cache lock held
	TInlineArray<TShared<SamplerState>, 16> evictedSamplerStates;
	{
		Lock lock(mSamplerStateMutex);

		for(auto iter = mCachedSamplerStates.begin(); iter != mCachedSamplerStates.end();)
		{
			if(iter->second.use_count() == 1)
			{
				evictedSamplerStates.Add(std::move(iter->second));
				iter = mCachedSamplerStates.erase(iter);
			}
			else
				++iter;
		}
	}

	return (u32)evictedSamplerStates.Size();
}

IGpuTransientHeapPool* GpuDevice::AddTransientHeapPool(TUnique<IGpuTransientHeapPool> pool)
{
	B3D_ASSERT(pool != nullptr);

	mTransientHeapPools.Add(pool.release());
	return mTransientHeapPools.Back();
}

void GpuDevice::DestroyTransientHeapPools()
{
	for(IGpuTransientHeapPool* pool : mTransientHeapPools)
		B3DDelete(pool);

	mTransientHeapPools.Clear();
}

TShared<render::GpuSplitBarrier> GpuDevice::CreateSplitBarrier(TOptional<GpuQueueType> releaseQueue, TOptional<GpuQueueType> acquireQueue)
{
	return B3DMakeShared<render::GpuSplitBarrier>(releaseQueue, acquireQueue);
}

namespace
{
	/**
	 * Checks that a resource with @p memoryRequirements can be created at @p allocation with @p flags, logging the reason
	 * if it can't.
	 */
	bool ValidateResourceAllocation(const GpuAllocation& allocation, const GpuMemoryRequirements& memoryRequirements, GpuObjectCreateFlags flags)
	{
		if(!B3D_ENSURE_LOG(allocation.HasMemory() || allocation.IsPending(), "Cannot create a GPU resource at an empty allocation."))
			return false;

		if(!B3D_ENSURE_LOG(!flags.IsSet(GpuObjectCreateFlag::Aliased) || allocation.HasMemory(), "Only a GPU resource created at a fixed memory location can be aliased."))
			return false;

		if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
			return false;

		if(!allocation.HasMemory())
			return true;

		return B3D_ENSURE_LOG(allocation.Size >= memoryRequirements.Size && allocation.Offset % memoryRequirements.Alignment == 0,
			"Allocation (offset {0}, size {1}) does not satisfy the resource's requirements (size {2}, alignment {3}).",
			allocation.Offset, allocation.Size, memoryRequirements.Size, memoryRequirements.Alignment);
	}

	/** Checks that a bindless resource can be created on @p device with @p flags, logging the reason if it can't. */
	bool ValidateBindlessResource(const GpuDevice& device, const String& name, GpuObjectCreateFlags flags)
	{
		if(!B3D_ENSURE_LOG(device.GetBindlessHeaps() != nullptr, "Cannot create bindless resource '{0}', the GPU device doesn't support bindless resources.", name))
			return false;

		return B3D_ENSURE_LOG(!flags.IsSet(GpuObjectCreateFlag::Aliased), "Bindless resource '{0}' cannot be aliased. Bindless resources must stay at rest.", name);
	}

	/** Checks that a texture described by @p createInformation can be created on @p device with @p flags, logging the reason if it can't. */
	bool ValidateTexture(const GpuDevice& device, const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
	{
		if(!createInformation.Usage.IsSet(TextureUsageFlag::Bindless))
			return true;

		return ValidateBindlessResource(device, createInformation.Name, flags);
	}

	/** Checks that a buffer described by @p createInformation can be created on @p device with @p flags, logging the reason if it can't. */
	bool ValidateGpuBuffer(const GpuDevice& device, const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
	{
		const GpuBufferType type = createInformation.Type;
		if(type == GpuBufferType::AccelerationStructure)
		{
			if(!B3D_ENSURE_LOG(device.GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES), "Cannot create an acceleration structure buffer, the GPU device doesn't support acceleration structures."))
				return false;

			if(!B3D_ENSURE_LOG(createInformation.AccelerationStructure.Size > 0, "An acceleration structure buffer must have a size."))
				return false;

			const GpuBufferFlags otherFlags = createInformation.Flags & ~GpuBufferFlags(GpuBufferFlag::StoreOnGPU | GpuBufferFlag::Bindless);
			if(!B3D_ENSURE_LOG(createInformation.Flags.IsSet(GpuBufferFlag::StoreOnGPU) && otherFlags == GpuBufferFlags(),
				"An acceleration structure buffer must be created with GpuBufferFlag::StoreOnGPU, and no other flag than GpuBufferFlag::Bindless."))
			{
				return false;
			}

			if(!B3D_ENSURE_LOG(!flags.IsSet(GpuObjectCreateFlag::Aliased) && createInformation.SuballocationCount <= 1, "An acceleration structure buffer cannot be aliased or have suballocations."))
				return false;

			if(!createInformation.Flags.IsSet(GpuBufferFlag::Bindless))
				return true;

			if(!B3D_ENSURE_LOG(createInformation.AccelerationStructure.Type == GpuAccelerationStructureType::TopLevel, "Only a top level acceleration structure buffer can be bindless."))
				return false;

			if(!B3D_ENSURE_LOG(device.GetCapabilities().RayTracing.SupportsBindlessAccelerationStructures, "Cannot create a bindless acceleration structure buffer, the GPU device doesn't support them."))
				return false;

			static const String kAccelerationStructureName = "<acceleration structure>";
			return ValidateBindlessResource(device, kAccelerationStructureName, flags);
		}

		if(createInformation.Flags.IsSet(GpuBufferFlag::AccelerationStructureInput))
		{
			if(!B3D_ENSURE_LOG(device.GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES), "Cannot create an acceleration structure input buffer, the GPU device doesn't support acceleration structures."))
				return false;

			const bool isInputType = type == GpuBufferType::Vertex || type == GpuBufferType::Index || type == GpuBufferType::StructuredStorage;
			if(!B3D_ENSURE_LOG(isInputType, "An acceleration structure input buffer must be a vertex, index or structured storage buffer."))
				return false;
		}

		if(!createInformation.Flags.IsSet(GpuBufferFlag::Bindless))
			return true;

		// Buffers are named after creation
		static const String kBufferName = "<buffer>";
		if(!ValidateBindlessResource(device, kBufferName, flags))
			return false;

		// These buffers have no suballocations, so each one has a single view
		const bool isStorage = type == GpuBufferType::SimpleStorage || type == GpuBufferType::StructuredStorage;
		return B3D_ENSURE_LOG(isStorage || type == GpuBufferType::Vertex || type == GpuBufferType::Index, "A bindless buffer must be a storage, vertex or index buffer.");
	}
}

TShared<render::Texture> GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	const GpuMemoryRequirements memoryRequirements = GetMemoryRequirements(createInformation);
	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	const GpuAllocation allocation = GpuAllocation::CreatePending(GetPersistentAllocator(memoryRequirements.MemoryType));
	if(!ValidateResourceAllocation(allocation, memoryRequirements, flags))
		return nullptr;

	if(!ValidateTexture(*this, createInformation, flags))
		return nullptr;

	return CreateTextureInternal(createInformation, allocation, flags);
}

TShared<render::Texture> GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
{
	if(!ValidateResourceAllocation(allocation, GetMemoryRequirements(createInformation), flags))
		return nullptr;

	if(!ValidateTexture(*this, createInformation, flags))
		return nullptr;

	return CreateTextureInternal(createInformation, allocation, flags);
}

TShared<render::GpuBuffer> GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	// Before the memory requirements, which backends only answer for buffers they can create
	if(!ValidateGpuBuffer(*this, createInformation, flags))
		return nullptr;

	const GpuMemoryRequirements memoryRequirements = GetMemoryRequirements(createInformation);
	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	const GpuAllocation allocation = GpuAllocation::CreatePending(GetPersistentAllocator(memoryRequirements.MemoryType));
	if(!ValidateResourceAllocation(allocation, memoryRequirements, flags))
		return nullptr;

	return CreateGpuBufferInternal(createInformation, allocation, flags);
}

TShared<render::GpuBuffer> GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
{
	// Before the memory requirements, which backends only answer for buffers they can create
	if(!ValidateGpuBuffer(*this, createInformation, flags))
		return nullptr;

	if(!ValidateResourceAllocation(allocation, GetMemoryRequirements(createInformation), flags))
		return nullptr;

	return CreateGpuBufferInternal(createInformation, allocation, flags);
}

bool GpuDevice::GetAccelerationStructureSizes(const GpuAccelerationStructureInputs& inputs, GpuAccelerationStructureSizes& outSizes) const
{
	outSizes = GpuAccelerationStructureSizes();

	if(!B3D_ENSURE_LOG(GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES), "Cannot query acceleration structure sizes, the GPU device doesn't support acceleration structures."))
		return false;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(!render::GpuAccelerationStructure::ValidateInputs(*this, inputs))
		return false;
#endif

	return true;
}

bool GpuDevice::WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance> instances, void* destination) const
{
	if(!B3D_ENSURE_LOG(GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES), "Cannot write ray tracing instances, the GPU device doesn't support acceleration structures."))
		return false;

#if B3D_BUILD_TYPE_DEVELOPMENT
	const u64 maximumInstanceCount = GetCapabilities().RayTracing.MaximumInstanceCount;
	if(!B3D_ENSURE_LOG(instances.size() <= maximumInstanceCount, "Cannot write {0} ray tracing instances, the device supports at most {1}.", instances.size(), maximumInstanceCount))
		return false;

	static constexpr u32 kMaximum24BitValue = (1u << 24) - 1;
	const GpuRayTracingInstanceFlags forcedOpacity = GpuRayTracingInstanceFlag::ForceOpaque | GpuRayTracingInstanceFlag::ForceNonOpaque;
	for(u32 instanceIndex = 0; instanceIndex < (u32)instances.size(); instanceIndex++)
	{
		const GpuRayTracingInstance& instance = instances[instanceIndex];
		if(!B3D_ENSURE_LOG(instance.InstanceId <= kMaximum24BitValue && instance.HitGroupIndexOffset <= kMaximum24BitValue,
			"Ray tracing instance {0} has instance id {1} and hit group index offset {2}. Both must fit in 24 bits.", instanceIndex, instance.InstanceId, instance.HitGroupIndexOffset))
		{
			return false;
		}

		if(!B3D_ENSURE_LOG((instance.Flags & forcedOpacity) != forcedOpacity, "Ray tracing instance {0} cannot force its geometry to be both opaque and non-opaque.", instanceIndex))
			return false;

		const bool hasBottomLevel = instance.BottomLevel != nullptr && instance.BottomLevel->GetBuffer() != nullptr;
		if(!B3D_ENSURE_LOG(hasBottomLevel && instance.BottomLevel->GetType() == GpuAccelerationStructureType::BottomLevel,
			"Ray tracing instance {0} must reference a bottom level acceleration structure.", instanceIndex))
		{
			return false;
		}
	}
#endif

	return true;
}

render::GpuStageFlags GpuDevice::GetSupportedStageFlags() const
{
	using namespace render;
	const GpuDeviceCapabilities& capabilities = GetCapabilities();

	GpuStageFlags stages = GpuStageFlag::All;
	if(!capabilities.HasCapability(RSC_RAY_TRACING_PIPELINE))
		stages &= ~GpuStageFlags(GpuStageFlag::RayTracingShaderNonUniform | GpuStageFlag::RayTracingShaderUniform | GpuStageFlag::RayTracingShaderAccelerationStructure);

	// Without inline ray tracing, only ray tracing programs trace rays
	if(!capabilities.HasCapability(RSC_RAY_QUERY))
		stages &= ~GpuStageFlags(GpuStageFlag::VertexShaderAccelerationStructure | GpuStageFlag::FragmentShaderAccelerationStructure | GpuStageFlag::ComputeShaderAccelerationStructure);

	if(!capabilities.HasCapability(RSC_ACCELERATION_STRUCTURES))
		stages &= ~GpuStageFlags(GpuStageFlag::AccelerationStructureBuild | GpuStageFlag::AccelerationStructureBuildInput);

	return stages;
}

void GpuDevice::DoForEachQueue(const std::function<void(GpuQueue&)>&& callback) const
{
	for(u32 queueTypeIndex = 0; queueTypeIndex < GQT_COUNT; queueTypeIndex++)
	{
		const GpuQueueType queueType = (GpuQueueType)queueTypeIndex;

		const u32 queueCount = GetQueueCount(queueType);
		for(u32 queueIndex = 0; queueIndex < queueCount; queueIndex++)
		{
			const TShared<GpuQueue>& queue = GetQueue(queueType, queueIndex);
			callback(*queue);
		}
	}
}

void GpuDevice::PresentRenderWindow(const TShared<render::RenderWindow>& renderWindow, GpuQueueMask syncMask)
{
	const TShared<GpuQueue> queue = GetQueue(GQT_GRAPHICS, 0);
	if(!B3D_ENSURE(queue != nullptr))
		return;

	queue->PresentRenderWindow(renderWindow, syncMask);
}

TShared<SamplerState> GpuDevice::FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation)
{
	Lock lock(mSamplerStateMutex);

	if (auto found = mCachedSamplerStates.find(createInformation); found != mCachedSamplerStates.end())
	{
		TShared<SamplerState> existingSamplerState = found->second;
		if (existingSamplerState != nullptr)
			return existingSamplerState;
	}

	TShared<SamplerState> newSamplerState = CreateSamplerState(createInformation);
	mCachedSamplerStates[createInformation] = newSamplerState;

	return newSamplerState;
}

