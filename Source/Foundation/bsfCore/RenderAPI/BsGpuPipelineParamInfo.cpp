//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Managers/BsRenderStateManager.h"

using namespace bs;

GpuPipelineParamInfoBase::GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc)
	: mNumSets(0), mNumElements(0), mSetInfos(nullptr), mResourceInfos()
{
	B3DZeroOut(mNumElementsPerType);

	mParamDescs[GPT_FRAGMENT_PROGRAM] = desc.FragmentParams;
	mParamDescs[GPT_VERTEX_PROGRAM] = desc.VertexParams;
	mParamDescs[GPT_GEOMETRY_PROGRAM] = desc.GeometryParams;
	mParamDescs[GPT_HULL_PROGRAM] = desc.HullParams;
	mParamDescs[GPT_DOMAIN_PROGRAM] = desc.DomainParams;
	mParamDescs[GPT_COMPUTE_PROGRAM] = desc.ComputeParams;

	auto countElements = [&](auto& entry, ParamType type)
	{
		int typeIdx = (int)type;

		if((entry.Set + 1) > mNumSets)
			mNumSets = entry.Set + 1;

		mNumElementsPerType[typeIdx]++;
		mNumElements++;
	};

	u32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
	for(u32 i = 0; i < numParamDescs; i++)
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->ParamBlocks)
			countElements(paramBlock.second, ParamType::ParamBlock);

		for(auto& texture : paramDesc->Textures)
			countElements(texture.second, ParamType::Texture);

		for(auto& texture : paramDesc->LoadStoreTextures)
			countElements(texture.second, ParamType::LoadStoreTexture);

		for(auto& buffer : paramDesc->Buffers)
			countElements(buffer.second, ParamType::Buffer);

		for(auto& sampler : paramDesc->Samplers)
			countElements(sampler.second, ParamType::SamplerState);
	}

	u32* numSlotsPerSet = (u32*)B3DStackAllocate(mNumSets * sizeof(u32));
	B3DZeroOut(numSlotsPerSet, mNumSets);

	for(u32 i = 0; i < numParamDescs; i++)
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->ParamBlocks)
			numSlotsPerSet[paramBlock.second.Set] =
				std::max(numSlotsPerSet[paramBlock.second.Set], paramBlock.second.Slot + 1);

		for(auto& texture : paramDesc->Textures)
			numSlotsPerSet[texture.second.Set] =
				std::max(numSlotsPerSet[texture.second.Set], texture.second.Slot + 1);

		for(auto& texture : paramDesc->LoadStoreTextures)
			numSlotsPerSet[texture.second.Set] =
				std::max(numSlotsPerSet[texture.second.Set], texture.second.Slot + 1);

		for(auto& buffer : paramDesc->Buffers)
			numSlotsPerSet[buffer.second.Set] =
				std::max(numSlotsPerSet[buffer.second.Set], buffer.second.Slot + 1);

		for(auto& sampler : paramDesc->Samplers)
			numSlotsPerSet[sampler.second.Set] =
				std::max(numSlotsPerSet[sampler.second.Set], sampler.second.Slot + 1);
	}

	u32 totalNumSlots = 0;
	for(u32 i = 0; i < mNumSets; i++)
		totalNumSlots += numSlotsPerSet[i];

	mAlloc.Reserve<SetInfo>(mNumSets)
		.Reserve<u32>(totalNumSlots)
		.Reserve<ParamType>(totalNumSlots)
		.Reserve<u32>(totalNumSlots);

	for(u32 i = 0; i < (u32)ParamType::Count; i++)
		mAlloc.Reserve<ResourceInfo>(mNumElementsPerType[i]);

	mAlloc.Init();

	mSetInfos = mAlloc.Alloc<SetInfo>(mNumSets);

	if(mSetInfos != nullptr)
		B3DZeroOut(mSetInfos, mNumSets);

	for(u32 i = 0; i < mNumSets; i++)
		mSetInfos[i].NumSlots = numSlotsPerSet[i];

	B3DStackFree(numSlotsPerSet);

	for(u32 i = 0; i < mNumSets; i++)
	{
		mSetInfos[i].SlotIndices = mAlloc.Alloc<u32>(mSetInfos[i].NumSlots);
		memset(mSetInfos[i].SlotIndices, -1, sizeof(u32) * mSetInfos[i].NumSlots);

		mSetInfos[i].SlotTypes = mAlloc.Alloc<ParamType>(mSetInfos[i].NumSlots);

		mSetInfos[i].SlotSamplers = mAlloc.Alloc<u32>(mSetInfos[i].NumSlots);
		memset(mSetInfos[i].SlotSamplers, -1, sizeof(u32) * mSetInfos[i].NumSlots);
	}

	for(u32 i = 0; i < (u32)ParamType::Count; i++)
	{
		mResourceInfos[i] = mAlloc.Alloc<ResourceInfo>(mNumElementsPerType[i]);
		mNumElementsPerType[i] = 0;
	}

	auto populateSetInfo = [&](auto& entry, ParamType type)
	{
		int typeIdx = (int)type;

		u32 sequentialIdx = mNumElementsPerType[typeIdx];

		SetInfo& setInfo = mSetInfos[entry.Set];
		setInfo.SlotIndices[entry.Slot] = sequentialIdx;
		setInfo.SlotTypes[entry.Slot] = type;

		mResourceInfos[typeIdx][sequentialIdx].Set = entry.Set;
		mResourceInfos[typeIdx][sequentialIdx].Slot = entry.Slot;

		mNumElementsPerType[typeIdx]++;
	};

	for(u32 i = 0; i < numParamDescs; i++)
	{
		const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
		if(paramDesc == nullptr)
			continue;

		for(auto& paramBlock : paramDesc->ParamBlocks)
			populateSetInfo(paramBlock.second, ParamType::ParamBlock);

		for(auto& texture : paramDesc->Textures)
			populateSetInfo(texture.second, ParamType::Texture);

		for(auto& texture : paramDesc->LoadStoreTextures)
			populateSetInfo(texture.second, ParamType::LoadStoreTexture);

		for(auto& buffer : paramDesc->Buffers)
			populateSetInfo(buffer.second, ParamType::Buffer);

		// Samplers need to be handled specially because certain slots could be texture/buffer + sampler combinations
		{
			int typeIdx = (int)ParamType::SamplerState;
			for(auto& entry : paramDesc->Samplers)
			{
				const GpuParamObjectDesc& samplerDesc = entry.second;
				u32 sequentialIdx = mNumElementsPerType[typeIdx];

				SetInfo& setInfo = mSetInfos[samplerDesc.Set];
				if(setInfo.SlotIndices[samplerDesc.Slot] == (u32)-1) // Slot is sampler only
				{
					setInfo.SlotIndices[samplerDesc.Slot] = sequentialIdx;
					setInfo.SlotTypes[samplerDesc.Slot] = ParamType::SamplerState;
				}
				else // Slot is a combination
				{
					setInfo.SlotSamplers[samplerDesc.Slot] = sequentialIdx;
				}

				mResourceInfos[typeIdx][sequentialIdx].Set = samplerDesc.Set;
				mResourceInfos[typeIdx][sequentialIdx].Slot = samplerDesc.Slot;

				mNumElementsPerType[typeIdx]++;
			}
		}
	}
}

