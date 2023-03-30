//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuPipelineParameterLayout.h"

#include "BsCoreApplication.h"
#include "BsGpuDevice.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Managers/BsRenderStateManager.h"
#include "Math/BsMath.h"

using namespace bs;

GpuPipelineParameterLayoutBase::GpuPipelineParameterLayoutBase(const GpuPipelineParameterLayoutCreateInformation& createInformation)
	: mResourceInfos()
{
	B3DZeroOut(mBindingSlotCountPerType);
	B3DZeroOut(mResourceCountPerType);

	mPerProgramParameterDescriptions[GPT_FRAGMENT_PROGRAM] = createInformation.Fragment;
	mPerProgramParameterDescriptions[GPT_VERTEX_PROGRAM] = createInformation.Vertex;
	mPerProgramParameterDescriptions[GPT_GEOMETRY_PROGRAM] = createInformation.Geometry;
	mPerProgramParameterDescriptions[GPT_HULL_PROGRAM] = createInformation.Hull;
	mPerProgramParameterDescriptions[GPT_DOMAIN_PROGRAM] = createInformation.Domain;
	mPerProgramParameterDescriptions[GPT_COMPUTE_PROGRAM] = createInformation.Compute;

	auto fnCountElementsForType = [this](auto& entry, GpuParameterType type)
	{
		const u32 typeIndex = (u32)type;

		if((entry.Set + 1) > mSetCount)
			mSetCount = entry.Set + 1;

		mBindingSlotCountPerType[typeIndex]++;
		mBindingSlotCount++;

		mResourceCountPerType[typeIndex]++;
		mResourceCount++;
	};

	auto fnCountElementsForTypeWithArraySupport = [this](auto& entry, GpuParameterType type)
	{
		const u32 typeIndex = (u32)type;
		const u32 arraySize = Math::Max(1u, entry.ArraySize);

		if((entry.Set + 1) > mSetCount)
			mSetCount = entry.Set + 1;

		mBindingSlotCountPerType[typeIndex]++;
		mBindingSlotCount++;

		mResourceCountPerType[typeIndex] += arraySize;
		mResourceCount += arraySize;
	};

	const u32 gpuProgramParameterSetsCount = sizeof(mPerProgramParameterDescriptions) / sizeof(mPerProgramParameterDescriptions[0]);
	for(u32 gpuProgramParameterSetIndex = 0; gpuProgramParameterSetIndex < gpuProgramParameterSetsCount; gpuProgramParameterSetIndex++)
	{
		const SPtr<GpuProgramParameterDescription>& paramDesc = mPerProgramParameterDescriptions[gpuProgramParameterSetIndex];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->DataParameterBlocks)
			fnCountElementsForType(paramBlock.second, GpuParameterType::UniformBuffer);

		for(auto& texture : paramDesc->Textures)
			fnCountElementsForTypeWithArraySupport(texture.second, GpuParameterType::SampledTexture);

		for(auto& texture : paramDesc->StorageTextures)
			fnCountElementsForTypeWithArraySupport(texture.second, GpuParameterType::StorageTexture);

		for(auto& buffer : paramDesc->Buffers)
			fnCountElementsForTypeWithArraySupport(buffer.second, GpuParameterType::StorageBuffer);

		for(auto& sampler : paramDesc->Samplers)
			fnCountElementsForType(sampler.second, GpuParameterType::Sampler);
	}

	u32* slotCountPerSet = (u32*)B3DStackAllocate(mSetCount * sizeof(u32));
	B3DZeroOut(slotCountPerSet, mSetCount);

	for(u32 gpuProgramParameterSetIndex = 0; gpuProgramParameterSetIndex < gpuProgramParameterSetsCount; gpuProgramParameterSetIndex++)
	{
		const SPtr<GpuProgramParameterDescription>& paramDesc = mPerProgramParameterDescriptions[gpuProgramParameterSetIndex];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->DataParameterBlocks)
			slotCountPerSet[paramBlock.second.Set] =
				std::max(slotCountPerSet[paramBlock.second.Set], paramBlock.second.Slot + 1);

		for(auto& texture : paramDesc->Textures)
			slotCountPerSet[texture.second.Set] =
				std::max(slotCountPerSet[texture.second.Set], texture.second.Slot + 1);

		for(auto& texture : paramDesc->StorageTextures)
			slotCountPerSet[texture.second.Set] =
				std::max(slotCountPerSet[texture.second.Set], texture.second.Slot + 1);

		for(auto& buffer : paramDesc->Buffers)
			slotCountPerSet[buffer.second.Set] =
				std::max(slotCountPerSet[buffer.second.Set], buffer.second.Slot + 1);

		for(auto& sampler : paramDesc->Samplers)
			slotCountPerSet[sampler.second.Set] =
				std::max(slotCountPerSet[sampler.second.Set], sampler.second.Slot + 1);
	}

	u32 totalSlotCount = 0;
	for(u32 setIndex = 0; setIndex < mSetCount; setIndex++)
		totalSlotCount += slotCountPerSet[setIndex];

	mAlloc.Reserve<SetInfo>(mSetCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<u32>(totalSlotCount)
		.Reserve<GpuParameterType>(totalSlotCount);

	for(u32 parameterTypeIndex = 0; parameterTypeIndex < (u32)GpuParameterType::Count; parameterTypeIndex++)
		mAlloc.Reserve<ResourceInfo>(mBindingSlotCountPerType[parameterTypeIndex]);

	mAlloc.Init();

	mSetInfos = mAlloc.Alloc<SetInfo>(mSetCount);

	if(mSetInfos != nullptr)
		B3DZeroOut(mSetInfos, mSetCount);

	for(u32 i = 0; i < mSetCount; i++)
		mSetInfos[i].SlotCount = slotCountPerSet[i];

	B3DStackFree(slotCountPerSet);

	for(u32 setIndex = 0; setIndex < mSetCount; setIndex++)
	{
		mSetInfos[setIndex].SlotToSequentialBindingIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		memset(mSetInfos[setIndex].SlotToSequentialBindingIndex, -1, sizeof(u32) * mSetInfos[setIndex].SlotCount);

		mSetInfos[setIndex].SlotToSequentialResourceIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		memset(mSetInfos[setIndex].SlotToSequentialResourceIndex, -1, sizeof(u32) * mSetInfos[setIndex].SlotCount);

		mSetInfos[setIndex].SlotToSequentialSamplerBindingIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		memset(mSetInfos[setIndex].SlotToSequentialSamplerBindingIndex, -1, sizeof(u32) * mSetInfos[setIndex].SlotCount);

		mSetInfos[setIndex].SlotToSequentialSamplerResourceIndex = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		memset(mSetInfos[setIndex].SlotToSequentialSamplerResourceIndex, -1, sizeof(u32) * mSetInfos[setIndex].SlotCount);

		mSetInfos[setIndex].SlotArraySizes = mAlloc.Alloc<u32>(mSetInfos[setIndex].SlotCount);
		memset(mSetInfos[setIndex].SlotArraySizes, 0, sizeof(u32) * mSetInfos[setIndex].SlotCount);

		mSetInfos[setIndex].SlotTypes = mAlloc.Alloc<GpuParameterType>(mSetInfos[setIndex].SlotCount);
	}

	for(u32 parameterTypeIndex = 0; parameterTypeIndex < (u32)GpuParameterType::Count; parameterTypeIndex++)
	{
		mResourceInfos[parameterTypeIndex] = mAlloc.Alloc<ResourceInfo>(mBindingSlotCountPerType[parameterTypeIndex]);
		mBindingSlotCountPerType[parameterTypeIndex] = 0;
		mResourceCountPerType[parameterTypeIndex] = 0;
	}

	auto fnPopulateSetInfo = [this](auto& entry, GpuParameterType type)
	{
		const u32 typeIndex = (u32)type;
		const u32 sequentialBindingIndex = mBindingSlotCountPerType[typeIndex];
		const u32 sequentialResourceIndex = mResourceCountPerType[typeIndex];

		SetInfo& setInfo = mSetInfos[entry.Set];
		setInfo.SlotToSequentialBindingIndex[entry.Slot] = sequentialBindingIndex;
		setInfo.SlotToSequentialResourceIndex[entry.Slot] = sequentialResourceIndex;
		setInfo.SlotArraySizes[entry.Slot] = 1;
		setInfo.SlotTypes[entry.Slot] = type;

		mResourceInfos[typeIndex][sequentialBindingIndex].Set = entry.Set;
		mResourceInfos[typeIndex][sequentialBindingIndex].Slot = entry.Slot;
		mResourceInfos[typeIndex][sequentialBindingIndex].ArraySize = 1;

		mBindingSlotCountPerType[typeIndex]++;
		mResourceCountPerType[typeIndex]++;
	};

	auto fnPopulateSetInfoWithArraySupport = [this](auto& entry, GpuParameterType type)
	{
		const u32 typeIndex = (u32)type;
		const u32 sequentialBindingIndex = mBindingSlotCountPerType[typeIndex];
		const u32 sequentialResourceIndex = mResourceCountPerType[typeIndex];
		const u32 arraySize = Math::Max(1u, entry.ArraySize);

		SetInfo& setInfo = mSetInfos[entry.Set];
		setInfo.SlotToSequentialBindingIndex[entry.Slot] = sequentialBindingIndex;
		setInfo.SlotToSequentialResourceIndex[entry.Slot] = sequentialResourceIndex;
		setInfo.SlotArraySizes[entry.Slot] = arraySize;
		setInfo.SlotTypes[entry.Slot] = type;

		mResourceInfos[typeIndex][sequentialBindingIndex].Set = entry.Set;
		mResourceInfos[typeIndex][sequentialBindingIndex].Slot = entry.Slot;
		mResourceInfos[typeIndex][sequentialBindingIndex].ArraySize = arraySize;

		mBindingSlotCountPerType[typeIndex]++;
		mResourceCountPerType[typeIndex] += arraySize;
	};

	for(u32 gpuProgramParameterSetIndex = 0; gpuProgramParameterSetIndex < gpuProgramParameterSetsCount; gpuProgramParameterSetIndex++)
	{
		const SPtr<GpuProgramParameterDescription>& paramDesc = mPerProgramParameterDescriptions[gpuProgramParameterSetIndex];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->DataParameterBlocks)
			fnPopulateSetInfo(paramBlock.second, GpuParameterType::UniformBuffer);

		for(auto& texture : paramDesc->Textures)
			fnPopulateSetInfoWithArraySupport(texture.second, GpuParameterType::SampledTexture);

		for(auto& texture : paramDesc->StorageTextures)
			fnPopulateSetInfoWithArraySupport(texture.second, GpuParameterType::StorageTexture);

		for(auto& buffer : paramDesc->Buffers)
			fnPopulateSetInfoWithArraySupport(buffer.second, GpuParameterType::StorageBuffer);

		// Samplers need to be handled specially because certain slots could be texture/buffer + sampler combinations
		{
			const u32 typeIndex = (u32)GpuParameterType::Sampler;
			for(auto& entry : paramDesc->Samplers)
			{
				const GpuObjectParameterInformation& samplerInformation = entry.second;
				const u32 sequentialBindingIndex = mBindingSlotCountPerType[typeIndex];
				const u32 sequentialResourceIndex = mResourceCountPerType[typeIndex];
				const u32 arraySize = Math::Max(1u, samplerInformation.ArraySize);

				SetInfo& setInfo = mSetInfos[samplerInformation.Set];
				if(setInfo.SlotToSequentialResourceIndex[samplerInformation.Slot] == ~0u) // Slot is sampler only
				{
					setInfo.SlotToSequentialBindingIndex[samplerInformation.Slot] = sequentialBindingIndex;
					setInfo.SlotToSequentialResourceIndex[samplerInformation.Slot] = sequentialResourceIndex;
					setInfo.SlotArraySizes[samplerInformation.Slot] = arraySize;
					setInfo.SlotTypes[samplerInformation.Slot] = GpuParameterType::Sampler;
				}
				else // Slot is a combination
				{
					setInfo.SlotToSequentialSamplerBindingIndex[samplerInformation.Slot] = sequentialBindingIndex;
					setInfo.SlotToSequentialSamplerResourceIndex[samplerInformation.Slot] = sequentialResourceIndex;
				}

				mResourceInfos[typeIndex][sequentialBindingIndex].Set = samplerInformation.Set;
				mResourceInfos[typeIndex][sequentialBindingIndex].Slot = samplerInformation.Slot;
				mResourceInfos[typeIndex][sequentialBindingIndex].ArraySize = arraySize;

				mBindingSlotCountPerType[typeIndex]++;
				mResourceCountPerType[typeIndex] += arraySize;
			}
		}
	}
}

