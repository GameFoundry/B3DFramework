//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSamplerOverrides.h"
#include "B3DRenderBeastOptions.h"
#include "Material/B3DMaterial.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "RenderAPI/B3DGpuProgramParameterDescription.h"
#include "Material/B3DMaterialParameters.h"
#include "RenderAPI/B3DSamplerState.h"
#include "RenderAPI/B3DGpuDevice.h"

namespace b3d {
namespace render {

MaterialSamplerOverrides* SamplerOverrideUtility::GenerateSamplerOverrides(GpuDevice& gpuDevice, const SPtr<Shader>& shader, const SPtr<MaterialParameters>& params, const SPtr<MaterialParameterAdapter>& materialParameterAdapter, const SPtr<RenderBeastOptions>& options)
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
			auto result = params->GetParamIndex(samplerParam.first, MaterialParameters::ParamType::Sampler, GPDT_UNKNOWN, 0, paramIdx);

			// Parameter shouldn't be in the valid parameter list if it cannot be found
			B3D_ASSERT(result == MaterialParameters::GetParamResult::Success);
			const MaterialParametersBase::ParamData* materialParamData = params->GetParamData(paramIdx);

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

		u32 passCount = materialParameterAdapter->GetPassCount();

		// First pass just determine if we even need to override and count the number of sampler states
		u32* numSetsPerPass = (u32*)B3DStackAllocate<u32>(passCount);
		memset(numSetsPerPass, 0, sizeof(u32) * passCount);

		u32 totalNumSets = 0;
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<GpuParameterSet> gpuParameters = materialParameterAdapter->GetGpuParameterSet(passIndex);
			const SPtr<GpuPipelineParameterLayout> uniformLayout = gpuParameters->GetPipelineParameterLayout();

			// Count sets that have samplers
			u32 maxSamplerSet = 0;
			const u32 setCount = uniformLayout->GetSetCount();
			for(u32 setIndex = 0; setIndex < setCount; setIndex++)
			{
				if(uniformLayout->GetBindingCount(setIndex, GpuParameterType::Sampler) > 0)
					maxSamplerSet = std::max(maxSamplerSet, setIndex + 1);
			}

			numSetsPerPass[passIndex] = maxSamplerSet;
			totalNumSets += maxSamplerSet;
		}

		u32 totalNumSamplerStates = 0;
		u32* slotsPerSet = (u32*)B3DStackAllocate<u32>(totalNumSets);
		memset(slotsPerSet, 0, sizeof(u32) * totalNumSets);

		u32* slotsPerSetIter = slotsPerSet;
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<GpuParameterSet> paramsPtr = materialParameterAdapter->GetGpuParameterSet(passIndex);
			const SPtr<GpuPipelineParameterLayout> uniformLayout = paramsPtr->GetPipelineParameterLayout();

			for(u32 setIndex = 0; setIndex < numSetsPerPass[passIndex]; setIndex++)
			{
				const u32 samplerCount = uniformLayout->GetBindingCount(setIndex, GpuParameterType::Sampler);
				for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
				{
					const UniformInformation* uniformInformation = uniformLayout->TryGetUniformInformation(GpuParameterType::Sampler, setIndex, samplerIndex);
					if(uniformInformation)
						slotsPerSetIter[setIndex] = std::max(slotsPerSetIter[setIndex], uniformInformation->Slot + 1);
				}
			}

			for(u32 setIndex = 0; setIndex < numSetsPerPass[passIndex]; setIndex++)
				totalNumSamplerStates += slotsPerSetIter[setIndex];

			slotsPerSetIter += numSetsPerPass[passIndex];
		}

		u32 outputSize = sizeof(MaterialSamplerOverrides) +
			passCount * sizeof(PassSamplerOverrides) +
			totalNumSets * sizeof(u32*) +
			totalNumSamplerStates * sizeof(u32) +
			(u32)overrides.size() * sizeof(SamplerOverride);

		u8* outputData = (u8*)B3DAllocate(outputSize);
		output = (MaterialSamplerOverrides*)outputData;
		outputData += sizeof(MaterialSamplerOverrides);

		output->RefCount = 0;
		output->NumPasses = passCount;
		output->Passes = (PassSamplerOverrides*)outputData;
		output->IsDirty = true;
		outputData += sizeof(PassSamplerOverrides) * passCount;

		slotsPerSetIter = slotsPerSet;
		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<GpuParameterSet> paramsPtr = materialParameterAdapter->GetGpuParameterSet(passIndex);
			const SPtr<GpuPipelineParameterLayout> uniformLayout = paramsPtr->GetPipelineParameterLayout();

			PassSamplerOverrides& passOverrides = output->Passes[passIndex];
			passOverrides.NumSets = numSetsPerPass[passIndex];
			passOverrides.StateOverrides = (u32**)outputData;
			outputData += sizeof(u32*) * passOverrides.NumSets;

			for(u32 setIndex = 0; setIndex < passOverrides.NumSets; setIndex++)
			{
				passOverrides.StateOverrides[setIndex] = (u32*)outputData;
				outputData += sizeof(u32) * slotsPerSetIter[setIndex];

				// Initialize all slots to invalid
				for(u32 slotIndex = 0; slotIndex < slotsPerSetIter[setIndex]; slotIndex++)
					passOverrides.StateOverrides[setIndex][slotIndex] = (u32)-1;
			}

			// Fill in sampler overrides for each set
			for(u32 setIndex = 0; setIndex < passOverrides.NumSets; setIndex++)
			{
				const u32 samplerCount = uniformLayout->GetBindingCount(setIndex, GpuParameterType::Sampler);
				for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
				{
					const UniformInformation* uniformInformation = uniformLayout->TryGetUniformInformation(GpuParameterType::Sampler, setIndex, samplerIndex);
					if(!B3D_ENSURE(uniformInformation))
						continue;

					auto iterFind = overrideLookup.find(uniformInformation->Name);
					if(iterFind != overrideLookup.end())
						passOverrides.StateOverrides[setIndex][uniformInformation->Slot] = iterFind->second;
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
}} // namespace b3d::render
