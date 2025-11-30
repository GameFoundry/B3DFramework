//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderableUniformBufferManager.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"

using namespace b3d;
using namespace b3d::render;

void RenderableUniformBufferManager::Initialize(GpuDevice& device)
{
	mDevice = &device;

	const u32 perObjectSize = gPerObjectUniformDefinition.GetSize();
	GpuBufferCreateInformation perObjectCreateInfo = GpuBufferCreateInformation::CreateUniform(perObjectSize, GpuBufferFlag::StoreOnGPU);

	mRenderablePool.Initialize(device, perObjectCreateInfo, kRenderableEntriesPerBuffer, 1);

	const u32 decalSize = gDecalParamDef.GetSize();
	GpuBufferCreateInformation decalCreateInfo = GpuBufferCreateInformation::CreateUniform(decalSize, GpuBufferFlag::StoreOnGPU);

	mDecalPool.Initialize(device, decalCreateInfo, kDecalEntriesPerBuffer, 1);
}

RenderableUniformBufferManager::RenderableAllocation RenderableUniformBufferManager::AllocateForRenderable(const SPtr<GpuPipelineParameterLayout>& layout)
{
	RenderableAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), nullptr, layout);

	GpuParameterBinding binding = layout->GetBinding("PerObject");
	if(binding.IsValid())
		result.PerObjectDynamicOffsetIndex = layout->GetDynamicOffsetIndex(binding.Set, binding.Slot);

	return result;
}

RenderableUniformBufferManager::DecalAllocation RenderableUniformBufferManager::AllocateForDecal(const SPtr<GpuPipelineParameterLayout>& layout)
{
	DecalAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.DecalSuballocation = mDecalPool.Allocate();

	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), result.DecalSuballocation.GetBuffer(), layout);

	GpuParameterBinding perObjectBinding = layout->GetBinding("PerObject");
	if(perObjectBinding.IsValid())
		result.PerObjectDynamicOffsetIndex = layout->GetDynamicOffsetIndex(perObjectBinding.Set, perObjectBinding.Slot);

	GpuParameterBinding decalBinding = layout->GetBinding("DecalParams");
	if(decalBinding.IsValid())
		result.DecalDynamicOffsetIndex = layout->GetDynamicOffsetIndex(decalBinding.Set, decalBinding.Slot);

	return result;
}

void RenderableUniformBufferManager::Release(const RenderableAllocation& allocation)
{
	if(!allocation.PerObjectSuballocation.IsValid())
		return;

	ReleaseParameterSet(allocation.PerObjectSuballocation.GetBuffer(), nullptr);
	mRenderablePool.Release(allocation.PerObjectSuballocation);
}

void RenderableUniformBufferManager::Release(const DecalAllocation& allocation)
{
	if(!allocation.PerObjectSuballocation.IsValid())
		return;

	ReleaseParameterSet(allocation.PerObjectSuballocation.GetBuffer(), allocation.DecalSuballocation.GetBuffer());

	mRenderablePool.Release(allocation.PerObjectSuballocation);

	if(allocation.DecalSuballocation.IsValid())
		mDecalPool.Release(allocation.DecalSuballocation);
}

SPtr<render::GpuParameterSet> RenderableUniformBufferManager::GetOrCreateParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer, const SPtr<GpuPipelineParameterLayout>& layout)
{
	BufferKey key = { perObjectBuffer.get(), decalBuffer ? decalBuffer.get() : nullptr };

	auto iter = mParameterSetsByBuffer.find(key);
	if(iter != mParameterSetsByBuffer.end())
	{
		iter->second.RefCount++;
		return iter->second.ParameterSet;
	}

	SPtr<GpuParameterSet> parameterSet = mDevice->CreateGpuParameterSet(layout, 1);

	parameterSet->SetUniformBuffer("PerObject", perObjectBuffer, 0);

	if(decalBuffer != nullptr)
		parameterSet->SetUniformBuffer("DecalParams", decalBuffer, 0);

	BufferParameterSetEntry entry;
	entry.ParameterSet = parameterSet;
	entry.RefCount = 1;

	mParameterSetsByBuffer[key] = entry;

	return parameterSet;
}

void RenderableUniformBufferManager::ReleaseParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer)
{
	BufferKey key = { perObjectBuffer.get(), decalBuffer ? decalBuffer.get() : nullptr };

	auto iter = mParameterSetsByBuffer.find(key);
	if(iter == mParameterSetsByBuffer.end())
		return;

	iter->second.RefCount--;
	if(iter->second.RefCount == 0)
		mParameterSetsByBuffer.erase(iter);
}
