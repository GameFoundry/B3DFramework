//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSamplerOverrides.h"
#include "BsRenderBeastOptions.h"
#include "BsMaterial.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsGpuParamDesc.h"
#include "BsMaterialParams.h"
#include "BsSamplerState.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
{
	MaterialSamplerOverrides* SamplerOverrideUtility::generateSamplerOverrides(const SPtr<ShaderCore>& shader, 
		const SPtr<MaterialParamsCore>& params, const SPtr<GpuParamsSetCore>& paramsSet, 
		const SPtr<RenderBeastOptions>& options)
	{
		MaterialSamplerOverrides* output = nullptr;

		if (shader == nullptr)
			return nullptr;

		bs_frame_mark();
		{
			// Generate a list of all sampler state overrides
			FrameUnorderedMap<String, UINT32> overrideLookup;
			Vector<SamplerOverride> overrides;

			auto& samplerParams = shader->getSamplerParams();
			for(auto& samplerParam : samplerParams)
			{
				UINT32 paramIdx;
				auto result = params->getParamIndex(samplerParam.first, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 
					0, paramIdx);

				// Parameter shouldn't be in the valid parameter list if it cannot be found
				assert(result == MaterialParams::GetParamResult::Success);
				const MaterialParamsBase::ParamData* materialParamData = params->getParamData(paramIdx);

				UINT32 overrideIdx = (UINT32)overrides.size();
				overrides.push_back(SamplerOverride());
				SamplerOverride& override = overrides.back();

				SPtr<SamplerStateCore> samplerState;
				params->getSamplerState(materialParamData->index, samplerState);

				if (samplerState == nullptr)
					samplerState = SamplerStateCore::getDefault();

				override.paramIdx = paramIdx;
				override.originalStateHash = samplerState->getProperties().getHash();

				if (checkNeedsOverride(samplerState, options))
					override.state = generateSamplerOverride(samplerState, options);
				else
					override.state = samplerState;

				overrideLookup[samplerParam.first] = overrideIdx;
			}

			UINT32 numPasses = paramsSet->getNumPasses();

			// First pass just determine if we even need to override and count the number of sampler states
			UINT32 totalNumSamplerStates = 0;
			for (UINT32 i = 0; i < numPasses; i++)
			{
				UINT32 maxSamplerSlot = 0;

				for (UINT32 j = 0; j < GpuParamsSetCore::NUM_STAGES; j++)
				{
					SPtr<GpuParamsCore> paramsPtr = paramsSet->getParamByIdx(j, i);
					if (paramsPtr == nullptr)
						continue;

					const GpuParamDesc& paramDesc = paramsPtr->getParamDesc();

					for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
					{
						UINT32 slot = iter->second.slot;
						maxSamplerSlot = std::max(maxSamplerSlot, slot + 1);
					}

					totalNumSamplerStates += maxSamplerSlot;
				}
			}

			UINT32 outputSize = sizeof(MaterialSamplerOverrides) +
				numPasses * (sizeof(PassSamplerOverrides) + GpuParamsSetCore::NUM_STAGES * sizeof(StageSamplerOverrides)) +
				totalNumSamplerStates * sizeof(UINT32) +
				(UINT32)overrides.size() * sizeof(SamplerOverride);

			UINT8* outputData = (UINT8*)bs_alloc(outputSize);
			output = (MaterialSamplerOverrides*)outputData;
			outputData += sizeof(MaterialSamplerOverrides);

			output->refCount = 0;
			output->numPasses = numPasses;
			output->passes = (PassSamplerOverrides*)outputData;
			outputData += sizeof(PassSamplerOverrides) * numPasses;

			for (UINT32 i = 0; i < numPasses; i++)
			{
				PassSamplerOverrides& passOverrides = output->passes[i];
				passOverrides.numStages = GpuParamsSetCore::NUM_STAGES;
				passOverrides.stages = (StageSamplerOverrides*)outputData;
				outputData += sizeof(StageSamplerOverrides) * GpuParamsSetCore::NUM_STAGES;

				for (UINT32 j = 0; j < passOverrides.numStages; j++)
				{
					StageSamplerOverrides& stageOverrides = passOverrides.stages[j];
					stageOverrides.numStates = 0;
					stageOverrides.stateOverrides = (UINT32*)outputData;

					SPtr<GpuParamsCore> paramsPtr = paramsSet->getParamByIdx(j, i);
					if (paramsPtr == nullptr)
						continue;

					const GpuParamDesc& paramDesc = paramsPtr->getParamDesc();

					for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
					{
						UINT32 slot = iter->second.slot;
						while (slot > stageOverrides.numStates)
						{
							stageOverrides.stateOverrides[stageOverrides.numStates] = (UINT32)-1;
							stageOverrides.numStates++;
						}

						stageOverrides.numStates = std::max(stageOverrides.numStates, slot + 1);

						auto iterFind = overrideLookup.find(iter->first);
						if (iterFind != overrideLookup.end())
							stageOverrides.stateOverrides[slot] = iterFind->second;
						else
							stageOverrides.stateOverrides[slot] = (UINT32)-1;						
					}

					outputData += sizeof(UINT32) * stageOverrides.numStates;
				}
			}

			output->numOverrides = (UINT32)overrides.size();
			output->overrides = (SamplerOverride*)outputData;

			for(UINT32 i = 0; i < output->numOverrides; i++)
			{
				new (&output->overrides[i].state) SPtr<SamplerStateCore>();
				output->overrides[i] = overrides[i];
			}
		}
		bs_frame_clear();

		return output;
	}

	void SamplerOverrideUtility::destroySamplerOverrides(MaterialSamplerOverrides* overrides)
	{
		if (overrides != nullptr)
		{
			for (UINT32 i = 0; i < overrides->numOverrides; i++)
				overrides->overrides[i].state.~SPtr<SamplerStateCore>();

			bs_free(overrides);
			overrides = nullptr;
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