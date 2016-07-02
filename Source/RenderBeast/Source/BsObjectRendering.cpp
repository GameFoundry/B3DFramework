//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsObjectRendering.h"
#include "BsShader.h"
#include "BsGpuParams.h"
#include "BsRenderBeast.h"
#include "BsMaterial.h"

namespace BansheeEngine
{
	ObjectRenderer::ObjectRenderer()
	{ }

	void ObjectRenderer::initElement(RenderableElement& element)
	{
		SPtr<ShaderCore> shader = element.material->getShader();
		if (shader == nullptr)
		{
			LOGWRN("Missing shader on material.");
			return;
		}

		const Map<String, SHADER_PARAM_BLOCK_DESC>& paramBlockDescs = shader->getParamBlocks();
		String perFrameBlockName;
		String perCameraBlockName;
		String perObjectBlockName;

		for (auto& paramBlockDesc : paramBlockDescs)
		{
			if (paramBlockDesc.second.rendererSemantic == RBS_PerFrame)
				perFrameBlockName = paramBlockDesc.second.name;
			else if (paramBlockDesc.second.rendererSemantic == RBS_PerCamera)
				perCameraBlockName = paramBlockDesc.second.name;
			else if (paramBlockDesc.second.rendererSemantic == RBS_PerObject)
				perObjectBlockName = paramBlockDesc.second.name;
		}

		UINT32 numPasses = element.material->getNumPasses();
		for (UINT32 i = 0; i < numPasses; i++)
		{
			SPtr<PassParametersCore> passParams = element.material->getPassParameters(i);

			for (UINT32 j = 0; j < PassParametersCore::NUM_PARAMS; j++)
			{
				SPtr<GpuParamsCore> gpuParams = passParams->getParamByIdx(j);
				if (gpuParams == nullptr)
					continue;

				const GpuParamDesc& paramsDesc = gpuParams->getParamDesc();

				// Note: We only validate buffer size and not buffer contents. We should check the contents as well, but
				// likely on a higher level rather than here.
				
				if (perFrameBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(perFrameBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						if (findIter->second.blockSize == mPerFrameParams.getDesc().blockSize)
						{
							SPtr<GpuParamsCore> params = element.material->getPassParameters(i)->getParamByIdx(j);

							UINT32 slotIdx = findIter->second.slot;
							params->setParamBlockBuffer(slotIdx, mPerFrameParams.getBuffer());
						}
					}
				}

				if (perCameraBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(perCameraBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						if (findIter->second.blockSize == mPerCameraParams.getDesc().blockSize)
						{
							SPtr<GpuParamsCore> params = element.material->getPassParameters(i)->getParamByIdx(j);

							UINT32 slotIdx = findIter->second.slot;
							params->setParamBlockBuffer(slotIdx, mPerCameraParams.getBuffer());
						}
					}
				}

				if (perObjectBlockName != "")
				{
					auto findIter = paramsDesc.paramBlocks.find(perObjectBlockName);
					if (findIter != paramsDesc.paramBlocks.end())
					{
						if (findIter->second.blockSize == mPerObjectParams.getDesc().blockSize)
						{
							SPtr<GpuParamsCore> params = element.material->getPassParameters(i)->getParamByIdx(j);

							UINT32 slotIdx = findIter->second.slot;
							params->setParamBlockBuffer(slotIdx, mPerObjectParams.getBuffer());
						}
					}
				}
			}
		}
	}

	void ObjectRenderer::updatePerFrameBuffers(float time)
	{
		mPerFrameParams.gTime.set(time);
	}

	void ObjectRenderer::updatePerCameraBuffers(const CameraShaderData& cameraData)
	{
		mPerCameraParams.gViewDir.set(cameraData.viewDir);
		mPerCameraParams.gViewOrigin.set(cameraData.viewOrigin);
		mPerCameraParams.gMatView.set(cameraData.view);
		mPerCameraParams.gMatProj.set(cameraData.proj);
		mPerCameraParams.gMatViewProj.set(cameraData.viewProj);
		mPerCameraParams.gMatInvProj.set(cameraData.invProj);
		mPerCameraParams.gMatInvViewProj.set(cameraData.invViewProj);
		mPerCameraParams.gMatScreenToWorld.set(cameraData.screenToWorld);
		mPerCameraParams.gDeviceZToWorldZ.set(cameraData.deviceZToWorldZ);
		mPerCameraParams.gClipToUVScaleOffset.set(cameraData.clipToUVScaleOffset);

		mPerCameraParams.flushToGPU();
	}

	void ObjectRenderer::updatePerObjectBuffers(RenderableElement& element, const RenderableShaderData& data, 
		const Matrix4& wvpMatrix)
	{
		// Note: If I kept all the values in the same structure maybe a simple memcpy directly into the constant buffer
		// would be better (i.e. faster)?
		mPerObjectParams.gMatWorld.set(data.worldTransform);
		mPerObjectParams.gMatInvWorld.set(data.invWorldTransform);
		mPerObjectParams.gMatWorldNoScale.set(data.worldNoScaleTransform);
		mPerObjectParams.gMatInvWorldNoScale.set(data.invWorldNoScaleTransform);
		mPerObjectParams.gWorldDeterminantSign.set(data.worldDeterminantSign);
		mPerObjectParams.gMatWorldViewProj.set(wvpMatrix);
	}

	void DefaultMaterial::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}
}