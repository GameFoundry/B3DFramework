//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuSubmitThread.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Material/B3DShaderCompiler.h"

using namespace b3d;

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

#if B3D_GPU_EXPLICIT_BARRIERS
TShared<render::GpuSplitBarrier> GpuDevice::CreateSplitBarrier(TOptional<GpuQueueType> releaseQueue, TOptional<GpuQueueType> acquireQueue)
{
	return B3DMakeShared<render::GpuSplitBarrier>(releaseQueue, acquireQueue);
}
#endif

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
}

TShared<render::Texture> GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, GpuObjectCreateFlags flags)
{
	const GpuMemoryRequirements memoryRequirements = GetMemoryRequirements(createInformation);
	if(memoryRequirements.MemoryType == GpuMemoryRequirements::kUnsupportedMemoryType)
		return nullptr;

	const GpuAllocation allocation = GpuAllocation::CreatePending(GetPersistentAllocator(memoryRequirements.MemoryType));
	if(!ValidateResourceAllocation(allocation, memoryRequirements, flags))
		return nullptr;

	return CreateTextureInternal(createInformation, allocation, flags);
}

TShared<render::Texture> GpuDevice::CreateTexture(const TextureCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
{
	if(!ValidateResourceAllocation(allocation, GetMemoryRequirements(createInformation), flags))
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

	return CreateGpuBufferInternal(createInformation, allocation, flags);
}

TShared<render::GpuBuffer> GpuDevice::CreateGpuBuffer(const GpuBufferCreateInformation& createInformation, const GpuAllocation& allocation, GpuObjectCreateFlags flags)
{
	if(!ValidateResourceAllocation(allocation, GetMemoryRequirements(createInformation), flags))
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