u32 GpuPipelineParamInfoBase::GetSequentialSlot(ParamType type, u32 set, u32 slot) const
{
#if BS_DEBUG_MODE
	if(set >= mNumSets)
	{
		BS_LOG(Error, RenderBackend, "Set index out of range: Valid range: [0, {0}). Requested: {1}.", mNumSets, set);
		return -1;
	}

	if(slot >= mSetInfos[set].NumSlots)
	{
		BS_LOG(Error, RenderBackend, "Slot index out of range: Valid range: [0, {0}). Requested: {1}.", mSetInfos[set].NumSlots, slot);
		return -1;
	}

	ParamType slotType = mSetInfos[set].SlotTypes[slot];
	if(slotType != type)
	{
		// Allow sampler states & textures/buffers to share the same slot, as some APIs combine them
		if(type == ParamType::SamplerState)
		{
			if(mSetInfos[set].SlotSamplers[slot] != (u32)-1)
				return mSetInfos[set].SlotSamplers[slot];
		}

		BS_LOG(Error, RenderBackend, "Requested parameter is not of the valid type. Requested: {0}. Actual: {1}.", (u32)type, (u32)mSetInfos[set].SlotTypes[slot]);
		return -1;
	}

#endif

	return mSetInfos[set].SlotIndices[slot];
}