u32 GpuPipelineParameterLayoutBase::GetSequentialResourceIndex(GpuParameterType type, u32 set, u32 slot, u32 arrayIndex) const
{
#if B3D_DEBUG
	if(set >= mSetCount)
	{
		B3D_LOG(Error, RenderBackend, "Set index out of range: Valid range: [0, {0}). Requested: {1}.", mSetCount, set);
		return ~0u;
	}

	if(slot >= mSetInfos[set].SlotCount)
	{
		B3D_LOG(Error, RenderBackend, "Slot index out of range: Valid range: [0, {0}). Requested: {1}.", mSetInfos[set].SlotCount, slot);
		return ~0u;
	}

	if(arrayIndex >= mSetInfos[set].SlotArraySizes[slot])
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve sequential resource index. Array index out of range: Valid range: [0, {0}). Requested: {1}.", mSetInfos[set].SlotArraySizes[slot], arrayIndex);
		return -1;
	}

	const GpuParameterType slotType = mSetInfos[set].SlotTypes[slot];
	if(slotType != type)
	{
		// Allow sampler states & textures/buffers to share the same slot, as some APIs combine them
		if(type == GpuParameterType::Sampler)
		{
			if(mSetInfos[set].SlotToSequentialSamplerResourceIndex[slot] != ~0u)
				return mSetInfos[set].SlotToSequentialSamplerResourceIndex[slot] + arrayIndex;

		}

		B3D_LOG(Error, RenderBackend, "Requested parameter is not of the valid type. Requested: {0}. Actual: {1}.", (u32)type, (u32)mSetInfos[set].SlotTypes[slot]);
		return ~0u;
	}

