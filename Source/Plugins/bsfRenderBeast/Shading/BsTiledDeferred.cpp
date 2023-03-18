//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsTiledDeferred.h"
#include "RenderAPI/BsGenericGpuBuffer.h"
#include "Renderer/BsReflectionProbe.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"
#include "BsRenderBeast.h"

namespace bs {
namespace ct {

TiledLightingParamDef gTiledLightingParamDef;

const u32 TiledDeferredLightingMat::kTileSize = 16;

void TiledDeferredLightingMat::Initialize()
{
	mGBufferParams.Initialize(GPT_COMPUTE_PROGRAM, mGPUParameters);
	mSampleCount = mVariationParameters.GetUInt("MSAA_COUNT");

	mGPUParameters->GetStorageBufferParameter(GPT_COMPUTE_PROGRAM, "gLights", mLightBufferParam);
	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);

	if(mGPUParameters->HasStorageTexture(GPT_COMPUTE_PROGRAM, "gOutput"))
		mGPUParameters->GetStorageTextureParameter(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

	mParamBuffer = gTiledLightingParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
}

void TiledDeferredLightingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredLightingMat::Execute(const RendererView& view, const VisibleLightData& lightData, const GBufferTextures& gbuffer, const SPtr<Texture>& inputTexture, const SPtr<Texture>& lightAccumTex, const SPtr<Texture>& lightAccumTexArray, const SPtr<Texture>& msaaCoverage)
{
	BS_RENMAT_PROFILE_BLOCK

	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderSettings& settings = view.GetRenderSettings();

	mLightBufferParam.Set(lightData.GetLightBuffer());

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledLightingParamDef.gFramebufferSize.Set(mParamBuffer, framebufferSize);

	if(!settings.EnableLighting)
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

		if(!settings.EnableShadows)
			gTiledLightingParamDef.gLightCounts.Set(mParamBuffer, lightCounts);
		else
			gTiledLightingParamDef.gLightCounts.Set(mParamBuffer, unshadowedLightCounts);

		gTiledLightingParamDef.gLightStrides.Set(mParamBuffer, lightStrides);
	}

	mParamBuffer->FlushToGpu();

	mGBufferParams.Bind(gbuffer);
	mGPUParameters->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());
	mInColorTextureParam.Set(inputTexture);

	if(mSampleCount > 1)
	{
		mOutputTextureParam.Set(lightAccumTexArray, TextureSurface::kComplete);
		mMSAACoverageTexParam.Set(msaaCoverage);
	}
	else
		mOutputTextureParam.Set(lightAccumTex);

	u32 numTilesX = (u32)Math::CeilToInt(width / (float)kTileSize);
	u32 numTilesY = (u32)Math::CeilToInt(height / (float)kTileSize);

	Bind();
	RenderAPI::Instance().DispatchCompute(numTilesX, numTilesY);
}

TiledDeferredLightingMat* TiledDeferredLightingMat::GetVariation(u32 msaaCount)
{
	switch(msaaCount)
	{
	case 1:
		return Get(GetVariation<1>());
	case 2:
		return Get(GetVariation<2>());
	case 4:
		return Get(GetVariation<4>());
	case 8:
	default:
		return Get(GetVariation<8>());
	}
}

void TextureArrayToMSAATexture::Initialize()
{
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gInput", mInputParam);
}

void TextureArrayToMSAATexture::Execute(const SPtr<Texture>& inputArray, const SPtr<Texture>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& inputProps = inputArray->GetProperties();
	const TextureProperties& targetProps = target->GetProperties();

	B3D_ASSERT(inputProps.ArraySliceCount == targetProps.SampleCount);
	B3D_ASSERT(inputProps.Width == targetProps.Width);
	B3D_ASSERT(inputProps.Height == targetProps.Height);

	mInputParam.Set(inputArray);

	Bind();

	Rect2 area(0.0f, 0.0f, (float)targetProps.Width, (float)targetProps.Height);
	GetRendererUtility().DrawScreenQuad(area);
}

ClearLoadStoreParamDef gClearLoadStoreParamDef;

