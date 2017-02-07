//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuPipelineParamInfo.h"
#include "BsGpuParamDesc.h"
#include "BsRenderStateManager.h"

namespace bs
{
	GpuPipelineParamInfoBase::GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc)
		:mNumSets(0), mNumElements(0), mSetInfos(nullptr), mResourceInfos()
	{
		bs_zero_out(mNumElementsPerType);

		mParamDescs[GPT_FRAGMENT_PROGRAM] = desc.fragmentParams;
		mParamDescs[GPT_VERTEX_PROGRAM] = desc.vertexParams;
		mParamDescs[GPT_GEOMETRY_PROGRAM] = desc.geometryParams;
		mParamDescs[GPT_HULL_PROGRAM] = desc.hullParams;
		mParamDescs[GPT_DOMAIN_PROGRAM] = desc.domainParams;
		mParamDescs[GPT_COMPUTE_PROGRAM] = desc.computeParams;

		auto countElements = [&](auto& entry, ParamType type)
		{
			int typeIdx = (int)type;

			if ((entry.set + 1) > mNumSets)
				mNumSets = entry.set + 1;

			mNumElementsPerType[typeIdx]++;
			mNumElements++;
		};

		UINT32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
				countElements(paramBlock.second, ParamType::ParamBlock);

			for (auto& texture : paramDesc->textures)
				countElements(texture.second, ParamType::Texture);

			for (auto& texture : paramDesc->loadStoreTextures)
				countElements(texture.second, ParamType::LoadStoreTexture);

			for (auto& buffer : paramDesc->buffers)
				countElements(buffer.second, ParamType::Buffer);

			for (auto& sampler : paramDesc->samplers)
				countElements(sampler.second, ParamType::SamplerState);
		}

		UINT32* numSlotsPerSet = (UINT32*)bs_stack_alloc(mNumSets * sizeof(UINT32));
		bs_zero_out(numSlotsPerSet, mNumSets);

		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
				numSlotsPerSet[paramBlock.second.set] = 
					std::max(numSlotsPerSet[paramBlock.second.set], paramBlock.second.slot + 1);

			for (auto& texture : paramDesc->textures)
				numSlotsPerSet[texture.second.set] =
					std::max(numSlotsPerSet[texture.second.set], texture.second.slot + 1);

			for (auto& texture : paramDesc->loadStoreTextures)
				numSlotsPerSet[texture.second.set] =
					std::max(numSlotsPerSet[texture.second.set], texture.second.slot + 1);

			for (auto& buffer : paramDesc->buffers)
				numSlotsPerSet[buffer.second.set] =
					std::max(numSlotsPerSet[buffer.second.set], buffer.second.slot + 1);

			for (auto& sampler : paramDesc->samplers)
				numSlotsPerSet[sampler.second.set] =
					std::max(numSlotsPerSet[sampler.second.set], sampler.second.slot + 1);
		}

		UINT32 totalNumSlots = 0;
		for (UINT32 i = 0; i < mNumSets; i++)
			totalNumSlots += numSlotsPerSet[i];

		mAlloc.reserve<SetInfo>(mNumSets)
			.reserve<UINT32>(totalNumSlots)
			.reserve<ParamType>(totalNumSlots)
			.reserve<UINT32>(totalNumSlots);

		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
			mAlloc.reserve<ResourceInfo>(mNumElementsPerType[i]);

		mAlloc.init();

		mSetInfos = mAlloc.alloc<SetInfo>(mNumSets);

		if(mSetInfos != nullptr)
			bs_zero_out(mSetInfos, mNumSets);

		for (UINT32 i = 0; i < mNumSets; i++)
			mSetInfos[i].numSlots = numSlotsPerSet[i];

		bs_stack_free(numSlotsPerSet);

		for(UINT32 i = 0; i < mNumSets; i++)
		{
			mSetInfos[i].slotIndices = mAlloc.alloc<UINT32>(mSetInfos[i].numSlots);
			memset(mSetInfos[i].slotIndices, -1, sizeof(UINT32) * mSetInfos[i].numSlots);

			mSetInfos[i].slotTypes = mAlloc.alloc<ParamType>(mSetInfos[i].numSlots);

			mSetInfos[i].slotSamplers = mAlloc.alloc<UINT32>(mSetInfos[i].numSlots);
			memset(mSetInfos[i].slotSamplers, -1, sizeof(UINT32) * mSetInfos[i].numSlots);
		}

		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			mResourceInfos[i] = mAlloc.alloc<ResourceInfo>(mNumElementsPerType[i]);
			mNumElementsPerType[i] = 0;
		}

		auto populateSetInfo = [&](auto& entry, ParamType type)
		{
			int typeIdx = (int)type;

			UINT32 sequentialIdx = mNumElementsPerType[typeIdx];

			SetInfo& setInfo = mSetInfos[entry.set];
			setInfo.slotIndices[entry.slot] = sequentialIdx;
			setInfo.slotTypes[entry.slot] = type;

			mResourceInfos[typeIdx][sequentialIdx].set = entry.set;
			mResourceInfos[typeIdx][sequentialIdx].slot = entry.slot;

			mNumElementsPerType[typeIdx]++;
		};

		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
				populateSetInfo(paramBlock.second, ParamType::ParamBlock);

			for (auto& texture : paramDesc->textures)
				populateSetInfo(texture.second, ParamType::Texture);

			for (auto& texture : paramDesc->loadStoreTextures)
				populateSetInfo(texture.second, ParamType::LoadStoreTexture);

			for (auto& buffer : paramDesc->buffers)
				populateSetInfo(buffer.second, ParamType::Buffer);

			// Samplers need to be handled specially because certain slots could be texture/buffer + sampler combinations
			{
				int typeIdx = (int)ParamType::SamplerState;
				UINT32 sequentialIdx = mNumElementsPerType[typeIdx];

				for (auto& entry : paramDesc->samplers)
				{
					const GpuParamObjectDesc& samplerDesc = entry.second;

					SetInfo& setInfo = mSetInfos[samplerDesc.set];
					if (setInfo.slotIndices[samplerDesc.slot] == -1) // Slot is sampler only
					{
						setInfo.slotIndices[samplerDesc.slot] = sequentialIdx;
						setInfo.slotTypes[samplerDesc.slot] = ParamType::SamplerState;
					}
					else // Slot is a combination
					{
						setInfo.slotSamplers[samplerDesc.slot] = sequentialIdx;
					}

					mResourceInfos[typeIdx][sequentialIdx].set = samplerDesc.set;
					mResourceInfos[typeIdx][sequentialIdx].slot = samplerDesc.slot;

					mNumElementsPerType[typeIdx]++;
				}
			}
		}
	}

	GpuPipelineParamInfoBase::~GpuPipelineParamInfoBase()
	{ }

	UINT32 GpuPipelineParamInfoBase::getSequentialSlot(ParamType type, UINT32 set, UINT32 slot) const
	{
#if BS_DEBUG_MODE
		if (set >= mNumSets)
		{
			LOGERR("Set index out of range: Valid range: [0, " +
				   toString(mNumSets) + "). Requested: " + toString(set) + ".");
			return -1;
		}

		if (slot >= mSetInfos[set].numSlots)
		{
			LOGERR("Slot index out of range: Valid range: [0, " +
				   toString(mSetInfos[set].numSlots) + "). Requested: " + toString(slot) + ".");
			return -1;
		}

		ParamType slotType = mSetInfos[set].slotTypes[slot];
		if(slotType != type)
		{
			// Allow sampler states & textures/buffers to share the same slot, as some APIs combine them
			if(type == ParamType::SamplerState)
			{
				if (mSetInfos[set].slotSamplers[slot] != -1)
					return mSetInfos[set].slotSamplers[slot];
			}

			LOGERR("Requested parameter is not of the valid type. Requested: " + toString((UINT32)type) + ". Actual: " +
					toString((UINT32)mSetInfos[set].slotTypes[slot]) + ".");
			return -1;
		}

#endif

		return mSetInfos[set].slotIndices[slot];
	}

	void GpuPipelineParamInfoBase::getSetSlot(ParamType type, UINT32 sequentialSlot, UINT32& set, UINT32& slot) const
	{
#if BS_DEBUG_MODE
		if(sequentialSlot >= mNumElementsPerType[(int)type])
		{
			LOGERR("Sequential slot index out of range: Valid range: [0, " + toString(mNumElementsPerType[(int)type]) + 
				"). Requested: " + toString(sequentialSlot) + ".");

			set = 0;
			slot = 0;
			return;
		}
#endif

		set = mResourceInfos[(int)type][sequentialSlot].set;
		slot = mResourceInfos[(int)type][sequentialSlot].slot;
	}

	GpuPipelineParamInfo::GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc)
		:GpuPipelineParamInfoBase(desc)
	{ }

	SPtr<GpuPipelineParamInfo> GpuPipelineParamInfo::create(const GPU_PIPELINE_PARAMS_DESC& desc)
	{
		SPtr<GpuPipelineParamInfo> paramInfo = 
			bs_core_ptr<GpuPipelineParamInfo>(new (bs_alloc<GpuPipelineParamInfo>()) GpuPipelineParamInfo(desc));
		paramInfo->_setThisPtr(paramInfo);
		paramInfo->initialize();

		return paramInfo;
	}

	SPtr<ct::GpuPipelineParamInfo> GpuPipelineParamInfo::getCore() const
	{
		return std::static_pointer_cast<ct::GpuPipelineParamInfo>(mCoreSpecific);
	}

	SPtr<ct::CoreObject> GpuPipelineParamInfo::createCore() const
	{
		GPU_PIPELINE_PARAMS_DESC desc;
		desc.fragmentParams = mParamDescs[GPT_FRAGMENT_PROGRAM];
		desc.vertexParams = mParamDescs[GPT_VERTEX_PROGRAM];
		desc.geometryParams = mParamDescs[GPT_GEOMETRY_PROGRAM];
		desc.hullParams = mParamDescs[GPT_HULL_PROGRAM];
		desc.domainParams = mParamDescs[GPT_DOMAIN_PROGRAM];
		desc.computeParams = mParamDescs[GPT_COMPUTE_PROGRAM];

		return ct::RenderStateManager::instance()._createPipelineParamInfo(desc);
	}

	namespace ct
	{
	GpuPipelineParamInfo::GpuPipelineParamInfo(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuPipelineParamInfoBase(desc)
	{ }

	SPtr<GpuPipelineParamInfo> GpuPipelineParamInfo::create(const GPU_PIPELINE_PARAMS_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		return RenderStateManager::instance().createPipelineParamInfo(desc, deviceMask);
	}
	}
}