#endif

	return mSetInfos[set].SlotToSequentialResourceIndex[slot] != ~0u ? mSetInfos[set].SlotToSequentialResourceIndex[slot] + arrayIndex : ~0u;
}


u32 GpuPipelineParameterLayoutBase::GetSequentialBindingIndex(GpuParameterType type, u32 set, u32 slot) const
{
#if B3D_DEBUG
	if(set >= mSetCount)
	{
		B3D_LOG(Error, RenderBackend, "Set index out of range: Valid range: [0, {0}). Requested: {1}.", mSetCount, set);
		return ~0u;
	}

	if(slot >= mSetInfos[set].SlotCount)
	{
		B3D_LOG(Error, RenderBackend, "Slot index out of range: Valid range: [0, {0}). Requested: {1}.", mSetInfos[set].SlotCount, slot);
		return ~0u;
	}

	const GpuParameterType slotType = mSetInfos[set].SlotTypes[slot];
	if(slotType != type)
	{
		// Allow sampler states & textures/buffers to share the same slot, as some APIs combine them
		if(type == GpuParameterType::Sampler)
		{
			if(mSetInfos[set].SlotToSequentialSamplerBindingIndex[slot] != ~0u)
				return mSetInfos[set].SlotToSequentialSamplerBindingIndex[slot];
		}

		B3D_LOG(Error, RenderBackend, "Requested parameter is not of the valid type. Requested: {0}. Actual: {1}.", (u32)type, (u32)mSetInfos[set].SlotTypes[slot]);
		return ~0u;
	}
#endif

	return mSetInfos[set].SlotToSequentialBindingIndex[slot];
}

