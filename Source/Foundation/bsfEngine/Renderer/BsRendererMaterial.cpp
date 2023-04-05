//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererMaterial.h"

#include "RenderAPI/BsCommandBuffer.h"

using namespace bs;
using namespace ct;

void RendererMaterialBase::Bind(CommandBuffer& commandBuffer, bool bindParameters) const
{
	if(mGraphicsPipeline)
	{
		commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
		commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
	}
	else
		commandBuffer.SetGpuComputePipelineState(mComputePipeline);

	if(bindParameters)
		commandBuffer.SetGpuParameters(mGPUParameters);
}

void RendererMaterialBase::BindParameters(CommandBuffer& commandBuffer) const
{
	commandBuffer.SetGpuParameters(mGPUParameters);
}
