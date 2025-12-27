//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererMaterial.h"

#include "RenderAPI/B3DGpuCommandBuffer.h"

using namespace b3d;
using namespace render;

B3D_LOG_CATEGORY(RendererMaterial)

void RendererMaterialBase::Bind(GpuCommandBuffer& commandBuffer, bool bindParameters) const
{
	if(mGraphicsPipeline)
	{
		commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
		commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
	}
	else
		commandBuffer.SetGpuComputePipelineState(mComputePipeline);

	if(bindParameters)
		commandBuffer.SetGpuParameterSet(mGpuParameterSet);
}

void RendererMaterialBase::BindParameters(GpuCommandBuffer& commandBuffer) const
{
	commandBuffer.SetGpuParameterSet(mGpuParameterSet);
}
