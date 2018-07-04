//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsTiledDeferred.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"
#include "BsRenderBeast.h"

namespace bs { namespace ct
{
	TiledLightingParamDef gTiledLightingParamDef;

	const UINT32 TiledDeferredLightingMat::TILE_SIZE = 16;

	TiledDeferredLightingMat::TiledDeferredLightingMat()
		:mGBufferParams(GPT_COMPUTE_PROGRAM, mParams)
	{
		mSampleCount = mVariation.getUInt("MSAA_COUNT");

		mParams->getBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);

		if (mParams->hasLoadStoreTexture(GPT_COMPUTE_PROGRAM, "gOutput"))
			mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		if (mSampleCount > 1)
			mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

		mParamBuffer = gTiledLightingParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);
	}

	void TiledDeferredLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredLightingMat::execute(const RendererView& view, const VisibleLightData& lightData, 
		const GBufferTextures& gbuffer, const SPtr<Texture>& lightAccumTex, const SPtr<Texture>& lightAccumTexArray, 
		const SPtr<Texture>& msaaCoverage)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.getProperties();
		const RenderSettings& settings = view.getRenderSettings();

		mLightBufferParam.set(lightData.getLightBuffer());

		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		if (!settings.enableLighting)
		{
			Vector4I lightCounts;
			lightCounts[0] = 0;
			lightCounts[1] = 0;
			lightCounts[2] = 0;
			lightCounts[3] = 0;

			Vector2I lightStrides;
			lightStrides[0] = 0;
			lightStrides[1] = 0;

			gTiledLightingParamDef.gLightCounts.set(mParamBuffer, lightCounts);
			gTiledLightingParamDef.gLightStrides.set(mParamBuffer, lightStrides);
		}
		else
		{
			Vector4I unshadowedLightCounts;
			unshadowedLightCounts[0] = lightData.getNumUnshadowedLights(LightType::Directional);
			unshadowedLightCounts[1] = lightData.getNumUnshadowedLights(LightType::Radial);
			unshadowedLightCounts[2] = lightData.getNumUnshadowedLights(LightType::Spot);
			unshadowedLightCounts[3] = unshadowedLightCounts[0] + unshadowedLightCounts[1] + unshadowedLightCounts[2];

			Vector4I lightCounts;
			lightCounts[0] = lightData.getNumLights(LightType::Directional);
			lightCounts[1] = lightData.getNumLights(LightType::Radial);
			lightCounts[2] = lightData.getNumLights(LightType::Spot);
			lightCounts[3] = lightCounts[0] + lightCounts[1] + lightCounts[2];

			Vector2I lightStrides;
			lightStrides[0] = lightCounts[0];
			lightStrides[1] = lightStrides[0] + lightCounts[1];

			if(!settings.enableShadows)
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, lightCounts);
			else
				gTiledLightingParamDef.gLightCounts.set(mParamBuffer, unshadowedLightCounts);

			gTiledLightingParamDef.gLightStrides.set(mParamBuffer, lightStrides);
		}

		mParamBuffer->flushToGPU();

		mGBufferParams.bind(gbuffer);
		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		if (mSampleCount > 1)
		{
			mOutputTextureParam.set(lightAccumTexArray, TextureSurface::COMPLETE);
			mMSAACoverageTexParam.set(msaaCoverage);
		}
		else
			mOutputTextureParam.set(lightAccumTex);

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		bind();
		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredLightingMat* TiledDeferredLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return get(getVariation<1>());
		case 2:
			return get(getVariation<2>());
		case 4:
			return get(getVariation<4>());
		case 8:
		default:
			return get(getVariation<8>());
		}
	}

	TextureArrayToMSAATexture::TextureArrayToMSAATexture()
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInput", mInputParam);
	}

	void TextureArrayToMSAATexture::execute(const SPtr<Texture>& inputArray, const SPtr<Texture>& target)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& inputProps = inputArray->getProperties();
		const TextureProperties& targetProps = target->getProperties();

		assert(inputProps.getNumArraySlices() == targetProps.getNumSamples());
		assert(inputProps.getWidth() == targetProps.getWidth());
		assert(inputProps.getHeight() == targetProps.getHeight());

		mInputParam.set(inputArray);

		bind();

		Rect2 area(0.0f, 0.0f, (float)targetProps.getWidth(), (float)targetProps.getHeight());
		gRendererUtility().drawScreenQuad(area);
	}

	ClearLoadStoreParamDef gClearLoadStoreParamDef;

	ClearLoadStore::ClearLoadStore()
	{
		mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputParam);

		mParamBuffer = gClearLoadStoreParamDef.createBuffer();
		mParams->setParamBlockBuffer(GPT_COMPUTE_PROGRAM, "Params", mParamBuffer);
	}

	void ClearLoadStore::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TILE_SIZE);
		defines.set("NUM_THREADS", NUM_THREADS);
	}

	void ClearLoadStore::execute(const SPtr<Texture>& target, const TextureSurface& surface)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& props = target->getProperties();
		PixelFormat pf = props.getFormat();

		assert(!PixelUtil::isCompressed(pf));

		UINT32 numBytes = PixelUtil::getNumElemBytes(pf);
		assert(numBytes % 4 == 0);

		mOutputParam.set(target, surface);

		bind();

		UINT32 numElements = numBytes / 4;
		UINT32 width = props.getWidth() * numElements;
		UINT32 height = props.getHeight() * numElements;

		UINT32 numGroupsX = Math::divideAndRoundUp(width, NUM_THREADS * TILE_SIZE);
		UINT32 numGroupsY = Math::divideAndRoundUp(height, NUM_THREADS * TILE_SIZE);
		
		RenderAPI::instance().dispatchCompute(numGroupsX, numGroupsY);
	}

	TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

	// Note: Using larger tiles than in tiled deferred lighting since we use AABB for intersections, which is more
	// expensive to compute than frustums. This way we amortize the cost even though other parts of the shader might suffer
	// due to increased thread group load.
	const UINT32 TiledDeferredImageBasedLightingMat::TILE_SIZE = 32;

	TiledDeferredImageBasedLightingMat::TiledDeferredImageBasedLightingMat()
	{
		mSampleCount = mVariation.getUInt("MSAA_COUNT");

		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		if (mSampleCount > 1)
			mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);

		mImageBasedParams.populate(mParams, GPT_COMPUTE_PROGRAM, false, false, true);

		mParams->setParamBlockBuffer("ReflProbeParams", mReflProbeParamBuffer.buffer);
	}

	void TiledDeferredImageBasedLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredImageBasedLightingMat::execute(const RendererView& view, const SceneInfo& sceneInfo, 
		const VisibleReflProbeData& probeData, const Inputs& inputs)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.getProperties();
		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledImageBasedLightingParamDef.gFramebufferSize.set(mParamBuffer, framebufferSize);

		Skybox* skybox = nullptr; 
		if(view.getRenderSettings().enableSkybox)
			skybox = sceneInfo.skybox;

		mReflProbeParamBuffer.populate(skybox, probeData.getNumProbes(), sceneInfo.reflProbeCubemapsTex, 
			viewProps.capturingReflections);

		mParamBuffer->flushToGPU();
		mReflProbeParamBuffer.buffer->flushToGPU();

		mGBufferA.set(inputs.gbuffer.albedo);
		mGBufferB.set(inputs.gbuffer.normals);
		mGBufferC.set(inputs.gbuffer.roughMetal);
		mGBufferDepth.set(inputs.gbuffer.depth);

		SPtr<Texture> skyFilteredRadiance;
		if(skybox)
			skyFilteredRadiance = skybox->getFilteredRadiance();

		mImageBasedParams.preintegratedEnvBRDFParam.set(inputs.preIntegratedGF);
		mImageBasedParams.reflectionProbesParam.set(probeData.getProbeBuffer());
		mImageBasedParams.reflectionProbeCubemapsTexParam.set(sceneInfo.reflProbeCubemapsTex);
		mImageBasedParams.skyReflectionsTexParam.set(skyFilteredRadiance);
		mImageBasedParams.ambientOcclusionTexParam.set(inputs.ambientOcclusion);
		mImageBasedParams.ssrTexParam.set(inputs.ssr);

		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		mInColorTextureParam.set(inputs.lightAccumulation);
		if (mSampleCount > 1)
		{
			mOutputTextureParam.set(inputs.sceneColorTexArray, TextureSurface::COMPLETE);
			mMSAACoverageTexParam.set(inputs.msaaCoverage);
		}
		else
			mOutputTextureParam.set(inputs.sceneColorTex);

		UINT32 numTilesX = (UINT32)Math::ceilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::ceilToInt(height / (float)TILE_SIZE);

		bind();
		RenderAPI::instance().dispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return get(getVariation<1>());
		case 2:
			return get(getVariation<2>());
		case 4:
			return get(getVariation<4>());
		case 8:
		default:
			return get(getVariation<8>());
		}
	}
}}
