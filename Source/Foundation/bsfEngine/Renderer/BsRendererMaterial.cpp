//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererMaterial.h"

using namespace bs;
using namespace ct;

void RendererMaterialBase::Bind(bool bindParameters) const
{
	RenderAPI& renderAPI = GetRenderAPI();

	if(mGraphicsPipeline)
	{
		renderAPI.SetGraphicsPipeline(mGraphicsPipeline);
		renderAPI.SetStencilRef(mStencilReferenceValue);
	}
	else
		renderAPI.SetComputePipeline(mComputePipeline);

	if(bindParameters)
		renderAPI.SetGpuParams(mGPUParameters);
}

void RendererMaterialBase::BindParameters() const
{
	GetRenderAPI().SetGpuParams(mGPUParameters);
}
