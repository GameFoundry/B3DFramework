//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUniformBufferPools.h"
#include "B3DRendererObject.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"
#include "B3DRendererParticles.h"
#include "Components/B3DDecal.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Shading/B3DGpuParticleSimulation.h"

using namespace b3d;
using namespace b3d::render;

void UniformBufferPools::RegisterType(const PoolConfiguration& config)
{
	B3D_ASSERT(!mInitialized);
	mPendingConfigurations.push_back(config);
}

void UniformBufferPools::Initialize(GpuDevice& device)
{
	B3D_ASSERT(!mInitialized);
	mDevice = &device;

	// Initialize per-object staging pool
	{
		const u32 perObjectSize = gPerObjectUniformDefinition.GetSize();
		GpuBufferCreateInformation stagingCreateInfo;
		stagingCreateInfo.Type = GpuBufferType::StagingWrite;
		stagingCreateInfo.Staging.Size = perObjectSize;
		stagingCreateInfo.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;

		mPerObjectStagingPool.Initialize(device, stagingCreateInfo, kStagingEntriesPerBuffer, 1);
	}

	// Initialize decal param staging pool
	{
		const u32 decalParamSize = gDecalParamDef.GetSize();
		GpuBufferCreateInformation stagingCreateInfo;
		stagingCreateInfo.Type = GpuBufferType::StagingWrite;
		stagingCreateInfo.Staging.Size = decalParamSize;
		stagingCreateInfo.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;

		mDecalStagingPool.Initialize(device, stagingCreateInfo, kStagingEntriesPerBuffer, 1);
	}

	// Initialize GPU particles param staging pool
	{
		const u32 gpuParticlesParamSize = gGpuParticlesParamDef.GetSize();
		GpuBufferCreateInformation stagingCreateInfo;
		stagingCreateInfo.Type = GpuBufferType::StagingWrite;
		stagingCreateInfo.Staging.Size = gpuParticlesParamSize;
		stagingCreateInfo.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;

		mGpuParticlesStagingPool.Initialize(device, stagingCreateInfo, kStagingEntriesPerBuffer, 1);
	}

	// Create pool groups from pending configurations
	for (const PoolConfiguration& poolConfiguration : mPendingConfigurations)
	{
		PoolGroup group;
		group.Type = poolConfiguration.Type;
		group.EntriesPerBuffer = poolConfiguration.EntriesPerBuffer;
		group.ParameterSetLayout = poolConfiguration.Layout;

		// Create pools for each configured buffer type
		for (const BufferConfiguration& bufferConfiguration : poolConfiguration.Buffers)
		{
			GpuBufferCreateInformation createInfo = GpuBufferCreateInformation::CreateUniform(bufferConfiguration.BufferSize, bufferConfiguration.Flags);
			GpuBufferPool pool;
			pool.Initialize(device, createInfo, poolConfiguration.EntriesPerBuffer, 1);

			group.Pools.Add(std::move(pool));
			group.PoolBufferTypes.Add(bufferConfiguration.Type);
			group.UniformBufferNames.Add(bufferConfiguration.UniformBufferParameterName);
		}

		const u32 typeIndex = (u32)poolConfiguration.Type;
		if (typeIndex >= mPoolGroups.Size())
			mPoolGroups.Resize(typeIndex + 1);

		mPoolGroups[typeIndex] = std::move(group);
	}

	mPendingConfigurations.clear();
	mInitialized = true;
}

UniformBufferPools::AllocationResult UniformBufferPools::Allocate(PoolType type)
{
	AllocationResult result;

	const u32 typeIndex = (u32)type;
	if (typeIndex >= mPoolGroups.Size())
	{
		B3D_ASSERT(false && "Allocation type not registered");
		return result;
	}

	PoolGroup& group = mPoolGroups[typeIndex];

	// Allocate or reuse an entry index
	u32 entryIndex;
	if (group.FreeListHead != PoolGroup::kInvalidIndex)
	{
		entryIndex = group.FreeListHead;
		group.FreeListHead = group.Entries[entryIndex].NextFreeIndex;
	}
	else
	{
		entryIndex = (u32)group.Entries.size();
		group.Entries.push_back({});
	}

	AllocationEntry& entry = group.Entries[entryIndex];
	entry.NextFreeIndex = PoolGroup::kInvalidIndex;
	entry.IsAllocated = true;

	// Allocate from all pools in the group
	entry.Suballocations.Clear();
	for (u32 poolIndex = 0; poolIndex < group.Pools.Size(); ++poolIndex)
	{
		B3D_ASSERT(poolIndex == (u32)group.PoolBufferTypes[poolIndex]);
		entry.Suballocations.Add(group.Pools[poolIndex].Allocate());
	}

	// Get or create shared parameter set
	result.ParameterSet = GetOrCreateParameterSet(group, entry);

	// Copy suballocations to result
	result.Suballocations = entry.Suballocations;

	// Set up handle
	result.Handle.Index = entryIndex;
	result.Handle.Type = type;

	return result;
}