void GpuPipelineParameterLayoutBase::GetBinding(GpuParameterType type, u32 sequentialBindingIndex, u32& set, u32& slot) const
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mBindingSlotCountPerType[(int)type])
	{
		B3D_LOG(Error, RenderBackend, "Sequential slot index out of range: Valid range: [0, {0}). Requested: {1}.", mBindingSlotCountPerType[(int)type], sequentialBindingIndex);

		set = 0;
		slot = 0;
		return;
	}
#endif

	set = mResourceInfos[(u32)type][sequentialBindingIndex].Set;
	slot = mResourceInfos[(u32)type][sequentialBindingIndex].Slot;
}

void GpuPipelineParameterLayoutBase::GetBindings(GpuParameterType type, const String& name, GpuParameterBinding (&bindings)[GPT_COUNT])
{
	constexpr u32 numParamDescs = sizeof(mPerProgramParameterDescriptions) / sizeof(mPerProgramParameterDescriptions[0]);
	static_assert(
		numParamDescs == GPT_COUNT,
		"Number of param descriptor structures must match the number of GPU program stages.");

	for(u32 i = 0; i < numParamDescs; i++)
		GetBinding((GpuProgramType)i, type, name, bindings[i]);
}

void GpuPipelineParameterLayoutBase::GetBinding(GpuProgramType progType, GpuParameterType type, const String& name, GpuParameterBinding& binding)
{
	auto findBinding = [](auto& paramMap, const String& name, GpuParameterBinding& binding)
	{
		auto iterFind = paramMap.find(name);
		if(iterFind != paramMap.end())
		{
			binding.Set = iterFind->second.Set;
			binding.Slot = iterFind->second.Slot;
		}
		else
			binding.Set = binding.Slot = (u32)-1;
	};

	const SPtr<GpuProgramParameterDescription>& paramDesc = mPerProgramParameterDescriptions[(u32)progType];
	if(paramDesc == nullptr)
	{
		binding.Set = binding.Slot = (u32)-1;
		return;
	}

	switch(type)
	{
	case GpuParameterType::UniformBuffer:
		findBinding(paramDesc->DataParameterBlocks, name, binding);
		break;
	case GpuParameterType::SampledTexture:
		findBinding(paramDesc->Textures, name, binding);
		break;
	case GpuParameterType::StorageTexture:
		findBinding(paramDesc->StorageTextures, name, binding);
		break;
	case GpuParameterType::StorageBuffer:
		findBinding(paramDesc->Buffers, name, binding);
		break;
	case GpuParameterType::Sampler:
		findBinding(paramDesc->Samplers, name, binding);
		break;
	default:
		break;
	}
}