void ClearLoadStoreMat::Initialize()
{
	i32 objType = mVariationParameters.GetInt("OBJ_TYPE");

	if(objType == 0 || objType == 1)
		mGPUParameters->GetStorageTextureParameter(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);
	else
		mGPUParameters->GetStorageBufferParameter(GPT_COMPUTE_PROGRAM, "gOutput", mOutputBufferParam);

	mParamBuffer = gClearLoadStoreParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer(GPT_COMPUTE_PROGRAM, "Params", mParamBuffer);
}

void ClearLoadStoreMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
	defines.Set("NUM_THREADS", kNumThreads);
}

void ClearLoadStoreMat::Execute(const SPtr<Texture>& target, const Color& clearValue, const TextureSurface& surface)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& props = target->GetProperties();
	PixelFormat pf = props.Format;

	B3D_ASSERT(!PixelUtil::IsCompressed(pf));

	mOutputTextureParam.Set(target, surface);

	u32 width = props.Width;
	u32 height = props.Height;
	gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreParamDef.gFloatClearVal.Set(mParamBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreParamDef.gIntClearVal.Set(mParamBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	Bind();

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * kTileSize);
	u32 numGroupsY = Math::DivideAndRoundUp(height, kNumThreads * kTileSize);

	RenderAPI::Instance().DispatchCompute(numGroupsX, numGroupsY);
}

void ClearLoadStoreMat::Execute(const SPtr<GenericGpuBuffer>& target, const Color& clearValue)
{
	BS_RENMAT_PROFILE_BLOCK

	mOutputBufferParam.Set(target);

	u32 width = target->GetProperties().GetElementCount();
	u32 height = 1;
	gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreParamDef.gFloatClearVal.Set(mParamBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreParamDef.gIntClearVal.Set(mParamBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	Bind();

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * (kTileSize * kTileSize));
	RenderAPI::Instance().DispatchCompute(numGroupsX, 1);
}

/** Helper method used for initializing variations of the ClearLoadStore material. */
template <ClearLoadStoreType OBJ_TYPE, ClearLoadStoreDataType DATA_TYPE, u32 NUM_COMPONENTS>
static const ShaderVariationParameters& GetClearLoadStoreVariation()
{
	static ShaderVariationParameters variation = ShaderVariationParameters(
		{
			ShaderVariationParameter("OBJ_TYPE", (int)OBJ_TYPE),
			ShaderVariationParameter("DATA_TYPE", (int)DATA_TYPE),
			ShaderVariationParameter("NUM_COMPONENTS", NUM_COMPONENTS),

		});

	return variation;
}

template <ClearLoadStoreType BUFFER_TYPE, ClearLoadStoreDataType DATA_TYPE>
const ShaderVariationParameters& GetClearLoadStoreVariation(u32 numComponents)
{
	switch(numComponents)
	{
	default:
	case 1:
		return GetClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 0>();
	case 2:
		return GetClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 1>();
	case 3:
		return GetClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 2>();
	case 4:
		return GetClearLoadStoreVariation<BUFFER_TYPE, DATA_TYPE, 3>();
	}
}

ClearLoadStoreMat* ClearLoadStoreMat::GetVariation(ClearLoadStoreType objType, ClearLoadStoreDataType dataType, u32 numComponents)
{
	switch(objType)
	{
	default:
	case ClearLoadStoreType::Texture:
		if(dataType == ClearLoadStoreDataType::Float)
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::Texture, ClearLoadStoreDataType::Float>(numComponents));
		else
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::Texture, ClearLoadStoreDataType::Int>(numComponents));
	case ClearLoadStoreType::TextureArray:
		if(dataType == ClearLoadStoreDataType::Float)
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Float>(numComponents));
		else
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::TextureArray, ClearLoadStoreDataType::Int>(numComponents));
	case ClearLoadStoreType::Buffer:
		if(dataType == ClearLoadStoreDataType::Float)
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Float>(numComponents));
		else
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::Buffer, ClearLoadStoreDataType::Int>(numComponents));
	case ClearLoadStoreType::StructuredBuffer:
		if(dataType == ClearLoadStoreDataType::Float)
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Float>(numComponents));
		else
			return Get(GetClearLoadStoreVariation<ClearLoadStoreType::StructuredBuffer, ClearLoadStoreDataType::Int>(numComponents));
	}
}

TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

