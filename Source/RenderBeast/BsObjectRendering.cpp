//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsObjectRendering.h"
#include "Material/BsShader.h"
#include "RenderAPI/BsGpuParams.h"
#include "BsRenderBeast.h"
#include "Material/BsMaterial.h"
#include "Mesh/BsMesh.h"
#include "Animation/BsSkeleton.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Material/BsGpuParamsSet.h"
#include "Animation/BsMorphShapes.h"
#include "Animation/BsAnimationManager.h"

namespace bs { namespace ct
{
	PerFrameParamDef gPerFrameParamDef;

	ObjectRenderer::ObjectRenderer()
	{
		mPerFrameParamBuffer = gPerFrameParamDef.createBuffer();
	}

	void ObjectRenderer::initElement(RendererObject& owner, BeastRenderableElement& element)
	{
		SPtr<Shader> shader = element.material->getShader();
		if (shader == nullptr)
		{
			LOGWRN("Missing shader on material.");
			return;
		}

		SPtr<GpuParams> gpuParams = element.params->getGpuParams();

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the provided
		// buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		gpuParams->setParamBlockBuffer("PerFrame", mPerFrameParamBuffer);
		gpuParams->setParamBlockBuffer("PerObject", owner.perObjectParamBuffer);
		gpuParams->setParamBlockBuffer("PerCall", owner.perCallParamBuffer);

		gpuParams->getParamInfo()->getBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock, 
			"PerCamera",
			element.perCameraBindings
		);

		gpuParams->getParamInfo()->getBindings(
			GpuPipelineParamInfoBase::ParamType::ParamBlock,
			"GridParams",
			element.gridParamsBindings
		);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLights"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLights", element.lightsBufferParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize", element.gridLightOffsetsAndSizeParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLightIndices"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLightIndices", element.gridLightIndicesParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize", element.gridProbeOffsetsAndSizeParam);

		element.imageBasedParams.populate(gpuParams, GPT_FRAGMENT_PROGRAM, true, true);

		if (gpuParams->hasBuffer(GPT_VERTEX_PROGRAM, "boneMatrices"))
			gpuParams->setBuffer(GPT_VERTEX_PROGRAM, "boneMatrices", element.boneMatrixBuffer);
	}

	void ObjectRenderer::setParamFrameParams(float time)
	{
		gPerFrameParamDef.gTime.set(mPerFrameParamBuffer, time);
	}
}}