u32 GpuPipelineParameterLayoutBase::GetArraySize(GpuParameterType type, u32 sequentialBindingIndex)
{
#if B3D_DEBUG
	if(sequentialBindingIndex >= mBindingSlotCountPerType[(int)type])
	{
		B3D_LOG(Error, RenderBackend, "Cannot retrieve array size. Sequential binding index out of range: Valid range: [0, {0}). Requested: {1}.", mBindingSlotCountPerType[(int)type], sequentialBindingIndex);

		return 0;
	}
#endif

	return mResourceInfos[(u32)type][sequentialBindingIndex].ArraySize;
}

GpuPipelineParameterLayout::GpuPipelineParameterLayout(const GpuPipelineParameterLayoutInformation& desc)
	: GpuPipelineParameterLayoutBase(desc)
{}

SPtr<GpuPipelineParameterLayout> GpuPipelineParameterLayout::Create(const GpuPipelineParameterLayoutInformation& desc)
{
	SPtr<GpuPipelineParameterLayout> paramInfo =
		B3DMakeCoreFromExisting<GpuPipelineParameterLayout>(new(B3DAllocate<GpuPipelineParameterLayout>()) GpuPipelineParameterLayout(desc));
	paramInfo->SetShared(paramInfo);
	paramInfo->Initialize();

	return paramInfo;
}

SPtr<ct::GpuPipelineParameterLayout> GpuPipelineParameterLayout::GetCore() const
{
	return std::static_pointer_cast<ct::GpuPipelineParameterLayout>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuPipelineParameterLayout::CreateCore() const
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if(!gpuDevice)
		return nullptr;

	GpuPipelineParameterLayoutCreateInformation createInformation;
	createInformation.Fragment = mPerProgramParameterDescriptions[GPT_FRAGMENT_PROGRAM];
	createInformation.Vertex = mPerProgramParameterDescriptions[GPT_VERTEX_PROGRAM];
	createInformation.Geometry = mPerProgramParameterDescriptions[GPT_GEOMETRY_PROGRAM];
	createInformation.Hull = mPerProgramParameterDescriptions[GPT_HULL_PROGRAM];
	createInformation.Domain = mPerProgramParameterDescriptions[GPT_DOMAIN_PROGRAM];
	createInformation.Compute = mPerProgramParameterDescriptions[GPT_COMPUTE_PROGRAM];

	return gpuDevice->CreateGpuPipelineParameterLayout(createInformation, true);
}

namespace bs { namespace ct
{
GpuPipelineParameterLayout::GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation)
	: GpuPipelineParameterLayoutBase(createInformation)
{}
}}
