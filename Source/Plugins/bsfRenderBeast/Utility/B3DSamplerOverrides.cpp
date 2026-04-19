//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DSamplerOverrides.h"
#include "B3DRenderBeastOptions.h"
#include "Material/B3DMaterial.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
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

		auto& samplerParams = shader->GetSamplerParameters();
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

		const u32 passCount = materialParameterAdapter->GetPassCount();

		// First pass: determine if we need to override and count sampler states
		// All materials should only use set 0
		u32 totalNumSamplerStates = 0;
		u32* slotsPerPass = (u32*)B3DStackAllocate<u32>(passCount);
		memset(slotsPerPass, 0, sizeof(u32) * passCount);

		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<GpuParameterSet> gpuParameters = materialParameterAdapter->GetGpuParameterSet(passIndex);
			B3D_ASSERT(gpuParameters->GetSet() == 0 && "Materials should only use set 0");

			const SPtr<GpuPipelineParameterSetLayout> layoutSet = gpuParameters->GetLayout();
			const u32 samplerCount = layoutSet->GetBindingCount(GpuParameterType::Sampler);

			for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
			{
				const UniformInformation* uniformInformation = layoutSet->TryGetUniformInformation(GpuParameterType::Sampler, samplerIndex);
				if(uniformInformation)
					slotsPerPass[passIndex] = std::max(slotsPerPass[passIndex], uniformInformation->Slot + 1);
			}

			totalNumSamplerStates += slotsPerPass[passIndex];
		}

		u32 outputSize = sizeof(MaterialSamplerOverrides) +
			passCount * sizeof(PassSamplerOverrides) +
			passCount * sizeof(u32*) +
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

		for(u32 passIndex = 0; passIndex < passCount; passIndex++)
		{
			SPtr<GpuParameterSet> paramsPtr = materialParameterAdapter->GetGpuParameterSet(passIndex);
			const SPtr<GpuPipelineParameterSetLayout> layoutSet = paramsPtr->GetLayout();

			PassSamplerOverrides& passOverrides = output->Passes[passIndex];
			passOverrides.NumSets = 1; // All materials use only set 0
			passOverrides.StateOverrides = (u32**)outputData;
			outputData += sizeof(u32*);

			passOverrides.StateOverrides[0] = (u32*)outputData;
			outputData += sizeof(u32) * slotsPerPass[passIndex];

			// Initialize all slots to invalid
			for(u32 slotIndex = 0; slotIndex < slotsPerPass[passIndex]; slotIndex++)
				passOverrides.StateOverrides[0][slotIndex] = (u32)-1;

			// Fill in sampler overrides
			const u32 samplerCount = layoutSet->GetBindingCount(GpuParameterType::Sampler);
			for(u32 samplerIndex = 0; samplerIndex < samplerCount; ++samplerIndex)
			{
				const UniformInformation* uniformInformation = layoutSet->TryGetUniformInformation(GpuParameterType::Sampler, samplerIndex);
				if(!B3D_ENSURE(uniformInformation))
					continue;

				auto iterFind = overrideLookup.find(uniformInformation->Name);
				if(iterFind != overrideLookup.end())
					passOverrides.StateOverrides[0][uniformInformation->Slot] = iterFind->second;
			}
		}

		output->NumOverrides = (u32)overrides.size();
		output->Overrides = (SamplerOverride*)outputData;

		for(u32 i = 0; i < output->NumOverrides; i++)
		{
			new(&output->Overrides[i].State) SPtr<SamplerState>();
			output->Overrides[i] = overrides[i];
		}

		B3DStackFree(slotsPerPass);
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
