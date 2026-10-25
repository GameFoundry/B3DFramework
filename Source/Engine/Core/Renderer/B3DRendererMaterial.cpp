//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererMaterial.h"

#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "FrameGraph/B3DFrameGraph.h"

using namespace b3d;
using namespace render;

B3D_LOG_CATEGORY(LogRendererMaterial)

TShared<render::GpuParameterSet> RendererMaterialBase::CreateGpuParameterSet(FrameGraphPass& pass, u32 set) const
{
	TShared<render::GpuParameterSet> parameters = CreateGpuParameterSet(set);
	pass.AddParameterSet(parameters);

	return parameters;
}

void RendererMaterialBase::Bind(GpuCommandBuffer& commandBuffer) const
{
	if(mGraphicsPipeline)
	{
		commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
		commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
	}
	else
		commandBuffer.SetGpuComputePipelineState(mComputePipeline);
}

void RendererMaterialBase::Bind(GpuCommandBuffer& commandBuffer, const TShared<GpuParameterSet>& parameters) const
{
	Bind(commandBuffer);
	commandBuffer.SetGpuParameterSet(parameters);
}

void RendererMaterialBase::ApplyShaderDefaults(GpuParameterSet& parameters) const
{
	for(const auto& entry : mShader->GetTextureParameters())
	{
		const u32 defaultValueIndex = entry.second.DefaultValueIndex;
		if(defaultValueIndex == ~0u)
			continue;

		for(const auto& variableName : entry.second.GpuVariableNames)
		{
			if(parameters.HasSampledTexture(variableName))
			{
				const TShared<Texture> texture = entry.second.Type == GPOT_TEXTURE3D ? mShader->GetDefault3DTexture(defaultValueIndex) : mShader->GetDefault2DTexture(defaultValueIndex);
				parameters.SetSampledTexture(variableName, texture);
			}
		}
	}

	for(const auto& entry : mShader->GetSamplerParameters())
	{
		const u32 defaultValueIndex = entry.second.DefaultValueIndex;
		if(defaultValueIndex == ~0u)
			continue;

		for(const auto& variableName : entry.second.GpuVariableNames)
		{
			if(parameters.HasSamplerState(variableName))
				parameters.SetSamplerState(variableName, mShader->GetDefaultSampler(defaultValueIndex));
		}
	}
}
