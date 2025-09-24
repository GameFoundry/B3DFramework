//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuPipelineParameterLayout.h"

#include "BsCoreApplication.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Math/BsMath.h"

using namespace b3d;

GpuPipelineParameterLayout::GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
{
	Array<SPtr<GpuProgramParameterDescription>, GPT_COUNT> perProgramParameterDescriptions;
	perProgramParameterDescriptions[GPT_FRAGMENT_PROGRAM] = createInformation.Fragment;
	perProgramParameterDescriptions[GPT_VERTEX_PROGRAM] = createInformation.Vertex;
	perProgramParameterDescriptions[GPT_GEOMETRY_PROGRAM] = createInformation.Geometry;
	perProgramParameterDescriptions[GPT_HULL_PROGRAM] = createInformation.Hull;
	perProgramParameterDescriptions[GPT_DOMAIN_PROGRAM] = createInformation.Domain;
	perProgramParameterDescriptions[GPT_COMPUTE_PROGRAM] = createInformation.Compute;

	auto fnRegisterUniforms = [this](const auto& entry, u32 arraySize, GpuParameterType type, GpuParameterObjectType objectType, GpuBufferFormat elementType, GpuProgramStageBit stage) {

		// If entry with the same name exists, ensure it's an exact duplicate we can ignore
		if(auto found = mUniformMap.find(entry.Name); found != mUniformMap.end())
		{
			if(found->second.Type != type)
			{
				B3D_LOG(Warning, RenderBackend, "Found a uniform with same name {0}, but a different type. One has type {1}, and other {2}",
					entry.Name, (u32)type, (u32)found->second.Type);
				return;
			}
		
			if(found->second.Set != entry.Set || found->second.Slot != entry.Slot)
			{
				B3D_LOG(Warning, RenderBackend, "Found a uniform with same name {0}, but a different set/slot combination. One is using set:{1}, slot:{2}, and other set:{3}, slot:{4}",
					entry.Name, entry.Set, entry.Slot, found->second.Set, found->second.Slot);
				return;
			}

			if(found->second.ArraySize != arraySize)
			{
				B3D_LOG(Warning, RenderBackend, "Found a uniform with same name {0}, but a different array size. One has array size {1}, and other {2}",
					entry.Name, arraySize, found->second.ArraySize);
				return;
			}

			found->second.Usage |= stage;
			return;
		}

		// Ensure provided set/slot combination is valid
		if(entry.Set == ~0u || entry.Slot == ~0u)
		{
			B3D_LOG(Warning, RenderBackend, "Invalid set/slot combination provided for uniform {0}. Set: {1}, slot: {2}", entry.Name, entry.Set, entry.Slot);
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
		uniformInformation.Usage |= stage;

		mUniformMap[entry.Name] = std::move(uniformInformation);
	};

	auto fnRegisterUniformBufferMember = [this](const GpuDataParameterInformation& entry) {

		// If entry with the same name exists, ensure it's an exact duplicate we can ignore
		if(auto found = mUniformBufferMembers.find(entry.Name); found != mUniformBufferMembers.end())
		{
			if(found->second != entry)
			{
				B3D_LOG(Warning, RenderBackend, "Found a uniform member with same name {0}, but different type information.", entry.Name);
			}

			return;
		}

		// Ensure provided set/slot combination is valid
		if(entry.ParentUniformBufferSet == ~0u || entry.ParentUniformBufferSlot == ~0u)
		{
			B3D_LOG(Warning, RenderBackend, "Invalid uniform buffer set/slot combination provided for uniform buffer member {0}. Set: {1}, slot: {2}", entry.Name, entry.ParentUniformBufferSet, entry.ParentUniformBufferSlot);
			return;
		}

		mUniformBufferMembers[entry.Name] = entry;
	};

	// Generate a unique list of uniforms from all per-program descriptions
	// NOTE: Generated map must remain immutable after this as we are referencing its elements by pointer
	for(u32 parameterDescriptionIndex = 0; parameterDescriptionIndex < GPT_COUNT; parameterDescriptionIndex++)
	{
		const SPtr<GpuProgramParameterDescription>& parameterDescription = perProgramParameterDescriptions[parameterDescriptionIndex];
		if(parameterDescription == nullptr)
			continue;

		const GpuProgramStageBit stageBit = (GpuProgramStageBit)(1 << parameterDescriptionIndex);
		for(const auto& uniformBuffer : parameterDescription->UniformBuffers)
			fnRegisterUniforms(uniformBuffer.second, 1, GpuParameterType::UniformBuffer, GPOT_UNKNOWN, BF_UNKNOWN, stageBit);

		for(auto& sampledTexture : parameterDescription->SampledTextures)
			fnRegisterUniforms(sampledTexture.second, sampledTexture.second.ArraySize, GpuParameterType::SampledTexture, sampledTexture.second.Type, sampledTexture.second.ElementType, stageBit);

		for(auto& storageTexture : parameterDescription->StorageTextures)
			fnRegisterUniforms(storageTexture.second, storageTexture.second.ArraySize, GpuParameterType::StorageTexture, storageTexture.second.Type, storageTexture.second.ElementType,stageBit);

		for(auto& buffer : parameterDescription->Buffers)
			fnRegisterUniforms(buffer.second, buffer.second.ArraySize, GpuParameterType::StorageBuffer, buffer.second.Type, buffer.second.ElementType,stageBit);

		for(auto& sampler : parameterDescription->Samplers)
			fnRegisterUniforms(sampler.second, sampler.second.ArraySize, GpuParameterType::Sampler, sampler.second.Type, sampler.second.ElementType,stageBit);

		for(auto& uniformBufferMember : parameterDescription->UniformBufferMembers)
			fnRegisterUniformBufferMember(uniformBufferMember.second);
	}

	// Register uniform information in per-slot and per-type arrays
	for(auto it = mUniformMap.begin(); it != mUniformMap.end();)
	{
		UniformInformation& uniformInformation = it->second;

		// Check if some other entry is using the same set/slot combination
		if(uniformInformation.Set < mSets.size())
		{
			if(uniformInformation.Slot < mSets[uniformInformation.Set].Uniforms.size())
			{
				UniformInformation* otherUniformInformation = mSets[uniformInformation.Set].Uniforms[uniformInformation.Slot];
				if(otherUniformInformation)
				{
					// Duplicate set/slot can be allowed only in the combined texture/sampler case
					const bool isPotentialCombinedSampler = (uniformInformation.Type == GpuParameterType::SampledTexture && otherUniformInformation->Type == GpuParameterType::Sampler || uniformInformation.Type == GpuParameterType::Sampler && otherUniformInformation->Type == GpuParameterType::SampledTexture);
					if(!isPotentialCombinedSampler)
					{
						B3D_LOG(Warning, RenderBackend, "Provided set/slot combination for uniform {0} is already in use by {1}. Set: {2}, slot: {3}", it->first, mSets[uniformInformation.Set].Uniforms[uniformInformation.Slot]->Name, uniformInformation.Set, uniformInformation.Slot);

						mUniformMap.erase(it);

						// Need to start from beginning, as the map pointers could now be out of date
						it = mUniformMap.begin();
						mSets.Clear();

						for(u32 i = 0; i < GPT_COUNT; i++)
							mUniformsPerType[i].Clear();

						continue;
					}
				}
			}
		}

		while(mSets.size() <= uniformInformation.Set)
			mSets.Add(SetInformation());

		while(mSets[uniformInformation.Set].Uniforms.size() <= uniformInformation.Slot)
			mSets[uniformInformation.Set].Uniforms.Add(nullptr);

		mSets[uniformInformation.Set].Uniforms[uniformInformation.Slot] = &uniformInformation;
		mUniformsPerType[(u32)uniformInformation.Type].Add(&uniformInformation);

		++it;
	}

	// Generate sequential indices and dynamic offset indices
	u32 nextDynamicOffsetIndex = 0;
	auto fnCalculateSequentialIndices = [this, &nextDynamicOffsetIndex](GpuParameterType type, bool supportsDynamicOffset)
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

			if(supportsDynamicOffset)
				uniformInformation->DynamicOffsetIndex = nextDynamicOffsetIndex++;

			sequentialResourceIndex += uniformInformation->ArraySize;

			mBindingCount++;
			mResourceCountPerType[typeIndex] += uniformInformation->ArraySize;
			mResourceCount += uniformInformation->ArraySize;
		}
	};

	fnCalculateSequentialIndices(GpuParameterType::UniformBuffer, true);
	fnCalculateSequentialIndices(GpuParameterType::SampledTexture, false);
	fnCalculateSequentialIndices(GpuParameterType::StorageTexture, false);
	fnCalculateSequentialIndices(GpuParameterType::StorageBuffer, true);
	fnCalculateSequentialIndices(GpuParameterType::Sampler, true);
}

