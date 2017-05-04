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

		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the provided
		// buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		const Map<String, SHADER_PARAM_BLOCK_DESC>& paramBlockDescs = shader->getParamBlocks();

		for (auto& paramBlockDesc : paramBlockDescs)
		{
			if (paramBlockDesc.second.rendererSemantic == RBS_PerFrame)
				element.params->setParamBlockBuffer(paramBlockDesc.second.name, mPerFrameParamBuffer, true);
			else if (paramBlockDesc.second.rendererSemantic == RBS_PerObject)
			{
				element.params->setParamBlockBuffer(paramBlockDesc.second.name,
													owner.perObjectParamBuffer, true);
			}
			else if (paramBlockDesc.second.rendererSemantic == RBS_PerCall)
			{
				element.params->setParamBlockBuffer(paramBlockDesc.second.name,
													owner.perCallParamBuffer, true);
			}
			else if(paramBlockDesc.second.rendererSemantic == RBS_PerCamera)
			{
				element.perCameraBindingIdx = element.params->getParamBlockBufferIndex(paramBlockDesc.second.name);
			}
		}

		element.gridParamsBindingIdx = element.params->getParamBlockBufferIndex("GridParams");

		SPtr<GpuParams> gpuParams = element.params->getGpuParams();
		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLights"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLights", element.lightsBufferParam);

		if(gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridLightOffsetsAndSize", element.gridLightOffsetsAndSizeParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gLightIndices"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gLightIndices", element.gridLightIndicesParam);

		if (gpuParams->hasBuffer(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize"))
			gpuParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gGridProbeOffsetsAndSize", element.gridProbeOffsetsAndSizeParam);

		element.imageBasedParams.populate(element.params, GPT_FRAGMENT_PROGRAM, true, true);

		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferDescs = shader->getBufferParams();
		String boneMatricesParamName;

		for(auto& entry : bufferDescs)
		{
			if (entry.second.rendererSemantic == RPS_BoneMatrices)
				boneMatricesParamName = entry.second.name;
		}
		
		if (!boneMatricesParamName.empty())
		{
			MaterialParamBuffer boneMatricesParam = element.material->getParamBuffer(boneMatricesParamName);
			boneMatricesParam.set(element.boneMatrixBuffer);
		}
	}

	void ObjectRenderer::setParamFrameParams(float time)
	{
		gPerFrameParamDef.gTime.set(mPerFrameParamBuffer, time);
	}

	void DefaultMaterial::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}
}}