// Note: Tile size was reduced from 32 to 16 because of macOS limitations. Ideally we should try keeping the larger
// size on non-macOS platforms, but currently where don't have a platform-specific way of setting this.
//
// The theory is that using larger tiles will amortize the cost of computing tile AABB's (which this shader uses,
// compared to the cheaper-to-compute frustums).
const u32 TiledDeferredImageBasedLightingMat::kTileSize = 16;

void TiledDeferredImageBasedLightingMat::Initialize()
{
	mSampleCount = mVariationParameters.GetUInt("MSAA_COUNT");

	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gGBufferATex", mGBufferA);
	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gGBufferBTex", mGBufferB);
	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gGBufferCTex", mGBufferC);
	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gDepthBufferTex", mGBufferDepth);

	mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gInColor", mInColorTextureParam);
	mGPUParameters->GetStorageTextureParameter(GPT_COMPUTE_PROGRAM, "gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGPUParameters->GetSampledTextureParameter(GPT_COMPUTE_PROGRAM, "gMSAACoverage", mMSAACoverageTexParam);

	mParamBuffer = gTiledImageBasedLightingParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);

	mImageBasedParams.Populate(mGPUParameters, GPT_COMPUTE_PROGRAM, false, false, true);

	mGPUParameters->SetUniformBuffer("ReflProbeParams", mReflProbeParamBuffer.Buffer);
}

void TiledDeferredImageBasedLightingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredImageBasedLightingMat::Execute(const RendererView& view, const SceneInfo& sceneInfo, const VisibleReflProbeData& probeData, const Inputs& inputs)
{
	BS_RENMAT_PROFILE_BLOCK

	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledImageBasedLightingParamDef.gFramebufferSize.Set(mParamBuffer, framebufferSize);

	Skybox* skybox = nullptr;
	if(view.GetRenderSettings().EnableSkybox)
		skybox = sceneInfo.Skybox;

	mReflProbeParamBuffer.Populate(skybox, probeData.GetNumProbes(), sceneInfo.ReflProbeCubemapsTex, viewProps.CapturingReflections);

	mParamBuffer->FlushToGpu();
	mReflProbeParamBuffer.Buffer->FlushToGpu();

	mGBufferA.Set(inputs.Gbuffer.Albedo);
	mGBufferB.Set(inputs.Gbuffer.Normals);
	mGBufferC.Set(inputs.Gbuffer.RoughMetal);
	mGBufferDepth.Set(inputs.Gbuffer.Depth);

	SPtr<Texture> skyFilteredRadiance;
	if(skybox)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	mImageBasedParams.PreintegratedEnvBrdfParam.Set(inputs.PreIntegratedGf);
	mImageBasedParams.ReflectionProbesParam.Set(probeData.GetProbeBuffer());
	mImageBasedParams.ReflectionProbeCubemapsTexParam.Set(sceneInfo.ReflProbeCubemapsTex);
	mImageBasedParams.SkyReflectionsTexParam.Set(skyFilteredRadiance);
	mImageBasedParams.AmbientOcclusionTexParam.Set(inputs.AmbientOcclusion);
	mImageBasedParams.SsrTexParam.Set(inputs.Ssr);

	mGPUParameters->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	mInColorTextureParam.Set(inputs.LightAccumulation);
	if(mSampleCount > 1)
	{
		mOutputTextureParam.Set(inputs.SceneColorTexArray, TextureSurface::kComplete);
		mMSAACoverageTexParam.Set(inputs.MsaaCoverage);
	}
	else
		mOutputTextureParam.Set(inputs.SceneColorTex);

	u32 numTilesX = (u32)Math::CeilToInt(width / (float)kTileSize);
	u32 numTilesY = (u32)Math::CeilToInt(height / (float)kTileSize);

	Bind();
	RenderAPI::Instance().DispatchCompute(numTilesX, numTilesY);
}

TiledDeferredImageBasedLightingMat* TiledDeferredImageBasedLightingMat::GetVariation(u32 msaaCount)
{
	switch(msaaCount)
	{
	case 1:
		return Get(GetVariation<1>());
	case 2:
		return Get(GetVariation<2>());
	case 4:
		return Get(GetVariation<4>());
	case 8:
	default:
		return Get(GetVariation<8>());
	}
}
}} // namespace bs::ct