void UniformBufferPools::Release(AllocationHandle handle)
{
	if (!handle.IsValid())
		return;

	const u32 typeIndex = (u32)handle.Type;
	if (typeIndex >= mPoolGroups.Size())
		return;

	PoolGroup& group = mPoolGroups[typeIndex];

	B3D_ASSERT(handle.Index < group.Entries.size());

	AllocationEntry& entry = group.Entries[handle.Index];

	if (!entry.IsAllocated)
		return;

	// Release parameter set
	ReleaseParameterSet(group, entry);

	// Release all pool allocations
	for (u32 poolIndex = 0; poolIndex < entry.Suballocations.Size(); ++poolIndex)
	{
		if (entry.Suballocations[poolIndex].IsValid())
			group.Pools[poolIndex].Release(entry.Suballocations[poolIndex]);
	}

	// Clear entry and add to free-list
	entry.Suballocations.Clear();
	entry.IsAllocated = false;
	entry.NextFreeIndex = group.FreeListHead;
	group.FreeListHead = handle.Index;
}

UniformBufferPools::BufferKey UniformBufferPools::BuildBufferKey(PoolType type, const AllocationEntry& entry) const
{
	BufferKey key;
	key.Type = type;
	for (const GpuBufferSuballocation& suballocation : entry.Suballocations)
		key.Buffers.Add(suballocation.GetBuffer().get());

	return key;
}

SPtr<render::GpuParameterSet> UniformBufferPools::GetOrCreateParameterSet(PoolGroup& group, const AllocationEntry& entry)
{
	BufferKey key = BuildBufferKey(group.Type, entry);

	auto iter = group.ParameterSetsByBuffer.find(key);
	if (iter != group.ParameterSetsByBuffer.end())
	{
		iter->second.RefCount++;
		return iter->second.ParameterSet;
	}

	SPtr<GpuParameterSet> parameterSet = mDevice->CreateGpuParameterSet(group.ParameterSetLayout, GpuPipelineSet::kPerObject);

	// Bind all buffers by their uniform buffer names
	for (u32 poolIndex = 0; poolIndex < entry.Suballocations.Size(); ++poolIndex)
	{
		const GpuBufferSuballocation& suballocation = entry.Suballocations[poolIndex];
		const String& uniformBufferName = group.UniformBufferNames[poolIndex];
		parameterSet->SetUniformBuffer(uniformBufferName, suballocation.GetBuffer(), 0);
	}

	BufferParameterSetEntry parameterSetEntry;
	parameterSetEntry.ParameterSet = parameterSet;
	parameterSetEntry.RefCount = 1;

	group.ParameterSetsByBuffer[key] = parameterSetEntry;

	return parameterSet;
}

void UniformBufferPools::ReleaseParameterSet(PoolGroup& group, const AllocationEntry& entry)
{
	BufferKey key = BuildBufferKey(group.Type, entry);

	auto iter = group.ParameterSetsByBuffer.find(key);
	if (iter == group.ParameterSetsByBuffer.end())
		return;

	iter->second.RefCount--;
	if (iter->second.RefCount == 0)
		group.ParameterSetsByBuffer.erase(iter);
}

void UniformBufferPools::UpdatePerObjectBuffer(const RendererObject& object, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if (!object.PerObjectSuballocation.IsValid())
		return;

	GpuBufferMappedScope staging = mPerObjectStagingPool.Allocate().Map();

	gPerObjectUniformDefinition.gMatWorld.Set(staging, object.WorldTransform);
	gPerObjectUniformDefinition.gMatInvWorld.Set(staging, object.WorldTransform.InverseAffine());
	gPerObjectUniformDefinition.gMatWorldNoScale.Set(staging, object.WorldNoScale);
	gPerObjectUniformDefinition.gMatInvWorldNoScale.Set(staging, object.WorldNoScale.InverseAffine());
	gPerObjectUniformDefinition.gMatPrevWorld.Set(staging, object.PrevWorldTransform);
	gPerObjectUniformDefinition.gWorldDeterminantSign.Set(staging, object.WorldTransform.Determinant3x3() >= 0.0f ? 1.0f : -1.0f);
	gPerObjectUniformDefinition.gLayer.Set(staging, (i32)object.Layer);

	staging.Unmap();

	const SPtr<GpuCommandBuffer>& actualCommandBuffer = commandBuffer ? commandBuffer : mDevice->GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer();

	const GpuBufferSuballocation& source = staging.GetSuballocation();
	const GpuBufferSuballocation& destination = object.PerObjectSuballocation;
	actualCommandBuffer->CopyBufferToBuffer(source.GetBuffer(), destination.GetBuffer(), source.GetSuballocationOffset(), destination.GetSuballocationOffset(), source.GetSize());
}