u32 GpuPipelineParameterLayout::GetSequentialResourceIndex(u32 set, u32 slot, u32 arrayIndex) const
{
#if B3D_DEBUG
	if(set >= mSets.size())
	{
		B3D_LOG(Error, RenderBackend, "Set index out of range: Valid range: [0, {0}). Requested: {1}.", mSets.size(), set);
		return ~0u;
	}

	if(slot >= mSets[set].Uniforms.size() || mSets[set].Uniforms[slot] == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Slot index doesn't exist in the set. Requested: {1}.", slot);
		return ~0u;
	}
#endif

	const UniformInformation& uniformInformation = *mSets[set].Uniforms[slot];

#if B3D_DEBUG
	if(arrayIndex >= uniformInformation.ArraySize)
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve sequential resource index. Array index out of range: Valid range: [0, {0}). Requested: {1}.", uniformInformation.ArraySize, arrayIndex);
		return -1;
	}
#endif

	return uniformInformation.SequentialResourceIndex != ~0u ? uniformInformation.SequentialResourceIndex + arrayIndex : ~0u;
}


u32 GpuPipelineParameterLayout::GetSequentialBindingIndex(u32 set, u32 slot) const
{
#if B3D_DEBUG
	if(set >= mSets.size())
	{
		B3D_LOG(Error, RenderBackend, "Set index out of range: Valid range: [0, {0}). Requested: {1}.", mSets.size(), set);
		return ~0u;
	}

	if(slot >= mSets[set].Uniforms.size() || mSets[set].Uniforms[slot] == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Slot index doesn't exist in the set. Requested: {0}.", slot);
		return ~0u;
	}
#endif

	return mSets[set].Uniforms[slot]->SequentialBindingIndex;
}

