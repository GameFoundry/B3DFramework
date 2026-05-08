//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"

#include "B3DGpuDevice.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "Math/B3DMath.h"
#include "Utility/B3DResult.h"

using namespace b3d;

GpuPipelineParameterSetLayout::GpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
{
	auto fnRegisterUniforms = [this](const auto& entry, u32 arraySize, GpuParameterType type, GpuParameterObjectType objectType, GpuBufferFormat elementType) mutable
	{
		// If entry with the same name exists, ensure it's an exact duplicate we can ignore
		if(auto found = mUniformMap.find(entry.Name); found != mUniformMap.end())
		{
			if(found->second.Type != type)
			{
				B3D_LOG(Warning, LogRenderBackend, "Found a uniform with same name {0}, but a different type. One has type {1}, and other {2}",
					entry.Name, (u32)type, (u32)found->second.Type);
				return;
			}
		
			if(found->second.Set != entry.Set || found->second.Slot != entry.Slot)
			{
				B3D_LOG(Warning, LogRenderBackend, "Found a uniform with same name {0}, but a different set/slot combination. One is using set:{1}, slot:{2}, and other set:{3}, slot:{4}",
					entry.Name, entry.Set, entry.Slot, found->second.Set, found->second.Slot);
				return;
			}

			if(found->second.ArraySize != arraySize)
			{
				B3D_LOG(Warning, LogRenderBackend, "Found a uniform with same name {0}, but a different array size. One has array size {1}, and other {2}",
					entry.Name, arraySize, found->second.ArraySize);
				return;
			}

			found->second.Usage |= entry.Stages;
			return;
		}

		// Ensure provided set/slot combination is valid
		if(entry.Set == ~0u || entry.Slot == ~0u)
		{
			B3D_LOG(Warning, LogRenderBackend, "Invalid set/slot combination provided for uniform {0}. Set: {1}, slot: {2}", entry.Name, entry.Set, entry.Slot);
			return;
		}

		UniformInformation uniformInformation;
		uniformInformation.Name = entry.Name;
		uniformInformation.Type = type;
		uniformInformation.Set = entry.Set;
		uniformInformation.Slot = entry.Slot;
		uniformInformation.ArraySize = arraySize;
		uniformInformation.ObjectType = objectType;
		uniformInformation.ElementType = elementType;
		uniformInformation.Usage |= entry.Stages;

		mUniformMap[entry.Name] = std::move(uniformInformation);
	};

	// Generate a unique list of uniforms from the parameter description
	// NOTE: Generated map must remain immutable after this as we are referencing its elements by pointer

	for(const auto& uniformBuffer : parameterDescription.UniformBuffers)
		fnRegisterUniforms(uniformBuffer.second, 1, GpuParameterType::UniformBuffer, GPOT_UNKNOWN, BF_UNKNOWN);

	for(auto& sampledTexture : parameterDescription.SampledTextures)
		fnRegisterUniforms(sampledTexture.second, sampledTexture.second.ArraySize, GpuParameterType::SampledTexture, sampledTexture.second.Type, sampledTexture.second.ElementType);

	for(auto& storageTexture : parameterDescription.StorageTextures)
		fnRegisterUniforms(storageTexture.second, storageTexture.second.ArraySize, GpuParameterType::StorageTexture, storageTexture.second.Type, storageTexture.second.ElementType);

	for(auto& buffer : parameterDescription.Buffers)
		fnRegisterUniforms(buffer.second, buffer.second.ArraySize, GpuParameterType::StorageBuffer, buffer.second.Type, buffer.second.ElementType);

	for(auto& sampler : parameterDescription.Samplers)
		fnRegisterUniforms(sampler.second, sampler.second.ArraySize, GpuParameterType::Sampler, sampler.second.Type, sampler.second.ElementType);

	// Register uniform information in per-slot and per-type arrays
	for(auto it = mUniformMap.begin(); it != mUniformMap.end();)
	{
		UniformInformation& uniformInformation = it->second;

		// Check if some other entry is using the same set/slot combination
		if(uniformInformation.Slot < mUniforms.size())
		{
			UniformInformation* otherUniformInformation = mUniforms[uniformInformation.Slot];
			if(otherUniformInformation)
			{
				// Duplicate set/slot can be allowed only in the combined texture/sampler case
				const bool isPotentialCombinedSampler = (uniformInformation.Type == GpuParameterType::SampledTexture && otherUniformInformation->Type == GpuParameterType::Sampler || uniformInformation.Type == GpuParameterType::Sampler && otherUniformInformation->Type == GpuParameterType::SampledTexture);
				if(!isPotentialCombinedSampler)
				{
					B3D_LOG(Warning, LogRenderBackend, "Provided set/slot combination for uniform {0} is already in use by {1}. Set: {2}, slot: {3}", it->first, mUniforms[uniformInformation.Slot]->Name, uniformInformation.Set, uniformInformation.Slot);

					mUniformMap.erase(it);

					// Need to start from beginning, as the map pointers could now be out of date
					it = mUniformMap.begin();
					continue;
				}
			}
		}

		while(mUniforms.size() <= uniformInformation.Slot)
			mUniforms.Add(nullptr);

		mUniforms[uniformInformation.Slot] = &uniformInformation;
		mUniformsPerType[(u32)uniformInformation.Type].Add(&uniformInformation);

		++it;
	}

	// Register uniform buffer members
	auto fnRegisterUniformBufferMember = [this](const GpuUniformBufferMemberInformation& entry)
	{
		// Ensure provided set/slot combination is valid
		if(entry.ParentUniformBufferSet == ~0u || entry.ParentUniformBufferSlot == ~0u)
		{
			B3D_LOG(Warning, LogRenderBackend, "Invalid uniform buffer set/slot combination provided for uniform buffer member {0}. Set: {1}, slot: {2}", entry.Name, entry.ParentUniformBufferSet, entry.ParentUniformBufferSlot);
			return;
		}

		// If entry with the same name exists, ensure it's an exact duplicate we can ignore
		if(auto found = mUniformBufferMembers.find(entry.Name); found != mUniformBufferMembers.end())
		{
			if(found->second != entry)
			{
				B3D_LOG(Warning, LogRenderBackend, "Found a uniform member with same name {0}, but different type information.", entry.Name);
			}

			return;
		}

		mUniformBufferMembers[entry.Name] = entry;
	};

	for(auto& uniformBufferMember : parameterDescription.UniformBufferMembers)
		fnRegisterUniformBufferMember(uniformBufferMember.second);

	// Generate sequential indices (per-type iteration for resource counting)
	auto fnCalculateSequentialIndices = [this](GpuParameterType type)
	{
		const u32 typeIndex = (u32)type;
		mResourceCountPerType[typeIndex] = 0;

		u32 sequentialResourceIndex = 0;
		for(u32 sequentialBindingIndex = 0; sequentialBindingIndex < mUniformsPerType[typeIndex].size(); ++sequentialBindingIndex)
		{
			UniformInformation* uniformInformation = mUniformsPerType[typeIndex][sequentialBindingIndex];
			if(uniformInformation == nullptr)
				continue;

			if(type != GpuParameterType::Sampler || uniformInformation->SequentialResourceIndex == ~0u)
			{
				uniformInformation->SequentialBindingIndex = sequentialBindingIndex;
				uniformInformation->SequentialResourceIndex = sequentialResourceIndex;
			}
			else
			{
				uniformInformation->SequentialSamplerBindingIndex = sequentialBindingIndex;
				uniformInformation->SequentialSamplerResourceIndex = sequentialResourceIndex;
			}

			sequentialResourceIndex += uniformInformation->ArraySize;

			mBindingCount++;
			mResourceCountPerType[typeIndex] += uniformInformation->ArraySize;
			mResourceCount += uniformInformation->ArraySize;
		}
	};

	fnCalculateSequentialIndices(GpuParameterType::UniformBuffer);
	fnCalculateSequentialIndices(GpuParameterType::SampledTexture);
	fnCalculateSequentialIndices(GpuParameterType::StorageTexture);
	fnCalculateSequentialIndices(GpuParameterType::StorageBuffer);
	fnCalculateSequentialIndices(GpuParameterType::Sampler);

	// Assign dynamic offset index in slot-order (Vulkan spec requires binding number order). This should match the order in VulkanGpuParameterSet::PrepareForBind
	u32 nextDynamicOffsetIndex = 0;
	for(u32 slotIndex = 0; slotIndex < mUniforms.size(); slotIndex++)
	{
		UniformInformation* uniformInformation = mUniforms[slotIndex];
		if(uniformInformation == nullptr)
			continue;

		bool supportsDynamicOffset = false;
		if(uniformInformation->Type == GpuParameterType::UniformBuffer)
			supportsDynamicOffset = true;
		else if(uniformInformation->Type == GpuParameterType::StorageBuffer)
			supportsDynamicOffset = (uniformInformation->ObjectType == GPOT_STRUCTURED_BUFFER || uniformInformation->ObjectType == GPOT_RWSTRUCTURED_BUFFER);

		if(supportsDynamicOffset)
			uniformInformation->DynamicOffsetIndex = nextDynamicOffsetIndex++;
	}

	mDynamicOffsetCount = nextDynamicOffsetIndex;
}

