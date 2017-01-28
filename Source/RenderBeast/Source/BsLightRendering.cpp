//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightRendering.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsRenderBeast.h"
#include "BsRenderTargets.h"
#include "BsGpuParams.h"
#include "BsGpuParamsSet.h"
#include "BsGpuBuffer.h"
#include "BsLight.h"
#include "BsRendererUtility.h"

namespace bs { namespace ct
{
	static const UINT32 BUFFER_INCREMENT = 16 * sizeof(LightData);

	TiledLightingParamDef gTiledLightingParamDef;

	RendererLight::RendererLight(Light* light)
		:mInternal(light)
	{ }

	void RendererLight::getParameters(LightData& output) const
	{
		Radian spotAngle = Math::clamp(mInternal->getSpotAngle() * 0.5f, Degree(0), Degree(89));
		Radian spotFalloffAngle = Math::clamp(mInternal->getSpotFalloffAngle() * 0.5f, Degree(0), (Degree)spotAngle);
		Color color = mInternal->getColor();

		output.position = mInternal->getPosition();
		output.radius = mInternal->getBounds().getRadius();
		output.direction = mInternal->getRotation().zAxis();
		output.intensity = mInternal->getIntensity();
		output.spotAngles.x = spotAngle.valueRadians();
		output.spotAngles.y = Math::cos(output.spotAngles.x);
		output.spotAngles.z = 1.0f / (Math::cos(spotFalloffAngle) - output.spotAngles.y);
		output.radiusSqrdInv = 1.0f / (output.radius * output.radius);
		output.color = Vector3(color.r, color.g, color.b);
	}

	const UINT32 TiledDeferredLightingMat::TILE_SIZE = 16;

	TiledDeferredLightingMat::TiledDeferredLightingMat()
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		auto& texParams = mMaterial->getShader()->getTextureParams();
		for (auto& entry : texParams)
		{
			if (entry.second.rendererSemantic == RPS_GBufferA)
				params->getTextureParam(GPT_COMPUTE_PROGRAM, entry.second.name, mGBufferA);
			else if (entry.second.rendererSemantic == RPS_GBufferB)
				params->getTextureParam(GPT_COMPUTE_PROGRAM, entry.second.name, mGBufferB);
			else if (entry.second.rendererSemantic == RPS_GBufferDepth)
				params->getTextureParam(GPT_COMPUTE_PROGRAM, entry.second.name, mGBufferDepth);
		}

		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gDirLights", mDirLightBufferParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gPointLights", mPointLightBufferParam);
		params->getBufferParam(GPT_COMPUTE_PROGRAM, "gSpotLights", mSpotLightBufferParam);

		params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputParam);

		mParamBuffer = gTiledLightingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);
	}

	void TiledDeferredLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredLightingMat::execute(const SPtr<RenderTargets>& gbuffer, const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		mGBufferA.set(gbuffer->getTextureA());
		mGBufferB.set(gbuffer->getTextureB());
		mGBufferDepth.set(gbuffer->getTextureDepth());

		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);

		SPtr<Texture> sceneColorTex = gbuffer->getSceneColor();
		mOutputParam.set(sceneColorTex);

		UINT32 width = sceneColorTex->getProperties().getWidth();
		UINT32 height = sceneColorTex->getProperties().getHeight();

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		gRendererUtility().setComputePass(mMaterial, 0);
		gRendererUtility().setPassParams(mParamsSet);

		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	void TiledDeferredLightingMat::setLights(const Vector<LightData>(&lightData)[3])
	{
		Vector3I numLightsPerType;
		for (UINT32 i = 0; i < 3; i++)
		{
			UINT32 numLights = (UINT32)lightData[i].size();
			numLightsPerType[i] = numLights;

			UINT32 size = numLights * sizeof(LightData);
			UINT32 curBufferSize;

			if (mLightBuffers[i] != nullptr)
				curBufferSize = mLightBuffers[i]->getSize();
			else
				curBufferSize = 0;

			if(size > curBufferSize || curBufferSize == 0)
			{
				// Allocate at least one block even if no lights, to avoid issues with null buffers
				UINT32 bufferSize = std::max(1, Math::ceilToInt(size / (float)BUFFER_INCREMENT)) * BUFFER_INCREMENT;

				GPU_BUFFER_DESC bufferDesc;
				bufferDesc.type = GBT_STRUCTURED;
				bufferDesc.elementCount = bufferSize / sizeof(LightData);
				bufferDesc.elementSize = sizeof(LightData);
				bufferDesc.format = BF_UNKNOWN;

				mLightBuffers[i] = GpuBuffer::create(bufferDesc);
			}

			mLightBuffers[i]->writeData(0, size, lightData[i].data(), BWT_DISCARD);
		}

		mDirLightBufferParam.set(mLightBuffers[0]);
		mPointLightBufferParam.set(mLightBuffers[1]);
		mSpotLightBufferParam.set(mLightBuffers[2]);

		gTiledLightingParamDef.gNumLightsPerType.set(mParamBuffer, numLightsPerType);

		mParamBuffer->flushToGPU();
	}
}}