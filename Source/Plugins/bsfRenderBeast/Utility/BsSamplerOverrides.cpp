//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSamplerOverrides.h"
#include "BsRenderBeastOptions.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParameters.h"
#include "Material/BsGpuParamsSet.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "Material/BsMaterialParams.h"
#include "RenderAPI/BsSamplerState.h"
#include "Managers/BsRenderStateManager.h"

namespace bs {
namespace ct {

MaterialSamplerOverrides* SamplerOverrideUtility::GenerateSamplerOverrides(const SPtr<Shader>& shader, const SPtr<MaterialParams>& params, const SPtr<GpuParamsSet>& paramsSet, const SPtr<RenderBeastOptions>& options)
{
	MaterialSamplerOverrides* output = nullptr;

	if(shader == nullptr)
		return nullptr;

	B3DMarkAllocatorFrame();
	{
		// Generate a list of all sampler state overrides
		FrameUnorderedMap<String, u32> overrideLookup;
		Vector<SamplerOverride> overrides;

		auto& samplerParams = shader->GetSamplerParams();
		for(auto& samplerParam : samplerParams)
		{
			u32 paramIdx;
			auto result = params->GetParamIndex(samplerParam.first, MaterialParams::ParamType::Sampler, GPDT_UNKNOWN, 0, paramIdx);

			// Parameter shouldn't be in the valid parameter list if it cannot be found
			B3D_ASSERT(result == MaterialParams::GetParamResult::Success);
			const MaterialParamsBase::ParamData* materialParamData = params->GetParamData(paramIdx);

			u32 overrideIdx = (u32)overrides.size();
			overrides.push_back(SamplerOverride());
			SamplerOverride& override = overrides.back();

			SPtr<SamplerState> samplerState;
			params->GetSamplerState(*materialParamData, samplerState);

			if(samplerState == nullptr)
				samplerState = SamplerState::GetDefault();

			override.ParamIdx = paramIdx;

			if(CheckNeedsOverride(samplerState, options))
				override.State = GenerateSamplerOverride(samplerState, options);
			else
				override.State = samplerState;

			override.OriginalStateHash = override.State->GetProperties().GetHash();

			for(auto& entry : samplerParam.second.GpuVariableNames)
				overrideLookup[entry] = overrideIdx;
		}

		u32 numPasses = paramsSet->GetNumPasses();

		// First pass just determine if we even need to override and count the number of sampler states
		u32* numSetsPerPass = (u32*)B3DStackAllocate<u32>(numPasses);
		memset(numSetsPerPass, 0, sizeof(u32) * numPasses);

		u32 totalNumSets = 0;
		for(u32 i = 0; i < numPasses; i++)
		{
			u32 maxSamplerSet = 0;

			SPtr<GpuParameters> paramsPtr = paramsSet->GetGpuParams(i);
			for(u32 j = 0; j < GpuParamsSet::kNumStages; j++)
			{
				GpuProgramType progType = (GpuProgramType)j;
				SPtr<GpuParamDesc> paramDesc = paramsPtr->GetParameterInformation(progType);
				if(paramDesc == nullptr)
					continue;

				for(auto iter = paramDesc->Samplers.begin(); iter != paramDesc->Samplers.end(); ++iter)
				{
					u32 set = iter->second.Set;
					maxSamplerSet = std::max(maxSamplerSet, set + 1);
				}
			}

			numSetsPerPass[i] = maxSamplerSet;
			totalNumSets += maxSamplerSet;
		}

		u32 totalNumSamplerStates = 0;
		u32* slotsPerSet = (u32*)B3DStackAllocate<u32>(totalNumSets);
		memset(slotsPerSet, 0, sizeof(u32) * totalNumSets);

		u32* slotsPerSetIter = slotsPerSet;
		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<GpuParameters> paramsPtr = paramsSet->GetGpuParams(i);
			for(u32 j = 0; j < GpuParamsSet::kNumStages; j++)
			{
				GpuProgramType progType = (GpuProgramType)j;
				SPtr<GpuParamDesc> paramDesc = paramsPtr->GetParameterInformation(progType);
				if(paramDesc == nullptr)
					continue;

				for(auto iter = paramDesc->Samplers.begin(); iter != paramDesc->Samplers.end(); ++iter)
				{
					u32 set = iter->second.Set;
					u32 slot = iter->second.Slot;
					slotsPerSetIter[set] = std::max(slotsPerSetIter[set], slot + 1);
				}
			}

			for(u32 j = 0; j < numSetsPerPass[i]; j++)
				totalNumSamplerStates += slotsPerSetIter[j];

			slotsPerSetIter += numSetsPerPass[i];
		}

		u32 outputSize = sizeof(MaterialSamplerOverrides) +
			numPasses * sizeof(PassSamplerOverrides) +
			totalNumSets * sizeof(u32*) +
			totalNumSamplerStates * sizeof(u32) +
			(u32)overrides.size() * sizeof(SamplerOverride);

		u8* outputData = (u8*)B3DAllocate(outputSize);
		output = (MaterialSamplerOverrides*)outputData;
		outputData += sizeof(MaterialSamplerOverrides);

		output->RefCount = 0;
		output->NumPasses = numPasses;
		output->Passes = (PassSamplerOverrides*)outputData;
		output->IsDirty = true;
		outputData += sizeof(PassSamplerOverrides) * numPasses;

		slotsPerSetIter = slotsPerSet;
		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<GpuParameters> paramsPtr = paramsSet->GetGpuParams(i);

			PassSamplerOverrides& passOverrides = output->Passes[i];
			passOverrides.NumSets = numSetsPerPass[i];
			passOverrides.StateOverrides = (u32**)outputData;
			outputData += sizeof(u32*) * passOverrides.NumSets;

			for(u32 j = 0; j < passOverrides.NumSets; j++)
			{
				passOverrides.StateOverrides[j] = (u32*)outputData;
				outputData += sizeof(u32) * slotsPerSetIter[j];
			}

			for(u32 j = 0; j < GpuParamsSet::kNumStages; j++)
			{
				GpuProgramType progType = (GpuProgramType)j;
				SPtr<GpuParamDesc> paramDesc = paramsPtr->GetParameterInformation(progType);
				if(paramDesc == nullptr)
					continue;

				u32 numStates = 0;
				for(auto iter = paramDesc->Samplers.begin(); iter != paramDesc->Samplers.end(); ++iter)
				{
					u32 set = iter->second.Set;
					u32 slot = iter->second.Slot;
					while(slot > numStates)
					{
						passOverrides.StateOverrides[set][numStates] = (u32)-1;
						numStates++;
					}

					numStates = std::max(numStates, slot + 1);

					auto iterFind = overrideLookup.find(iter->first);
					if(iterFind != overrideLookup.end())
						passOverrides.StateOverrides[set][slot] = iterFind->second;
					else
						passOverrides.StateOverrides[set][slot] = (u32)-1;
				}
			}

			slotsPerSetIter += passOverrides.NumSets;
		}