u32 GpuPipelineParameterSetLayout::GetSequentialResourceIndex(u32 slot, u32 arrayIndex) const
{
#if B3D_DEBUG
	if(slot >= mUniforms.size() || mUniforms[slot] == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Slot index doesn't exist in the set. Requested: {1}.", slot);
		return ~0u;
	}
#endif

	const UniformInformation& uniformInformation = *mUniforms[slot];

#if B3D_DEBUG
	if(arrayIndex >= uniformInformation.ArraySize)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot retrieve sequential resource index. Array index out of range: Valid range: [0, {0}). Requested: {1}.", uniformInformation.ArraySize, arrayIndex);
		return -1;
	}
#endif

	return uniformInformation.SequentialResourceIndex != ~0u ? uniformInformation.SequentialResourceIndex + arrayIndex : ~0u;
}


u32 GpuPipelineParameterSetLayout::GetSequentialBindingIndex(u32 slot) const
{
#if B3D_DEBUG
	if(slot >= mUniforms.size() || mUniforms[slot] == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Slot index doesn't exist in the set. Requested: {0}.", slot);
		return ~0u;
	}
#endif

	return mUniforms[slot]->SequentialBindingIndex;
}

u32 GpuPipelineParameterSetLayout::GetSlot(GpuParameterType type, u32 sequentialBindingIndex) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
	{
		B3D_LOG(Error, LogRenderBackend, "Sequential slot index out of range: Valid range: [0, {0}). Requested: {1}.", mUniformsPerType[(u32)type].size(), sequentialBindingIndex);
		return 0;
	}

	if(!B3D_ENSURE(mUniformsPerType[(u32)type][sequentialBindingIndex]))
		return 0;
