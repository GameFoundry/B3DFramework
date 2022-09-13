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
		mSampleCount = mVariation.GetUInt("MSAA_COUNT");

		mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);

		if (mParams->HasLoadStoreTexture(GPT_COMPUTE_PROGRAM, "gOutput"))
			mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		if (mSampleCount > 1)
			mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

		mParamBuffer = gTiledLightingParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Params", mParamBuffer);
	}

	void TiledDeferredLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredLightingMat::execute(const RendererView& view, const VisibleLightData& lightData,
		const GBufferTextures& gbuffer, const SPtr<Texture>& inputTexture, const SPtr<Texture>& lightAccumTex,
		const SPtr<Texture>& lightAccumTexArray, const SPtr<Texture>& msaaCoverage)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.GetProperties();
		const RenderSettings& settings = view.GetRenderSettings();

		mLightBufferParam.Set(lightData.getLightBuffer());

		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledLightingParamDef.gFramebufferSize.Set(mParamBuffer, framebufferSize);

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

			gTiledLightingParamDef.gLightCounts.Set(mParamBuffer, lightCounts);
			gTiledLightingParamDef.gLightStrides.Set(mParamBuffer, lightStrides);
		}
		else
		{
			Vector4I unshadowedLightCounts;
			unshadowedLightCounts[0] = lightData.GetNumUnshadowedLights(LightType::Directional);
			unshadowedLightCounts[1] = lightData.GetNumUnshadowedLights(LightType::Radial);
			unshadowedLightCounts[2] = lightData.GetNumUnshadowedLights(LightType::Spot);
			unshadowedLightCounts[3] = unshadowedLightCounts[0] + unshadowedLightCounts[1] + unshadowedLightCounts[2];

			Vector4I lightCounts;
			lightCounts[0] = lightData.GetNumLights(LightType::Directional);
			lightCounts[1] = lightData.GetNumLights(LightType::Radial);
			lightCounts[2] = lightData.GetNumLights(LightType::Spot);
			lightCounts[3] = lightCounts[0] + lightCounts[1] + lightCounts[2];

			Vector2I lightStrides;
			lightStrides[0] = lightCounts[0];
			lightStrides[1] = lightStrides[0] + lightCounts[1];

			if(!settings.enableShadows)
				gTiledLightingParamDef.gLightCounts.Set(mParamBuffer, lightCounts);
			else
				gTiledLightingParamDef.gLightCounts.Set(mParamBuffer, unshadowedLightCounts);

			gTiledLightingParamDef.gLightStrides.Set(mParamBuffer, lightStrides);
		}

		mParamBuffer->FlushToGPU();

		mGBufferParams.Bind(gbuffer);
		mParams->SetParamBlockBuffer("PerCamera", view.GetPerViewBuffer());
		mInColorTextureParam.Set(inputTexture);

		if (mSampleCount > 1)
		{
			mOutputTextureParam.Set(lightAccumTexArray, TextureSurface::COMPLETE);
			mMSAACoverageTexParam.Set(msaaCoverage);
		}
		else
			mOutputTextureParam.Set(lightAccumTex);

		UINT32 numTilesX = (UINT32)Math::CeilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::CeilToInt(height / (float)TILE_SIZE);

		bind();
		RenderAPI::instance().DispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredLightingMat* TiledDeferredLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return Get(getVariation<1>());
		case 2:
			return Get(getVariation<2>());
		case 4:
			return Get(getVariation<4>());
		case 8:
		default:
			return Get(getVariation<8>());
		}
	}

	TextureArrayToMSAATexture::TextureArrayToMSAATexture()
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInput", mInputParam);
	}

	void TextureArrayToMSAATexture::Execute(const SPtr<Texture>& inputArray, const SPtr<Texture>& target)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& inputProps = inputArray->GetProperties();
		const TextureProperties& targetProps = target->GetProperties();

		assert(inputProps.GetNumArraySlices() == targetProps.getNumSamples());
		assert(inputProps.GetWidth() == targetProps.getWidth());
		assert(inputProps.GetHeight() == targetProps.getHeight());

		mInputParam.Set(inputArray);

		bind();

		Rect2 Area(0.0f, 0.0f, (float)targetProps.GetWidth(), (float)targetProps.getHeight());
		gRendererUtility().DrawScreenQuad(area);
	}

	ClearLoadStoreParamDef gClearLoadStoreParamDef;

	ClearLoadStoreMat::ClearLoadStoreMat()
	{
		INT32 objType = mVariation.GetInt("OBJ_TYPE");

		if(objType == 0 || objType == 1)
			mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);
		else
			mParams->GetBufferParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);

		mParamBuffer = gClearLoadStoreParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer(GPT_COMPUTE_PROGRAM, "Params", mParamBuffer);
	}

	void ClearLoadStoreMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILE_SIZE", TILE_SIZE);
		defines.Set("NUM_THREADS", NUM_THREADS);
	}

	void ClearLoadStoreMat::execute(const SPtr<Texture>& target, const Color& clearValue,
			const TextureSurface& surface)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& props = target->GetProperties();
		PixelFormat pf = props.GetFormat();

		assert(!PixelUtil::isCompressed(pf));

		mOutputTextureParam.Set(target, surface);

		UINT32 width = props.GetWidth();
		UINT32 height = props.GetHeight();
		gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((INT32)width, (INT32)height));
		gClearLoadStoreParamDef.gFloatClearVal.set(mParamBuffer,
			Vector4(clearValue.r, clearValue.g, clearValue.a, clearValue.a));
		gClearLoadStoreParamDef.gIntClearVal.set(mParamBuffer,
			Vector4I(*(INT32*)&clearValue.r, *(INT32*)&clearValue.g, *(INT32*)&clearValue.a, *(INT32*)&clearValue.a));

		bind();

		UINT32 numGroupsX = Math::DivideAndRoundUp(width, NUM_THREADS * TILE_SIZE);
		UINT32 numGroupsY = Math::DivideAndRoundUp(height, NUM_THREADS * TILE_SIZE);
		
		RenderAPI::instance().DispatchCompute(numGroupsX, numGroupsY);
	}

	void ClearLoadStoreMat::Execute(const SPtr<GpuBuffer>& target, const Color& clearValue)
	{
		BS_RENMAT_PROFILE_BLOCK

		mOutputBufferParam.Set(target);

		UINT32 width = target->GetProperties().GetElementCount();
		UINT32 height = 1;
		gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((INT32)width, (INT32)height));
		gClearLoadStoreParamDef.gFloatClearVal.set(mParamBuffer,
			Vector4(clearValue.r, clearValue.g, clearValue.a, clearValue.a));
		gClearLoadStoreParamDef.gIntClearVal.set(mParamBuffer,
			Vector4I(*(INT32*)&clearValue.r, *(INT32*)&clearValue.g, *(INT32*)&clearValue.a, *(INT32*)&clearValue.a));

		bind();

		UINT32 numGroupsX = Math::DivideAndRoundUp(width, NUM_THREADS * (TILE_SIZE * TILE_SIZE));
		RenderAPI::instance().DispatchCompute(numGroupsX, 1);
	}

	/** Helper method used for initializing variations of the ClearLoadStore material. */
	template<ClearLoadStoreType OBJ_TYPE, ClearLoadStoreDataType DATA_TYPE, UINT32 NUM_COMPONENTS>
	static const ShaderVariation& GetClearLoadStoreVariation()
	{
		static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("OBJ_TYPE", (int)OBJ_TYPE),
				ShaderVariation::Param("DATA_TYPE", (int)DATA_TYPE),
				ShaderVariation::Param("NUM_COMPONENTS", NUM_COMPONENTS),

			});

		return variation;
	}

	template<ClearLoadStoreType BUFFER_TYPE, ClearLoadStoreDataType DATA_TYPE>
	const ShaderVariation& GetClearLoadStoreVariation(UINT32 numComponents)
	{
		switch (numComponents)
		{
		default:
		case 1:
			return getClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 0>();
		case 2:
			return getClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 1>();
		case 3:
			return getClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 2>();
		case 4:
			return getClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 3>();
		}
	}

	ClearLoadStoreMat* ClearLoadStoreMat::getVariation(ClearLoadStoreType objType, ClearLoadStoreDataType dataType,
									UINT32 numComponents)
	{
		switch(objType)
		{
		default:
		case ClearLoadStoreType::Texture:
			if(dataType == ClearLoadStoreDataType::Float)
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::Texture, ClearLoadStoreDataType::Float>(numComponents));
			else
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::Texture, ClearLoadStoreDataType::Int>(numComponents));
		case ClearLoadStoreType::TextureArray:
			if(dataType == ClearLoadStoreDataType::Float)
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float>(numComponents));
			else
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Int>(numComponents));
		case ClearLoadStoreType::Buffer:
			if(dataType == ClearLoadStoreDataType::Float)
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Float>(numComponents));
			else
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Int>(numComponents));
		case ClearLoadStoreType::StructuredBuffer:
			if(dataType == ClearLoadStoreDataType::Float)
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Float>(numComponents));
			else
				return Get(getClearLoadStoreVariation<ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int>(numComponents));
		}
	}

	TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

	// Note: Tile size was reduced from 32 to 16 because of macOS limitations. Ideally we should try keeping the larger
	// size on non-macOS platforms, but currently where don't have a platform-specific way of setting this.
	//
	// The theory is that using larger tiles will amortize the cost of computing tile AABB's (which this shader uses,
	// compared to the cheaper-to-compute frustums).
	const UINT32 TiledDeferredImageBasedLightingMat::TILE_SIZE = 16;

	TiledDeferredImageBasedLightingMat::TiledDeferredImageBasedLightingMat()
	{
		mSampleCount = mVariation.GetUInt("MSAA_COUNT");

		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

		mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
		mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

		if (mSampleCount > 1)
			mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

		mParamBuffer = gTiledImageBasedLightingParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Params", mParamBuffer);

		mImageBasedParams.Populate(mParams, GPT_COMPUTE_PROGRAM, false, false, true);

		mParams->SetParamBlockBuffer("ReflProbeParams", mReflProbeParamBuffer.buffer);
	}

	void TiledDeferredImageBasedLightingMat::_initDefines(ShaderDefines& defines)
	{
		defines.Set("TILE_SIZE", TILE_SIZE);
	}

	void TiledDeferredImageBasedLightingMat::execute(const RendererView& view, const SceneInfo& sceneInfo,
		const VisibleReflProbeData& probeData, const Inputs& inputs)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.GetProperties();
		UINT32 width = viewProps.target.viewRect.width;
		UINT32 height = viewProps.target.viewRect.height;

		Vector2I framebufferSize;
		framebufferSize[0] = width;
		framebufferSize[1] = height;
		gTiledImageBasedLightingParamDef.gFramebufferSize.Set(mParamBuffer, framebufferSize);

		Skybox* skybox = nullptr;
		if(view.GetRenderSettings().enableSkybox)
			skybox = sceneInfo.skybox;

		mReflProbeParamBuffer.Populate(skybox, probeData.getNumProbes(), sceneInfo.reflProbeCubemapsTex,
			viewProps.capturingReflections);

		mParamBuffer->FlushToGPU();
		mReflProbeParamBuffer.buffer->FlushToGPU();

		mGBufferA.Set(inputs.gbuffer.albedo);
		mGBufferB.Set(inputs.gbuffer.normals);
		mGBufferC.Set(inputs.gbuffer.roughMetal);
		mGBufferDepth.Set(inputs.gbuffer.depth);

		SPtr<Texture> skyFilteredRadiance;
		if(skybox)
			skyFilteredRadiance = skybox->GetFilteredRadiance();

		mImageBasedParams.preintegratedEnvBRDFParam.Set(inputs.preIntegratedGF);
		mImageBasedParams.reflectionProbesParam.Set(probeData.getProbeBuffer());
		mImageBasedParams.reflectionProbeCubemapsTexParam.Set(sceneInfo.reflProbeCubemapsTex);
		mImageBasedParams.skyReflectionsTexParam.Set(skyFilteredRadiance);
		mImageBasedParams.ambientOcclusionTexParam.Set(inputs.ambientOcclusion);
		mImageBasedParams.ssrTexParam.Set(inputs.ssr);

		mParams->SetParamBlockBuffer("PerCamera", view.GetPerViewBuffer());

		mInColorTextureParam.Set(inputs.lightAccumulation);
		if (mSampleCount > 1)
		{
			mOutputTextureParam.Set(inputs.sceneColorTexArray, TextureSurface::COMPLETE);
			mMSAACoverageTexParam.Set(inputs.msaaCoverage);
		}
		else
			mOutputTextureParam.Set(inputs.sceneColorTex);

		UINT32 numTilesX = (UINT32)Math::CeilToInt(width / (float)TILE_SIZE);
		UINT32 numTilesY = (UINT32)Math::CeilToInt(height / (float)TILE_SIZE);

		bind();
		RenderAPI::instance().DispatchCompute(numTilesX, numTilesY);
	}

	TiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 1:
			return Get(getVariation<1>());
		case 2:
			return Get(getVariation<2>());
		case 4:
			return Get(getVariation<4>());
		case 8:
		default:
			return Get(getVariation<8>());
		}
	}
}}
