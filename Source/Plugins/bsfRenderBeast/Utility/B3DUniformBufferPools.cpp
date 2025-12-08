//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DUniformBufferPools.h"
#include "B3DRendererObject.h"
#include "B3DRendererRenderable.h"
#include "B3DRendererDecal.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuDevice.h"

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

	// Initialize staging pool (shared across all types)
	const u32 perObjectSize = gPerObjectUniformDefinition.GetSize();
	GpuBufferCreateInformation stagingCreateInfo;
	stagingCreateInfo.Type = GpuBufferType::StagingWrite;
	stagingCreateInfo.Staging.Size = perObjectSize;
	stagingCreateInfo.Flags = GpuBufferFlag::AllowWriteCachingOnCPU; // TODO - Only while GpuUniformBuffer doesn't support non-cached writes

	mStagingPool.Initialize(device, stagingCreateInfo, kStagingEntriesPerBuffer, 1);

	// Create pool groups from pending configurations
	for (const PoolConfiguration& config : mPendingConfigurations)
	{
		PoolGroup group;
		group.Type = config.Type;
		group.EntriesPerBuffer = config.EntriesPerBuffer;
		group.ParameterSetLayout = config.Layout;

		// Create pools for each configured buffer type
		for (const BufferConfiguration& poolConfig : config.Buffers)
		{
			GpuBufferCreateInformation createInfo = GpuBufferCreateInformation::CreateUniform(poolConfig.BufferSize, poolConfig.Flags);
			GpuBufferPool pool;
			pool.Initialize(device, createInfo, config.EntriesPerBuffer, 1);

			group.Pools.Add(std::move(pool));
			group.UniformBufferNames.Add(poolConfig.UniformBufferParameterName);
		}

		const u32 typeIndex = (u32)config.Type;
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
		entry.Suballocations.Add(group.Pools[poolIndex].Allocate());

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

	SPtr<render::GpuParameterSet> parameterSet = mDevice->CreateGpuParameterSet(group.ParameterSetLayout, GpuPipelineSet::kPerObject);

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

	const GpuBufferSuballocation& destination = object.PerObjectSuballocation;
	actualCommandBuffer->CopyBufferToBuffer(staging.GetBuffer(), destination.GetBuffer(), staging.GetSuballocationOffset(), destination.GetSuballocationOffset(), staging.GetSize());
}

void UniformBufferPools::AdvanceFrame()
{
	mStagingPool.AdvanceFrame();
}