#endif

	return mUniformsPerType[(u32)type][sequentialBindingIndex]->Slot;
}

u32 GpuPipelineParameterSetLayout::GetSlot(const StringView& name) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return found->second.Slot;

	return ~0u;
}

u32 GpuPipelineParameterSetLayout::GetArraySize(GpuParameterType type, u32 sequentialBindingIndex) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot retrieve array size. Sequential binding index out of range: Valid range: [0, {0}). Requested: {1}.", mUniformsPerType[(u32)type].size(), sequentialBindingIndex);
		return 0;
	}

	if(!B3D_ENSURE(mUniformsPerType[(u32)type][sequentialBindingIndex]))
		return 0;
#endif

	return mUniformsPerType[(u32)type][sequentialBindingIndex]->ArraySize;
}

u32 GpuPipelineParameterSetLayout::GetDynamicOffsetIndex(u32 slot, u32 arrayIndex) const
{
#if B3D_BUILD_TYPE_DEVELOPMENT
	if(slot >= mUniforms.size() || mUniforms[slot] == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot retrieve dynamic offset index. Slot index doesn't exist in the set. Requested: {0}.", slot);
		return ~0u;
	}
#endif

	const UniformInformation& uniformInformation = *mUniforms[slot];

#if B3D_DEBUG
	if(arrayIndex >= uniformInformation.ArraySize)
	{
		B3D_LOG(Error, LogRenderBackend, "Cannot retrieve dynamic offset index. Array index out of range: Valid range: [0, {0}). Requested: {1}.", uniformInformation.ArraySize, arrayIndex);
		return -1;
	}
#endif

	return uniformInformation.DynamicOffsetIndex + arrayIndex;
}

