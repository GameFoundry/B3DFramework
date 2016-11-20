//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGpuPipelineParamInfo.h"
#include "BsGpuParamDesc.h"
#include "BsRenderStateManager.h"

namespace bs
{
	GpuPipelineParamInfoBase::GpuPipelineParamInfoBase(const GPU_PIPELINE_PARAMS_DESC& desc)
		:mTotalNumSets(0), mTotalNumElements(0)
	{
		mParamDescs[GPT_FRAGMENT_PROGRAM] = desc.fragmentParams;
		mParamDescs[GPT_VERTEX_PROGRAM] = desc.vertexParams;
		mParamDescs[GPT_GEOMETRY_PROGRAM] = desc.geometryParams;
		mParamDescs[GPT_HULL_PROGRAM] = desc.hullParams;
		mParamDescs[GPT_DOMAIN_PROGRAM] = desc.domainParams;
		mParamDescs[GPT_COMPUTE_PROGRAM] = desc.computeParams;

		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			mNumSets[i] = 0;
			mNumElements[i] = 0;
			mOffsets[i] = nullptr;
		}

		UINT32 numParamDescs = sizeof(mParamDescs) / sizeof(mParamDescs[0]);
		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				if ((paramBlock.second.set + 1) > mNumSets[(int)ParamType::ParamBlock])
					mNumSets[(int)ParamType::ParamBlock] = paramBlock.second.set + 1;
			}

			for (auto& texture : paramDesc->textures)
			{
				if ((texture.second.set + 1) > mNumSets[(int)ParamType::Texture])
					mNumSets[(int)ParamType::Texture] = texture.second.set + 1;
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				if ((texture.second.set + 1) > mNumSets[(int)ParamType::LoadStoreTexture])
					mNumSets[(int)ParamType::LoadStoreTexture] = texture.second.set + 1;
			}

			for (auto& buffer : paramDesc->buffers)
			{
				if ((buffer.second.set + 1) > mNumSets[(int)ParamType::Buffer])
					mNumSets[(int)ParamType::Buffer] = buffer.second.set + 1;
			}

			for (auto& sampler : paramDesc->samplers)
			{
				if ((sampler.second.set + 1) > mNumSets[(int)ParamType::SamplerState])
					mNumSets[(int)ParamType::SamplerState] = sampler.second.set + 1;
			}
		}

		UINT32 totalNumSets = 0;
		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			totalNumSets += mNumSets[i];
			mTotalNumSets = std::max(mTotalNumSets, mNumSets[i]);
		}

		UINT32* slotsPerSetData = bs_stack_alloc<UINT32>(totalNumSets);
		memset(slotsPerSetData, 0, sizeof(UINT32) * totalNumSets);

		UINT32* slotsPerSet[(UINT32)ParamType::Count];
		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			if (i == 0)
				slotsPerSet[i] = slotsPerSetData;
			else
				slotsPerSet[i] = slotsPerSet[i - 1] + mNumSets[i - 1];
		}

		for (UINT32 i = 0; i < numParamDescs; i++)
		{
			const SPtr<GpuParamDesc>& paramDesc = mParamDescs[i];
			if (paramDesc == nullptr)
				continue;

			for (auto& paramBlock : paramDesc->paramBlocks)
			{
				UINT32* slots = slotsPerSet[(int)ParamType::ParamBlock];
				slots[paramBlock.second.set] = std::max(slots[paramBlock.second.set], paramBlock.second.slot + 1);
			}

			for (auto& texture : paramDesc->textures)
			{
				UINT32* slots = slotsPerSet[(int)ParamType::Texture];
				slots[texture.second.set] = std::max(slots[texture.second.set], texture.second.slot + 1);
			}

			for (auto& texture : paramDesc->loadStoreTextures)
			{
				UINT32* slots = slotsPerSet[(int)ParamType::LoadStoreTexture];
				slots[texture.second.set] = std::max(slots[texture.second.set], texture.second.slot + 1);
			}

			for (auto& buffer : paramDesc->buffers)
			{
				UINT32* slots = slotsPerSet[(int)ParamType::Buffer];
				slots[buffer.second.set] = std::max(slots[buffer.second.set], buffer.second.slot + 1);
			}

			for (auto& sampler : paramDesc->samplers)
			{
				UINT32* slots = slotsPerSet[(int)ParamType::SamplerState];
				slots[sampler.second.set] = std::max(slots[sampler.second.set], sampler.second.slot + 1);
			}
		}

		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			for (UINT32 j = 0; j < mNumSets[i]; j++)
				mNumElements[i] += slotsPerSet[i][j];

			mTotalNumElements += mNumElements[i];
		}

		UINT32 setOffsetsSize = sizeof(UINT32) * totalNumSets;
		mData = (UINT8*)bs_alloc(setOffsetsSize);

		UINT8* dataPtr = mData;
		for (UINT32 i = 0; i < (UINT32)ParamType::Count; i++)
		{
			mOffsets[i] = (UINT32*)dataPtr;
			dataPtr += sizeof(UINT32) * mNumSets[i];

			if (mNumSets[i] == 0)
				continue;

			mOffsets[i][0] = 0;

			for (UINT32 j = 0; j < mNumSets[i] - 1; j++)
				mOffsets[i][j + 1] = mOffsets[i][j] + slotsPerSet[i][j];
		}

		bs_stack_free(slotsPerSetData);
	}

	GpuPipelineParamInfoBase::~GpuPipelineParamInfoBase()
	{
		bs_free(mData);
	}

	UINT32 GpuPipelineParamInfoBase::getSequentialSlot(ParamType type, UINT32 set, UINT32 slot) const
	{
#if BS_DEBUG_MODE
		if (set >= mNumSets[(int)type])
		{
			LOGERR("Set index out of range: Valid range: 0 .. " +
				   toString(mNumSets[(int)type] - 1) + ". Requested: " + toString(set));
			return (UINT32)-1;
		}
#endif

		UINT32 globalSlot = mOffsets[(int)type][set] + slot;

#if BS_DEBUG_MODE
		if (globalSlot >= mNumElements[(int)type])
		{
			UINT32 maxSlot;
			if (set < (mNumSets[(int)type] - 1))
				maxSlot = mOffsets[(int)type][set + 1];
			else
				maxSlot = mNumElements[(int)type];

			maxSlot -= mOffsets[(int)type][set];

			LOGERR("Slot index out of range: Valid range: 0 .. " +
				   toString(maxSlot - 1) + ". Requested: " + toString(slot));
			return (UINT32)-1;
		}
#endif

		return globalSlot;
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