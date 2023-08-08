//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsSamplerOverrides.h"
#include "BsRenderBeastOptions.h"
#include "Material/BsMaterial.h"
#include "RenderAPI/BsGpuParameters.h"
#include "Material/BsGpuParamsSet.h"
#include "RenderAPI/BsGpuProgramParameterDescription.h"
#include "Material/BsMaterialParams.h"
#include "RenderAPI/BsSamplerState.h"
#include "RenderAPI/BsGpuDevice.h"

namespace bs {
namespace ct {

MaterialSamplerOverrides* SamplerOverrideUtility::GenerateSamplerOverrides(GpuDevice& gpuDevice, const SPtr<Shader>& shader, const SPtr<MaterialParams>& params, const SPtr<GpuParamsSet>& paramsSet, const SPtr<RenderBeastOptions>& options)
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

			if (samplerState == nullptr)
				samplerState = gpuDevice.FindOrCreateSamplerState(SamplerStateCreateInformation());

			override.ParamIdx = paramIdx;

			if(CheckNeedsOverride(samplerState, options))
				override.State = GenerateSamplerOverride(gpuDevice, samplerState, options);
			else
				override.State = samplerState;

			override.OriginalStateHash = B3DHash(override.State->GetInformation());

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
			const SPtr<GpuPipelineParameterLayout> uniformLayout = paramsPtr->GetPipelineParameterInformation();

			const u32 samplerCount = uniformLayout->GetBindingCount(GpuParameterType::Sampler);
			for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
			{
				u32 set = uniformLayout->GetBinding(GpuParameterType::Sampler, samplerIndex).Set;
				maxSamplerSet = std::max(maxSamplerSet, set + 1);
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
			const SPtr<GpuPipelineParameterLayout> uniformLayout = paramsPtr->GetPipelineParameterInformation();

			const u32 samplerCount = uniformLayout->GetBindingCount(GpuParameterType::Sampler);
			for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
			{
				const GpuParameterBinding& binding = uniformLayout->GetBinding(GpuParameterType::Sampler, samplerIndex);
				slotsPerSetIter[binding.Set] = std::max(slotsPerSetIter[binding.Set], binding.Slot + 1);
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
			const SPtr<GpuPipelineParameterLayout> uniformLayout = paramsPtr->GetPipelineParameterInformation();

			PassSamplerOverrides& passOverrides = output->Passes[i];
			passOverrides.NumSets = numSetsPerPass[i];
			passOverrides.StateOverrides = (u32**)outputData;
			outputData += sizeof(u32*) * passOverrides.NumSets;

			for(u32 j = 0; j < passOverrides.NumSets; j++)
			{
				passOverrides.StateOverrides[j] = (u32*)outputData;
				outputData += sizeof(u32) * slotsPerSetIter[j];
			}

			u32 numStates = 0;
			const u32 samplerCount = uniformLayout->GetBindingCount(GpuParameterType::Sampler);
			for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
			{
				const GpuParameterBinding& binding = uniformLayout->GetBinding(GpuParameterType::Sampler, samplerIndex);
				const UniformInformation* uniformInformation = uniformLayout->TryGetUniformInformation(GpuParameterType::Sampler, samplerIndex);
				if(!B3D_ENSURE(uniformInformation))
					continue;

				while(binding.Slot > numStates)
				{
					passOverrides.StateOverrides[binding.Set][numStates] = (u32)-1;
					numStates++;
				}

				numStates = std::max(numStates, binding.Slot + 1);

				auto iterFind = overrideLookup.find(uniformInformation->Name);
				if(iterFind != overrideLookup.end())
					passOverrides.StateOverrides[binding.Set][binding.Slot] = iterFind->second;
				else
					passOverrides.StateOverrides[binding.Set][binding.Slot] = (u32)-1;
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
	const SamplerStateInformation& samplerStateInformation = samplerState->GetInformation();

	switch(options->Filtering)
	{
	case RenderBeastFiltering::Bilinear:
		{
			if(samplerStateInformation.MinFilter != FO_LINEAR)
				return true;

			if(samplerStateInformation.MagFilter != FO_LINEAR)
				return true;

			if(samplerStateInformation.MipFilter != FO_POINT)
				return true;
		}
		break;
	case RenderBeastFiltering::Trilinear:
		{
			if(samplerStateInformation.MinFilter != FO_LINEAR)
				return true;

			if(samplerStateInformation.MagFilter != FO_LINEAR)
				return true;

			if(samplerStateInformation.MipFilter != FO_LINEAR)
				return true;
		}
		break;
	case RenderBeastFiltering::Anisotropic:
		{
			if(samplerStateInformation.MinFilter != FO_ANISOTROPIC)
				return true;

			if(samplerStateInformation.MagFilter != FO_ANISOTROPIC)
				return true;

			if(samplerStateInformation.MipFilter != FO_ANISOTROPIC)
				return true;

			if(samplerStateInformation.MaxAniso != options->AnisotropyMax)
				return true;
		}
		break;
	}

	return false;
}

SPtr<SamplerState> SamplerOverrideUtility::GenerateSamplerOverride(GpuDevice& gpuDevice, const SPtr<SamplerState>& samplerState, const SPtr<RenderBeastOptions>& options)
{
	SamplerStateCreateInformation samplerStateCreateInformation = samplerState->GetInformation();

	switch(options->Filtering)
	{
	case RenderBeastFiltering::Bilinear:
		samplerStateCreateInformation.MinFilter = FO_LINEAR;
		samplerStateCreateInformation.MagFilter = FO_LINEAR;
		samplerStateCreateInformation.MipFilter = FO_POINT;
		break;
	case RenderBeastFiltering::Trilinear:
		samplerStateCreateInformation.MinFilter = FO_LINEAR;
		samplerStateCreateInformation.MagFilter = FO_LINEAR;
		samplerStateCreateInformation.MipFilter = FO_LINEAR;
		break;
	case RenderBeastFiltering::Anisotropic:
		samplerStateCreateInformation.MinFilter = FO_ANISOTROPIC;
		samplerStateCreateInformation.MagFilter = FO_ANISOTROPIC;
		samplerStateCreateInformation.MipFilter = FO_ANISOTROPIC;
		break;
	}

	samplerStateCreateInformation.MaxAniso = options->AnisotropyMax;

	return gpuDevice.FindOrCreateSamplerState(samplerStateCreateInformation);
}
}} // namespace bs::ct