void GpuPipelineParamInfoBase::GetBinding(ParamType type, u32 sequentialSlot, u32& set, u32& slot) const
{
#if BS_DEBUG_MODE
	if(sequentialSlot >= mNumElementsPerType[(int)type])
	{
		BS_LOG(Error, RenderBackend, "Sequential slot index out of range: Valid range: [0, {0}). Requested: {1}.", mNumElementsPerType[(int)type], sequentialSlot);

		set = 0;
		slot = 0;
		return;
	}
#endif

	set = mResourceInfos[(int)type][sequentialSlot].Set;
	slot = mResourceInfos[(int)type][sequentialSlot].Slot;
}

void GpuPipelineParamInfoBase::GetBindings(ParamType type, const String& name, GpuParamBinding (&bindings)[GPT_COUNT])
{
	constexpr u32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
	static_assert(
		numParamDescs == GPT_COUNT,
		"Number of param descriptor structures must match the number of GPU program stages.");

	for(u32 i = 0; i < numParamDescs; i++)
		GetBinding((GpuProgramType)i, type, name, bindings[i]);
}

void GpuPipelineParamInfoBase::GetBinding(GpuProgramType progType, ParamType type, const String& name, GpuParamBinding& binding)
{
	auto findBinding = [](auto& paramMap, const String& name, GpuParamBinding& binding)
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

	const SPtr<GpuParamDesc>& paramDesc = mParamDescs[(u32)progType];
	if(paramDesc == nullptr)
	{
		binding.Set = binding.Slot = (u32)-1;
		return;
	}

	switch(type)
	{
	case ParamType::ParamBlock:
		findBinding(paramDesc->ParamBlocks, name, binding);
		break;
	case ParamType::Texture:
		findBinding(paramDesc->Textures, name, binding);
		break;
	case ParamType::LoadStoreTexture:
		findBinding(paramDesc->LoadStoreTextures, name, binding);
		break;
	case ParamType::Buffer:
		findBinding(paramDesc->Buffers, name, binding);
		break;
	case ParamType::SamplerState:
		findBinding(paramDesc->Samplers, name, binding);
		break;
	default:
		break;
	}
}

GpuPipelineParamInfo::GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc)
	: GpuPipelineParamInfoBase(desc)
{}

SPtr<GpuPipelineParamInfo> GpuPipelineParamInfo::Create(const GPU_PIPELINE_PARAMS_DESC& desc)
{
	SPtr<GpuPipelineParamInfo> paramInfo =
		B3DMakeCoreFromExisting<GpuPipelineParamInfo>(new(B3DAllocate<GpuPipelineParamInfo>()) GpuPipelineParamInfo(desc));
	paramInfo->SetThisPtrInternal(paramInfo);
	paramInfo->Initialize();

	return paramInfo;
}

SPtr<ct::GpuPipelineParamInfo> GpuPipelineParamInfo::GetCore() const
{
	return std::static_pointer_cast<ct::GpuPipelineParamInfo>(mCoreSpecific);
}

SPtr<ct::CoreObject> GpuPipelineParamInfo::CreateCore() const
{
	GPU_PIPELINE_PARAMS_DESC desc;
	desc.FragmentParams = mParamDescs[GPT_FRAGMENT_PROGRAM];
	desc.VertexParams = mParamDescs[GPT_VERTEX_PROGRAM];
	desc.GeometryParams = mParamDescs[GPT_GEOMETRY_PROGRAM];
	desc.HullParams = mParamDescs[GPT_HULL_PROGRAM];
	desc.DomainParams = mParamDescs[GPT_DOMAIN_PROGRAM];
	desc.ComputeParams = mParamDescs[GPT_COMPUTE_PROGRAM];

	return ct::RenderStateManager::Instance().CreatePipelineParamInfoInternal(desc);
}

namespace bs { namespace ct
{
GpuPipelineParamInfo::GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
	: GpuPipelineParamInfoBase(desc)
{}

SPtr<GpuPipelineParamInfo> GpuPipelineParamInfo::Create(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
{
	return RenderStateManager::Instance().CreatePipelineParamInfo(desc, deviceMask);
}
}}
