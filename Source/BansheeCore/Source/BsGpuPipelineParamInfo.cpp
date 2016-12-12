//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuPipelineParamInfo.h"
#include "BsGpuParamDesc.h"
#include "BsRenderStateManager.h"

namespace bs
{
	GpuPipelineParamInfoBase::GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc)
		:mNumSets(0), mNumElements(0), mSetInfos(nullptr)
	{
		bs_zero_out(mNumElementsPerType);

		mParamDescs[GPT_FRAGMENT_PROGRAM] = desc.fragmentParams;
		mParamDescs[GPT_VERTEX_PROGRAM] = desc.vertexParams;
		mParamDescs[GPT_GEOMETRY_PROGRAM] = desc.geometryParams;
		mParamDescs[GPT_HULL_PROGRAM] = desc.hullParams;
		mParamDescs[GPT_DOMAIN_PROGRAM] = desc.domainParams;
		mParamDescs[GPT_COMPUTE_PROGRAM] = desc.computeParams;

		UINT32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				if ((paramBlock.second.set + 1) > mNumSets)
					mNumSets = paramBlock.second.set + 1;

				mNumElements++;
			}

			for (auto& texture : paramDesc->textures)
			{
				if ((texture.second.set + 1) > mNumSets)
					mNumSets = texture.second.set + 1;

				mNumElements++;
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				if ((texture.second.set + 1) > mNumSets)
					mNumSets = texture.second.set + 1;

				mNumElements++;
			}

			for (auto& buffer : paramDesc->buffers)
			{
				if ((buffer.second.set + 1) > mNumSets)
					mNumSets = buffer.second.set + 1;

				mNumElements++;
			}

			for (auto& sampler : paramDesc->samplers)
			{
				if ((sampler.second.set + 1) > mNumSets)
					mNumSets = sampler.second.set + 1;

				mNumElements++;
			}
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

		UINT32 allocSize = mNumSets * sizeof(SetInfo) + totalNumSlots * (sizeof(UINT32) * sizeof(ParamType));
		mData = (UINT8*)bs_alloc(allocSize);
		UINT8* dataPtr = mData;

		mSetInfos = (SetInfo*)dataPtr;
		bs_zero_out(mSetInfos, mNumSets);

		dataPtr += mNumSets * sizeof(SetInfo);

		for (UINT32 i = 0; i < mNumSets; i++)
			mSetInfos[i].numSlots = numSlotsPerSet[i];

		bs_stack_free(numSlotsPerSet);

		for(UINT32 i = 0; i < mNumSets; i++)
		{
			mSetInfos[i].slotIndices = (UINT32*)dataPtr;
			memset(mSetInfos[i].slotIndices, -1, sizeof(UINT32) * mSetInfos[i].numSlots);
			dataPtr += sizeof(UINT32) * mSetInfos[i].numSlots;

			mSetInfos[i].slotTypes = (ParamType*)dataPtr;
			dataPtr += sizeof(ParamType) * mSetInfos[i].numSlots;
		}

		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				SetInfo& setInfo = mSetInfos[paramBlock.second.set];
				setInfo.slotIndices[paramBlock.second.slot] = mNumElementsPerType[(int)ParamType::ParamBlock];
				setInfo.slotTypes[paramBlock.second.slot] = ParamType::ParamBlock;

				mNumElementsPerType[(int)ParamType::ParamBlock]++;
			}

			for (auto& texture : paramDesc->textures)
			{
				SetInfo& setInfo = mSetInfos[texture.second.set];
				setInfo.slotIndices[texture.second.slot] = mNumElementsPerType[(int)ParamType::Texture];
				setInfo.slotTypes[texture.second.slot] = ParamType::Texture;

				mNumElementsPerType[(int)ParamType::Texture]++;
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				SetInfo& setInfo = mSetInfos[texture.second.set];
				setInfo.slotIndices[texture.second.slot] = mNumElementsPerType[(int)ParamType::LoadStoreTexture];
				setInfo.slotTypes[texture.second.slot] = ParamType::LoadStoreTexture;

				mNumElementsPerType[(int)ParamType::LoadStoreTexture]++;
			}

			for (auto& buffer : paramDesc->buffers)
			{
				SetInfo& setInfo = mSetInfos[buffer.second.set];
				setInfo.slotIndices[buffer.second.slot] = mNumElementsPerType[(int)ParamType::Buffer];
				setInfo.slotTypes[buffer.second.slot] = ParamType::Buffer;

				mNumElementsPerType[(int)ParamType::Buffer]++;
			}

			for (auto& sampler : paramDesc->samplers)
			{
				SetInfo& setInfo = mSetInfos[sampler.second.set];
				setInfo.slotIndices[sampler.second.slot] = mNumElementsPerType[(int)ParamType::SamplerState];
				setInfo.slotTypes[sampler.second.slot] = ParamType::SamplerState;

				mNumElementsPerType[(int)ParamType::SamplerState]++;
			}
		}
	}

	GpuPipelineParamInfoBase::~GpuPipelineParamInfoBase()
	{
		bs_free(mData);
	}

	UINT32 GpuPipelineParamInfoBase::getSequentialSlot(ParamType type, UINT32 set, UINT32 slot) const
	{
#if BS_DEBUG_MODE
		if (set >= mNumSets)
		{
			LOGERR("Set index out of range: Valid range: 0 .. " +
				   toString(mNumSets - 1) + ". Requested: " + toString(set));
			return -1;
		}

		if (slot >= mSetInfos[set].numSlots)
		{
			LOGERR("Slot index out of range: Valid range: 0 .. " +
				   toString(mSetInfos[set].numSlots - 1) + ". Requested: " + toString(slot));
			return -1;
		}

		if(mSetInfos[set].slotTypes[slot] != type)
		{
			LOGERR("Requested parameter is not of the valid type. Requested: " + toString((UINT32)type) + ". Actual: " + 
				toString((UINT32)mSetInfos[set].slotTypes[slot]) + ".");
			return -1;
		}

#endif

		return mSetInfos[set].slotIndices[slot];
	}

	GpuPipelineParamInfoCore::GpuPipelineParamInfoCore(const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuPipelineParamInfoBase(desc)
	{ }

	SPtr<GpuPipelineParamInfoCore> GpuPipelineParamInfoCore::create(const GPU_PIPELINE_PARAMS_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		return RenderStateCoreManager::instance().createPipelineParamInfo(desc, deviceMask);
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

	SPtr<GpuPipelineParamInfoCore> GpuPipelineParamInfo::getCore() const
	{
		return std::static_pointer_cast<GpuPipelineParamInfoCore>(mCoreSpecific);
	}

	SPtr<CoreObjectCore> GpuPipelineParamInfo::createCore() const
	{
		GPU_PIPELINE_PARAMS_DESC desc;
		desc.fragmentParams = mParamDescs[GPT_FRAGMENT_PROGRAM];
		desc.vertexParams = mParamDescs[GPT_VERTEX_PROGRAM];
		desc.geometryParams = mParamDescs[GPT_GEOMETRY_PROGRAM];
		desc.hullParams = mParamDescs[GPT_HULL_PROGRAM];
		desc.domainParams = mParamDescs[GPT_DOMAIN_PROGRAM];
		desc.computeParams = mParamDescs[GPT_COMPUTE_PROGRAM];

		return RenderStateCoreManager::instance()._createPipelineParamInfo(desc);
	}
}