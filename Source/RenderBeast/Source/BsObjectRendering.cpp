//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsObjectRendering.h"
#include "BsShader.h"
#include "BsGpuParams.h"
#include "BsRenderBeast.h"
#include "BsMaterial.h"
#include "BsMesh.h"
#include "BsSkeleton.h"
#include "BsGpuBuffer.h"
#include "BsGpuParamsSet.h"
#include "BsMorphShapes.h"
#include "BsAnimationManager.h"

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
			element.perCameraBindingIdx = -1;
			element.gridParamsBindingIdx = -1;

			LOGWRN("Missing shader on material.");
			return;
		}

		SPtr<GpuParams> gpuParams = element.params->getGpuParams();

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the provided
		// buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		if(shader->hasParamBlock("PerFrame"))
			element.params->setParamBlockBuffer("PerFrame", mPerFrameParamBuffer, true);

		if(shader->hasParamBlock("PerObject"))
			element.params->setParamBlockBuffer("PerObject", owner.perObjectParamBuffer, true);

		if(shader->hasParamBlock("PerCall"))
			element.params->setParamBlockBuffer("PerCall", owner.perCallParamBuffer, true);

		if(shader->hasParamBlock("PerCamera"))
			element.perCameraBindingIdx = element.params->getParamBlockBufferIndex("PerCamera");

		element.gridParamsBindingIdx = element.params->getParamBlockBufferIndex("GridParams");

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLights"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLights", element.lightsBufferParam);

		if(gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize", element.gridLightOffsetsAndSizeParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLightIndices"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLightIndices", element.gridLightIndicesParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize", element.gridProbeOffsetsAndSizeParam);

		element.imageBasedParams.populate(element.params, GPT_FRAGMENT_PROGRAM, true, true);

		if (gpuParams->hasBuffer(GPT_VERTEX_PROGRAM, "boneMatrices"))
			gpuParams->setBuffer(GPT_VERTEX_PROGRAM, "boneMatrices", element.boneMatrixBuffer);
	}

	void ObjectRenderer::setParamFrameParams(float time)
	{
		gPerFrameParamDef.gTime.set(mPerFrameParamBuffer, time);
	}

	void DefaultMaterial::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}
}}