u32 GpuPipelineParameterSetLayout::GetDynamicOffsetIndex(const StringView& name, u32 arrayIndex) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return GetDynamicOffsetIndex(found->second.Slot, arrayIndex);

	return ~0u;
}

bool GpuPipelineParameterSetLayout::HasUniformOfType(const StringView& name, GpuParameterType type) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return found->second.Type == type;

	return false;
}

const UniformInformation* GpuPipelineParameterSetLayout::TryGetUniformInformation(const StringView& name) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return &found->second;

	return nullptr;
}

const UniformInformation* GpuPipelineParameterSetLayout::TryGetUniformInformation(GpuParameterType type, u32 sequentialBindingIndex) const
{
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
		return nullptr;

	return mUniformsPerType[(u32)type][sequentialBindingIndex];
}

const UniformInformation* GpuPipelineParameterSetLayout::TryGetUniformInformation(u32 slot) const
{
	if(slot >= (u32)mUniforms.size())
		return nullptr;

	return mUniforms[slot];
}

const GpuUniformBufferMemberInformation* GpuPipelineParameterSetLayout::TryGetUniformBufferMemberInformation(const StringView& name) const
{
	if(auto found = mUniformBufferMembers.find(name); found != mUniformBufferMembers.end())
		return &found->second;

	return nullptr;
}

GpuPipelineParameterLayout::GpuPipelineParameterLayout(GpuDevice& device, const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	Array<TShared<GpuProgramParameterDescription>, GPT_COUNT> perProgramParameterDescriptions;
	perProgramParameterDescriptions[GPT_FRAGMENT_PROGRAM] = createInformation.Fragment;
	perProgramParameterDescriptions[GPT_VERTEX_PROGRAM] = createInformation.Vertex;
	perProgramParameterDescriptions[GPT_GEOMETRY_PROGRAM] = createInformation.Geometry;
	perProgramParameterDescriptions[GPT_HULL_PROGRAM] = createInformation.Hull;
	perProgramParameterDescriptions[GPT_DOMAIN_PROGRAM] = createInformation.Domain;
	perProgramParameterDescriptions[GPT_COMPUTE_PROGRAM] = createInformation.Compute;

	// Build per-set parameter descriptions by iterating over all per-program parameter descriptions
	TInlineArray<GpuProgramParameterDescription, 4> perSetParameterDescriptions;
	for(u32 programIndex = 0; programIndex < GPT_COUNT; programIndex++)
	{
		const TShared<GpuProgramParameterDescription>& parameterDescription = perProgramParameterDescriptions[programIndex];
		if(parameterDescription == nullptr)
			continue;

		const GpuProgramStageBit stageBit = (GpuProgramStageBit)(1 << programIndex);

		// Split the program's parameter description by set
		TInlineArray<GpuProgramParameterDescription, 4> programPerSetDescriptions;
		parameterDescription->SplitBySet(programPerSetDescriptions);

		while(perSetParameterDescriptions.size() < programPerSetDescriptions.size())
			perSetParameterDescriptions.Add(GpuProgramParameterDescription());

		// Combine each set's description
		for(u32 setIndex = 0; setIndex < programPerSetDescriptions.size(); setIndex++)
		{
			if(Result result = perSetParameterDescriptions[setIndex].TryCombine(programPerSetDescriptions[setIndex], stageBit); !result.IsSuccessful())
				B3D_LOG(Warning, LogRenderBackend, "{0}", result.GetFullErrorMessage());
		}
	}

	// Create sets
	for(u32 set = 0; set < (u32)perSetParameterDescriptions.Size(); ++set)
		mSets.Add(device.CreateGpuPipelineParameterSetLayout(perSetParameterDescriptions[set]));
}