void UniformBufferPools::UpdateDecalParamBuffer(const RendererDecal& decal, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if (!decal.DecalParamSuballocation.IsValid())
		return;

	const Transform& tfrm = decal.Decal->GetWorldTransform();

	const Vector2 size = decal.Decal->GetWorldSize();
	const Vector2 extent = size * 0.5f;
	const float maxDistance = decal.Decal->GetWorldMaxDistance();

	const Matrix4 view = Matrix4::View(tfrm.GetPosition(), tfrm.GetRotation());
	const Matrix4 proj = Matrix4::ProjectionOrthographic(-extent.X, extent.X, -extent.Y, extent.Y, 0.0f, maxDistance);

	const Matrix4 worldToDecal = proj * view;
	const Vector3 decalNormal = -tfrm.GetRotation().ZAxis();
	const float normalTolerance = -0.05f;

	float flipDerivatives = 1.0f;
	const GpuBackendConventions& gpuBackendConventions = mDevice->GetCapabilities().Conventions;
	if (gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up)
		flipDerivatives = -1.0f;

	GpuBufferMappedScope staging = mDecalStagingPool.Allocate().Map();

	gDecalParamDef.gWorldToDecal.Set(staging, worldToDecal);
	gDecalParamDef.gDecalNormal.Set(staging, decalNormal);
	gDecalParamDef.gNormalTolerance.Set(staging, normalTolerance);
	gDecalParamDef.gFlipDerivatives.Set(staging, flipDerivatives);
	gDecalParamDef.gLayerMask.Set(staging, (i32)decal.Decal->GetLayerMask());

	staging.Unmap();

	const SPtr<GpuCommandBuffer>& actualCommandBuffer = commandBuffer ? commandBuffer : mDevice->GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer();

	const GpuBufferSuballocation& source = staging.GetSuballocation();
	const GpuBufferSuballocation& destination = decal.DecalParamSuballocation;
	actualCommandBuffer->CopyBufferToBuffer(source.GetBuffer(), destination.GetBuffer(), source.GetSuballocationOffset(), destination.GetSuballocationOffset(), source.GetSize());
}

void UniformBufferPools::UpdateGpuParticlesParamBuffer(const RendererParticles& particles, const SPtr<GpuCommandBuffer>& commandBuffer)
{
	if (!particles.GpuParticlesParamSuballocation.IsValid())
		return;

	const Vector2 colorCurveOffset = GpuParticleCurves::GetUvOffset(particles.ColorCurveAlloc);
	const float colorCurveScale = GpuParticleCurves::GetUvScale(particles.ColorCurveAlloc);
	const Vector2 sizeScaleFrameIdxCurveOffset = GpuParticleCurves::GetUvOffset(particles.SizeScaleFrameIdxCurveAlloc);
	const float sizeScaleFrameIdxCurveScale = GpuParticleCurves::GetUvScale(particles.SizeScaleFrameIdxCurveAlloc);

	GpuBufferMappedScope staging = mGpuParticlesStagingPool.Allocate().Map();

	gGpuParticlesParamDef.gColorCurveOffset.Set(staging, colorCurveOffset);
	gGpuParticlesParamDef.gColorCurveScale.Set(staging, Vector2(colorCurveScale, 0.0f));
	gGpuParticlesParamDef.gSizeScaleFrameIdxCurveOffset.Set(staging, sizeScaleFrameIdxCurveOffset);
	gGpuParticlesParamDef.gSizeScaleFrameIdxCurveScale.Set(staging, Vector2(sizeScaleFrameIdxCurveScale, 0.0f));

	staging.Unmap();

	const SPtr<GpuCommandBuffer>& actualCommandBuffer = commandBuffer ? commandBuffer : mDevice->GetQueue(GQT_GRAPHICS, 0)->GetOrCreateTransferCommandBuffer();

	const GpuBufferSuballocation& source = staging.GetSuballocation();
	const GpuBufferSuballocation& destination = particles.GpuParticlesParamSuballocation;
	actualCommandBuffer->CopyBufferToBuffer(source.GetBuffer(), destination.GetBuffer(), source.GetSuballocationOffset(), destination.GetSuballocationOffset(), source.GetSize());
}

void UniformBufferPools::AdvanceFrame()
{
	mPerObjectStagingPool.AdvanceFrame();
	mDecalStagingPool.AdvanceFrame();
	mGpuParticlesStagingPool.AdvanceFrame();
}
