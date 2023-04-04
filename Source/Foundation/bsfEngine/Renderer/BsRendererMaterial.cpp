//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererMaterial.h"

#include "RenderAPI/BsCommandBuffer.h"

using namespace bs;
using namespace ct;

void RendererMaterialBase::Bind(CommandBuffer& commandBuffer, bool bindParameters) const
{
	RenderAPI& renderAPI = GetRenderAPI();

	if(mGraphicsPipeline)
	{
		commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
		renderAPI.SetStencilRef(mStencilReferenceValue, commandBuffer.GetShared());
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
