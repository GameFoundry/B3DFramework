//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderableUniformBufferManager.h"
#include "B3DRendererObject.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuDevice.h"

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

	GpuBufferCreateInformation stagingCreateInfo;
	stagingCreateInfo.Type = GpuBufferType::StagingWrite;
	stagingCreateInfo.Staging.Size = perObjectSize;
	stagingCreateInfo.Flags = GpuBufferFlag::AllowWriteCachingOnCPU; // TODO - Only while GpuUniformBuffer doesn't support non-cached writes

	mStagingPool.Initialize(device, stagingCreateInfo, kStagingEntriesPerBuffer, 1);
}

RenderableUniformBufferManager::RenderableAllocation RenderableUniformBufferManager::AllocateForRenderable(const SPtr<GpuPipelineParameterSetLayout>& layoutSet)
{
	RenderableAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), nullptr, layoutSet);

	const u32 slot = layoutSet->GetSlot("PerObject");
	if(slot != ~0u)
		result.PerObjectDynamicOffsetIndex = layoutSet->GetDynamicOffsetIndex(slot);

	return result;
}

RenderableUniformBufferManager::DecalAllocation RenderableUniformBufferManager::AllocateForDecal(const SPtr<GpuPipelineParameterSetLayout>& layoutSet)
{
	DecalAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.DecalSuballocation = mDecalPool.Allocate();

	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), result.DecalSuballocation.GetBuffer(), layoutSet);

	const u32 perObjectSlot = layoutSet->GetSlot("PerObject");
	if(perObjectSlot != ~0u)
		result.PerObjectDynamicOffsetIndex = layoutSet->GetDynamicOffsetIndex(perObjectSlot);

	const u32 decalSlot = layoutSet->GetSlot("DecalParams");
	if(decalSlot != ~0u)
		result.DecalDynamicOffsetIndex = layoutSet->GetDynamicOffsetIndex(decalSlot);

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

SPtr<render::GpuParameterSet> RenderableUniformBufferManager::GetOrCreateParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer, const SPtr<GpuPipelineParameterSetLayout>& layoutSet)
{
	BufferKey key = { perObjectBuffer.get(), decalBuffer ? decalBuffer.get() : nullptr };

	auto iter = mParameterSetsByBuffer.find(key);
	if(iter != mParameterSetsByBuffer.end())
	{
		iter->second.RefCount++;
		return iter->second.ParameterSet;
	}

	SPtr<GpuParameterSet> parameterSet = mDevice->CreateGpuParameterSet(layoutSet, 1);

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

void RenderableUniformBufferManager::UpdatePerObjectBuffer(const RendererObject& object, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if(!object.BufferAllocation.PerObjectSuballocation.IsValid())
		return;

	GpuBufferSuballocation staging = mStagingPool.Allocate();

	gPerObjectUniformDefinition.gMatWorld.Set(staging, object.WorldTransform);
	gPerObjectUniformDefinition.gMatInvWorld.Set(staging, object.WorldTransform.InverseAffine());
	gPerObjectUniformDefinition.gMatWorldNoScale.Set(staging, object.WorldNoScale);
	gPerObjectUniformDefinition.gMatInvWorldNoScale.Set(staging, object.WorldNoScale.InverseAffine());
	gPerObjectUniformDefinition.gMatPrevWorld.Set(staging, object.PrevWorldTransform);
	gPerObjectUniformDefinition.gWorldDeterminantSign.Set(staging, object.WorldTransform.Determinant3x3() >= 0.0f ? 1.0f : -1.0f);
	gPerObjectUniformDefinition.gLayer.Set(staging, (i32)object.Layer);

	staging.GetBuffer()->FlushCache(staging.GetSuballocationIndex());

	const SPtr<GpuCommandBuffer>& actualCommandBuffer = commandBuffer ? commandBuffer : mDevice->GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer();

	const GpuBufferSuballocation& destination = object.BufferAllocation.PerObjectSuballocation;
	actualCommandBuffer->CopyBufferToBuffer(staging.GetBuffer(), destination.GetBuffer(), staging.GetSuballocationOffset(), destination.GetSuballocationOffset(), staging.GetSize());
}

void RenderableUniformBufferManager::AdvanceFrame()
{
	mStagingPool.AdvanceFrame();
}