void GpuPipelineParameterLayout::GetBinding(GpuParameterType type, u32 sequentialBindingIndex, u32& set, u32& slot) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
	{
		B3D_LOG(Error, RenderBackend, "Sequential slot index out of range: Valid range: [0, {0}). Requested: {1}.", mUniformsPerType[(u32)type].size(), sequentialBindingIndex);

		set = 0;
		slot = 0;
		return;
	}

	if(!B3D_ENSURE(mUniformsPerType[(u32)type][sequentialBindingIndex]))
		return;
#endif

	set = mUniformsPerType[(u32)type][sequentialBindingIndex]->Set;
	slot = mUniformsPerType[(u32)type][sequentialBindingIndex]->Slot;
}

GpuParameterBinding GpuPipelineParameterLayout::GetBinding(GpuParameterType type, u32 sequentialBindingIndex) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
	{
		B3D_LOG(Error, RenderBackend, "Sequential slot index out of range: Valid range: [0, {0}). Requested: {1}.", mUniformsPerType[(u32)type].size(), sequentialBindingIndex);
		return GpuParameterBinding();
	}
#endif

	const UniformInformation* uniformInformation = mUniformsPerType[(u32)type][sequentialBindingIndex];
	
#if B3D_DEBUG
	if(!B3D_ENSURE(uniformInformation))
		return GpuParameterBinding();
