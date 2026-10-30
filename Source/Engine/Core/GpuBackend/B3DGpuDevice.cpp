//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuSubmitThread.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuVertexInputManager.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/Allocators/B3DGpuTransientHeapPool.h"
#include "Material/B3DShaderCompiler.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

GpuDevice::GpuDevice()
	: mVertexInputManager(B3DMakeUnique<render::GpuVertexInputManager>(*this))
{ }

GpuDevice::~GpuDevice()
{
	B3D_ASSERT(mTransientHeapPools.Empty() && "The backend must destroy its transient heap pools before its heap backends.");
	B3D_ASSERT(mVertexInputManager == nullptr && "The backend must destroy its vertex input manager before its resource manager.");
	B3D_ASSERT(mBindlessHeaps == nullptr && "The backend must destroy the bindless heaps before its native heaps.");
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

	// Signal end-of-frame to the submit thread. This blocks until the previous frame's resources are safe to reuse.
	if(mSubmitThread != nullptr)
		mSubmitThread->QueueEndFrameAndWaitForPreviousFrame();

	for(IGpuTransientHeapPool* pool : mTransientHeapPools)
		pool->ReclaimUnused();

	mFrameCompletionTracker.AdvanceFrame();

	if(mBindlessHeaps != nullptr)
		mBindlessHeaps->ProcessReleases();
}

void GpuDevice::CreateBindlessHeaps(IGpuBindlessHeap& backend, u32 resourceCapacity, u32 samplerCapacity)
{
	B3D_ASSERT(mBindlessHeaps == nullptr);

	mBindlessHeaps = B3DMakeUnique<GpuBindlessHeaps>(*this, backend, mFrameCompletionTracker, resourceCapacity, samplerCapacity);
	mBindlessHeaps->CreateGlobalSamplers();
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

		return B3D_ENSURE_LOG(!flags.IsSet(GpuObjectCreateFlag::Aliased), "Bindless resource '{0}' cannot be aliased. Bindless resources must stay at rest at a fixed location.", name);
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
		if(!createInformation.Flags.IsSet(GpuBufferFlag::Bindless))
			return true;

		// Buffers are named after creation
		static const String kBufferName = "<buffer>";
		if(!ValidateBindlessResource(device, kBufferName, flags))
			return false;

		// Storage buffers have no suballocations, so each one has a single view
		return B3D_ENSURE_LOG(createInformation.Type == GpuBufferType::SimpleStorage || createInformation.Type == GpuBufferType::StructuredStorage,
			"A bindless buffer must be a simple or structured storage buffer.");
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
	const GpuMemoryRequirements memoryRequirements = GetMemoryRequirements(createInformation);
	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	const GpuAllocation allocation = GpuAllocation::CreatePending(GetPersistentAllocator(memoryRequirements.MemoryType));
	if(!ValidateResourceAllocation(allocation, memoryRequirements, flags))
		return nullptr;

	if(!ValidateGpuBuffer(*this, createInformation, flags))
		return nullptr;

	return CreateGpuBufferInternal(createInformation, allocation, flags);
}

TShared<render::GpuBuffer> GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
{
	if(!ValidateResourceAllocation(allocation, GetMemoryRequirements(createInformation), flags))
		return nullptr;

	if(!ValidateGpuBuffer(*this, createInformation, flags))
		return nullptr;

	return CreateGpuBufferInternal(createInformation, allocation, flags);
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

