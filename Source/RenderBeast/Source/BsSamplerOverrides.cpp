//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSamplerOverrides.h"
#include "BsRenderBeastOptions.h"
#include "BsMaterial.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsGpuParamDesc.h"
#include "BsSamplerState.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
{
	MaterialSamplerOverrides* SamplerOverrideUtility::generateSamplerOverrides(const SPtr<GpuParamsSetCore>& paramsSet, const SPtr<RenderBeastOptions>& options)
	{
		UINT32 numPasses = paramsSet->getNumPasses();

		// First pass just determine if we even need to override and count the number of sampler states
		UINT32 totalNumSamplerStates = 0;
		for (UINT32 i = 0; i < numPasses; i++)
		{
			UINT32 maxSamplerSlot = 0;

			for (UINT32 j = 0; j < GpuParamsSetCore::NUM_PARAMS; j++)
			{
				SPtr<GpuParamsCore> params = paramsSet->getParamByIdx(j, i);
				if (params == nullptr)
					continue;

				const GpuParamDesc& paramDesc = params->getParamDesc();

				for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
				{
					UINT32 slot = iter->second.slot;
					maxSamplerSlot = std::max(maxSamplerSlot, slot + 1);
				}

				totalNumSamplerStates += maxSamplerSlot;
			}
		}

		UINT32 outputSize = sizeof(MaterialSamplerOverrides) +
			numPasses * (sizeof(PassSamplerOverrides) + GpuParamsSetCore::NUM_PARAMS * sizeof(StageSamplerOverrides)) +
			totalNumSamplerStates * sizeof(SPtr<SamplerStateCore>);

		UINT8* outputData = (UINT8*)bs_alloc(outputSize);
		MaterialSamplerOverrides* output = (MaterialSamplerOverrides*)outputData;
		outputData += sizeof(MaterialSamplerOverrides);

		output->refCount = 0;
		output->numPasses = numPasses;
		output->passes = (PassSamplerOverrides*)outputData;
		outputData += sizeof(PassSamplerOverrides) * numPasses;

		bs_frame_mark();
		{
			FrameUnorderedMap<SPtr<SamplerStateCore>, SPtr<SamplerStateCore>> overrideMap;

			for (UINT32 i = 0; i < numPasses; i++)
			{
				PassSamplerOverrides& passOverrides = output->passes[i];
				passOverrides.numStages = GpuParamsSetCore::NUM_PARAMS;
				passOverrides.stages = (StageSamplerOverrides*)outputData;
				outputData += sizeof(StageSamplerOverrides) * GpuParamsSetCore::NUM_PARAMS;

				for (UINT32 j = 0; j < passOverrides.numStages; j++)
				{
					StageSamplerOverrides& stageOverrides = passOverrides.stages[j];
					stageOverrides.numStates = 0;
					stageOverrides.stateOverrides = (SPtr<SamplerStateCore>*)outputData;

					SPtr<GpuParamsCore> params = paramsSet->getParamByIdx(j, i);
					if (params == nullptr)
						continue;

					const GpuParamDesc& paramDesc = params->getParamDesc();

					for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
					{
						UINT32 slot = iter->second.slot;
						while (slot >= stageOverrides.numStates)
						{
							new (&stageOverrides.stateOverrides[stageOverrides.numStates]) SPtr<SamplerStateCore>();
							stageOverrides.numStates++;
						}

						stageOverrides.numStates = std::max(stageOverrides.numStates, slot + 1);

						SPtr<SamplerStateCore> samplerState = params->getSamplerState(slot);
						if (samplerState == nullptr)
							samplerState = SamplerStateCore::getDefault();

						bool needsOverride = checkNeedsOverride(samplerState, options);

						if (needsOverride)
						{
							auto findIter = overrideMap.find(samplerState);
							if (findIter != overrideMap.end())
							{
								stageOverrides.stateOverrides[slot] = findIter->second;
							}
							else
							{
								SPtr<SamplerStateCore> newState = generateSamplerOverride(samplerState, options);

								overrideMap[samplerState] = newState;
								stageOverrides.stateOverrides[slot] = newState;
							}
						}
						else
						{
							stageOverrides.stateOverrides[slot] = samplerState;
						}
					}

					outputData += sizeof(SPtr<SamplerStateCore>) * stageOverrides.numStates;
				}
			}
		}

		bs_frame_clear();

		return output;
	}

	void SamplerOverrideUtility::destroySamplerOverrides(MaterialSamplerOverrides* overrides)
	{
		if (overrides != nullptr)
		{
			for (UINT32 i = 0; i < overrides->numPasses; i++)
			{
				PassSamplerOverrides& passOverrides = overrides->passes[i];

				for (UINT32 j = 0; j < passOverrides.numStages; j++)
				{
					StageSamplerOverrides& stageOverrides = passOverrides.stages[j];

					for (UINT32 k = 0; k < stageOverrides.numStates; k++)
						stageOverrides.stateOverrides[k].~SPtr<SamplerStateCore>();
				}
			}

			bs_free(overrides);
		}
	}

	bool SamplerOverrideUtility::checkNeedsOverride(const SPtr<SamplerStateCore>& samplerState, const SPtr<RenderBeastOptions>& options)
	{
		const SamplerProperties& props = samplerState->getProperties();

		switch (options->filtering)
		{
		case RenderBeastFiltering::Bilinear:
		{
			if (props.getTextureFiltering(FT_MIN) != FO_LINEAR)
				return true;

			if (props.getTextureFiltering(FT_MAG) != FO_LINEAR)
				return true;

			if (props.getTextureFiltering(FT_MIP) != FO_POINT)
				return true;
		}
			break;
		case RenderBeastFiltering::Trilinear:
		{
			if (props.getTextureFiltering(FT_MIN) != FO_LINEAR)
				return true;

			if (props.getTextureFiltering(FT_MAG) != FO_LINEAR)
				return true;

			if (props.getTextureFiltering(FT_MIP) != FO_LINEAR)
				return true;
		}
			break;
		case RenderBeastFiltering::Anisotropic:
		{
			if (props.getTextureFiltering(FT_MIN) != FO_ANISOTROPIC)
				return true;

			if (props.getTextureFiltering(FT_MAG) != FO_ANISOTROPIC)
				return true;

			if (props.getTextureFiltering(FT_MIP) != FO_ANISOTROPIC)
				return true;

			if (props.getTextureAnisotropy() != options->anisotropyMax)
				return true;
		}
			break;
		}

		return false;
	}

	SPtr<SamplerStateCore> SamplerOverrideUtility::generateSamplerOverride(const SPtr<SamplerStateCore>& samplerState, const SPtr<RenderBeastOptions>& options)
	{
		const SamplerProperties& props = samplerState->getProperties();
		SAMPLER_STATE_DESC desc = props.getDesc();

		switch (options->filtering)
		{
		case RenderBeastFiltering::Bilinear:
			desc.minFilter = FO_LINEAR;
			desc.magFilter = FO_LINEAR;
			desc.mipFilter = FO_POINT;
			break;
		case RenderBeastFiltering::Trilinear:
			desc.minFilter = FO_LINEAR;
			desc.magFilter = FO_LINEAR;
			desc.mipFilter = FO_LINEAR;
			break;
		case RenderBeastFiltering::Anisotropic:
			desc.minFilter = FO_ANISOTROPIC;
			desc.magFilter = FO_ANISOTROPIC;
			desc.mipFilter = FO_ANISOTROPIC;
			break;
		}

		desc.maxAniso = options->anisotropyMax;

		return RenderStateCoreManager::instance().createSamplerState(desc);
	}
}