#endif
	
	return GpuParameterBinding(uniformInformation->Set, uniformInformation->Slot);
}

void GpuPipelineParameterLayout::GetBinding(const String& name, GpuParameterBinding& binding) const
{
	binding = GetBinding(name);
}

GpuParameterBinding GpuPipelineParameterLayout::GetBinding(const String& name) const
{
	GpuParameterBinding output;

	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
	{
		output.Set = found->second.Set;
		output.Slot = found->second.Slot;
	}

	return output;
}

u32 GpuPipelineParameterLayout::GetArraySize(GpuParameterType type, u32 sequentialBindingIndex) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve array size. Sequential binding index out of range: Valid range: [0, {0}). Requested: {1}.", mUniformsPerType[(u32)type].size(), sequentialBindingIndex);
		return 0;
	}

	if(!B3D_ENSURE(mUniformsPerType[(u32)type][sequentialBindingIndex]))
		return 0;
#endif

	return mUniformsPerType[(u32)type][sequentialBindingIndex]->ArraySize;
}

u32 GpuPipelineParameterLayout::GetDynamicOffsetIndex(u32 set, u32 slot, u32 arrayIndex) const
{
#if B3D_BUILD_TYPE == B3D_BUILD_TYPE_DEVELOPMENT
	if(set >= mSets.size())
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve dynamic offset index. Set index out of range: Valid range: [0, {0}). Requested: {1}.", mSets.size(), set);
		return ~0u;
	}

	if(slot >= mSets[set].Uniforms.size() || mSets[set].Uniforms[slot] == nullptr)
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve dynamic offset index. Slot index doesn't exist in the set. Requested: {0}.", slot);
		return ~0u;
	}
#endif

	const UniformInformation& uniformInformation = *mSets[set].Uniforms[slot];

#if B3D_DEBUG
	if(arrayIndex >= uniformInformation.ArraySize)
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve dynamic offset index. Array index out of range: Valid range: [0, {0}). Requested: {1}.", uniformInformation.ArraySize, arrayIndex);
		return -1;
	}
#endif

	return uniformInformation.DynamicOffsetIndex + arrayIndex;
}

u32 GpuPipelineParameterLayout::GetDynamicOffsetIndex(const String& name, u32 arrayIndex) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return GetDynamicOffsetIndex(found->second.Set, found->second.Slot, arrayIndex);

	return ~0u;
}

bool GpuPipelineParameterLayout::HasUniformOfType(const String& name, GpuParameterType type) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return found->second.Type == type;

	return false;
}

const UniformInformation* GpuPipelineParameterLayout::TryGetUniformInformation(const String& name) const
{
	if(auto found = mUniformMap.find(name); found != mUniformMap.end())
		return &found->second;

	return nullptr;
}

const UniformInformation* GpuPipelineParameterLayout::TryGetUniformInformation(GpuParameterType type, u32 sequentialBindingIndex) const
{
	if(sequentialBindingIndex >= mUniformsPerType[(u32)type].size())
		return nullptr;

	return mUniformsPerType[(u32)type][sequentialBindingIndex];
}

const UniformInformation* GpuPipelineParameterLayout::TryGetUniformInformation(const GpuParameterBinding& binding) const
{
	if(binding.Set >= (u32)mSets.size())
		return nullptr;

	if(binding.Slot >= (u32)mSets[binding.Set].Uniforms.size())
		return nullptr;

	return mSets[binding.Set].Uniforms[binding.Slot];
}

const GpuDataParameterInformation* GpuPipelineParameterLayout::TryGetUniformBufferMemberInformation(const String& name) const
{
	if(auto found = mUniformBufferMembers.find(name); found != mUniformBufferMembers.end())
		return &found->second;

	return nullptr;
}
