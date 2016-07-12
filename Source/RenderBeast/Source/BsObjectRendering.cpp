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

namespace BansheeEngine
{
	ObjectRenderer::ObjectRenderer()
	{ }

	void ObjectRenderer::initElement(BeastRenderableElement& element)
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

		const Map<String, SHADER_OBJECT_PARAM_DESC>& bufferDescs = shader->getBufferParams();
		String boneMatricesParamName;

		for(auto& entry : bufferDescs)
		{
			if (entry.second.rendererSemantic == RPS_BoneMatrices)
				boneMatricesParamName = entry.second.name;
		}
		
		// Note: Perhaps perform buffer validation to ensure expected buffer has the same size and layout as the provided
		// buffer, and show a warning otherwise. But this is perhaps better handled on a higher level.
		element.material->setParamBlockBuffer(perFrameBlockName, mPerFrameParams.getBuffer());
		element.material->setParamBlockBuffer(perCameraBlockName, mPerCameraParams.getBuffer());
		element.material->setParamBlockBuffer(perObjectBlockName, mPerObjectParams.getBuffer());

		if (!boneMatricesParamName.empty())
		{
			// Note: Bone matrices should be shared between all sub-meshes, so maybe it's better to create this buffer
			// on a per-Renderable basis, rather than per-element?
			element.boneMatricesParam = element.material->getParamBuffer(boneMatricesParamName);

			SPtr<Skeleton> skeleton = element.mesh->getSkeleton();
			UINT32 numBones = skeleton != nullptr ? skeleton->getNumBones() : 0;

			if (numBones > 0)
			{
				SPtr<GpuBufferCore> buffer = GpuBufferCore::create(numBones * 3, 0, GBT_STANDARD, BF_32X4F, GBU_DYNAMIC);
				UINT8* dest = (UINT8*)buffer->lock(0, numBones * 3 * sizeof(Vector4), GBL_WRITE_ONLY_DISCARD);

				// Initialize bone transforms to identity, so the object renders properly even if no animation is animating it
				for (UINT32 i = 0; i < numBones; i++)
				{
					memcpy(dest, &Matrix4::IDENTITY, 12 * sizeof(float)); // Assuming row-major format

					dest += 12 * sizeof(float);
				}

				buffer->unlock();

				element.boneMatrixBuffer = buffer;
			}
		}
	}

	void ObjectRenderer::setParamFrameParams(float time)
	{
		mPerFrameParams.gTime.set(time);
	}

	void ObjectRenderer::setPerCameraParams(const CameraShaderData& cameraData)
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

	void ObjectRenderer::setPerObjectParams(const BeastRenderableElement& element, const RenderableShaderData& data,
		const Matrix4& wvpMatrix, const SPtr<GpuBufferCore>& boneMatrices)
	{
		// Note: If I kept all the values in the same structure maybe a simple memcpy directly into the constant buffer
		// would be better (i.e. faster)?
		mPerObjectParams.gMatWorld.set(data.worldTransform);
		mPerObjectParams.gMatInvWorld.set(data.invWorldTransform);
		mPerObjectParams.gMatWorldNoScale.set(data.worldNoScaleTransform);
		mPerObjectParams.gMatInvWorldNoScale.set(data.invWorldNoScaleTransform);
		mPerObjectParams.gWorldDeterminantSign.set(data.worldDeterminantSign);
		mPerObjectParams.gMatWorldViewProj.set(wvpMatrix);

		element.boneMatricesParam.set(boneMatrices);
	}

	void DefaultMaterial::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}
}