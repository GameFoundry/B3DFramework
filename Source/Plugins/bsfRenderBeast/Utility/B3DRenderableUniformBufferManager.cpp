//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderableUniformBufferManager.h"
#include "B3DRendererObject.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Math/B3DMath.h"

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

	// PerObject uniform buffer info (shared by both layouts)
	GpuUniformBufferInformation perObjectInfo;
	perObjectInfo.Name = "PerObject";
	perObjectInfo.Set = GpuPipelineSet::kPerObject;
	perObjectInfo.Slot = 0;
	perObjectInfo.Size = Math::CeilToMultiple(gPerObjectUniformDefinition.GetSize() / 4u, 4u);
	perObjectInfo.Stages = GpuProgramStageBit::Vertex | GpuProgramStageBit::Fragment;
	perObjectInfo.IsShareable = true;

	// Create renderable layout (PerObject only)
	{
		GpuProgramParameterDescription renderableDescription;
		renderableDescription.UniformBuffers["PerObject"] = perObjectInfo;
		mRenderableParameterSetLayout = device.CreateGpuPipelineParameterSetLayout(renderableDescription);
	}

	// Create decal layout (PerObject + DecalParams)
	{
		GpuProgramParameterDescription decalDescription;
		decalDescription.UniformBuffers["PerObject"] = perObjectInfo;

		GpuUniformBufferInformation decalInfo;
		decalInfo.Name = "DecalParams";
		decalInfo.Set = GpuPipelineSet::kPerObject;
		decalInfo.Slot = 1;
		decalInfo.Size = Math::CeilToMultiple(gDecalParamDef.GetSize() / 4u, 4u);
		decalInfo.Stages = GpuProgramStageBit::Fragment;
		decalInfo.IsShareable = true;
		decalDescription.UniformBuffers["DecalParams"] = decalInfo;

		mDecalParameterSetLayout = device.CreateGpuPipelineParameterSetLayout(decalDescription);
	}
}

RenderableUniformBufferManager::RenderableAllocation RenderableUniformBufferManager::AllocateForRenderable()
{
	RenderableAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), nullptr);

	const u32 slot = mRenderableParameterSetLayout->GetSlot("PerObject");
	if(slot != ~0u)
		result.PerObjectDynamicOffsetIndex = mRenderableParameterSetLayout->GetDynamicOffsetIndex(slot);

	return result;
}

RenderableUniformBufferManager::DecalAllocation RenderableUniformBufferManager::AllocateForDecal()
{
	DecalAllocation result;

	result.PerObjectSuballocation = mRenderablePool.Allocate();
	result.DecalSuballocation = mDecalPool.Allocate();

	result.SharedParameterSet = GetOrCreateParameterSet(result.PerObjectSuballocation.GetBuffer(), result.DecalSuballocation.GetBuffer());

	const u32 perObjectSlot = mDecalParameterSetLayout->GetSlot("PerObject");
	if(perObjectSlot != ~0u)
		result.PerObjectDynamicOffsetIndex = mDecalParameterSetLayout->GetDynamicOffsetIndex(perObjectSlot);

	const u32 decalSlot = mDecalParameterSetLayout->GetSlot("DecalParams");
	if(decalSlot != ~0u)
		result.DecalDynamicOffsetIndex = mDecalParameterSetLayout->GetDynamicOffsetIndex(decalSlot);

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

SPtr<render::GpuParameterSet> RenderableUniformBufferManager::GetOrCreateParameterSet(const SPtr<GpuBuffer>& perObjectBuffer, const SPtr<GpuBuffer>& decalBuffer)
{
	BufferKey key = { perObjectBuffer.get(), decalBuffer ? decalBuffer.get() : nullptr };

	auto iter = mParameterSetsByBuffer.find(key);
	if(iter != mParameterSetsByBuffer.end())
	{
		iter->second.RefCount++;
		return iter->second.ParameterSet;
	}

	const bool isDecal = decalBuffer != nullptr;
	const SPtr<GpuPipelineParameterSetLayout>& layout = isDecal ? mDecalParameterSetLayout : mRenderableParameterSetLayout;

	SPtr<GpuParameterSet> parameterSet = mDevice->CreateGpuParameterSet(layout, GpuPipelineSet::kPerObject);

	parameterSet->SetUniformBuffer("PerObject", perObjectBuffer, 0);

	if(isDecal)
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