		output->NumOverrides = (u32)overrides.size();
		output->Overrides = (SamplerOverride*)outputData;

		for(u32 i = 0; i < output->NumOverrides; i++)
		{
			new(&output->Overrides[i].State) SPtr<SamplerState>();
			output->Overrides[i] = overrides[i];
		}

		B3DStackFree(slotsPerSet);
		B3DStackFree(numSetsPerPass);
	}
	B3DClearAllocatorFrame();

	return output;
}

void SamplerOverrideUtility::DestroySamplerOverrides(MaterialSamplerOverrides* overrides)
{
	if(overrides != nullptr)
	{
		for(u32 i = 0; i < overrides->NumOverrides; i++)
			overrides->Overrides[i].State.~SPtr<SamplerState>();

		B3DFree(overrides);
		overrides = nullptr;
	}
}

bool SamplerOverrideUtility::CheckNeedsOverride(const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options)
{
	const SamplerProperties& props = samplerState->GetProperties();

	switch(options->Filtering)
	{
	case RenderBeastFiltering::Bilinear:
		{
			if(props.GetTextureFiltering(FT_MIN) != FO_LINEAR)
				return true;

			if(props.GetTextureFiltering(FT_MAG) != FO_LINEAR)
				return true;

			if(props.GetTextureFiltering(FT_MIP) != FO_POINT)
				return true;
		}
		break;
	case RenderBeastFiltering::Trilinear:
		{
			if(props.GetTextureFiltering(FT_MIN) != FO_LINEAR)
				return true;

			if(props.GetTextureFiltering(FT_MAG) != FO_LINEAR)
				return true;

			if(props.GetTextureFiltering(FT_MIP) != FO_LINEAR)
				return true;
		}
		break;
	case RenderBeastFiltering::Anisotropic:
		{
			if(props.GetTextureFiltering(FT_MIN) != FO_ANISOTROPIC)
				return true;

			if(props.GetTextureFiltering(FT_MAG) != FO_ANISOTROPIC)
				return true;

			if(props.GetTextureFiltering(FT_MIP) != FO_ANISOTROPIC)
				return true;

			if(props.GetTextureAnisotropy() != options->AnisotropyMax)
				return true;
		}
		break;
	}

	return false;
}

SPtr<SamplerState> SamplerOverrideUtility::GenerateSamplerOverride(const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options)
{
	const SamplerProperties& props = samplerState->GetProperties();
	SamplerStateInformation desc = props.GetDesc();

	switch(options->Filtering)
	{
	case RenderBeastFiltering::Bilinear:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_POINT;
		break;
	case RenderBeastFiltering::Trilinear:
		desc.MinFilter = FO_LINEAR;
		desc.MagFilter = FO_LINEAR;
		desc.MipFilter = FO_LINEAR;
		break;
	case RenderBeastFiltering::Anisotropic:
		desc.MinFilter = FO_ANISOTROPIC;
		desc.MagFilter = FO_ANISOTROPIC;
		desc.MipFilter = FO_ANISOTROPIC;
		break;
	}

	desc.MaxAniso = options->AnisotropyMax;

	return RenderStateManager::Instance().CreateSamplerState(desc);
}
}} // namespace bs::ct
