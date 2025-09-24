//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DTiledDeferred.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"
#include "B3DRenderBeast.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d {
namespace render {

TiledLightingParamDef gTiledLightingParamDef;

const u32 TiledDeferredLightingMat::kTileSize = 16;

void TiledDeferredLightingMat::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_COMPUTE_PROGRAM, mGPUParameters);
	mSampleCount = mVariationParameters.GetUI32("MSAA_COUNT");

	mGPUParameters->GetStorageBufferParameter("gLights", mLightBufferParam);
	mGPUParameters->GetSampledTextureParameter("gInColor", mInColorTextureParam);

	if(mGPUParameters->HasStorageTexture("gOutput"))
		mGPUParameters->GetStorageTextureParameter("gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGPUParameters->GetSampledTextureParameter("gMSAACoverage", mMSAACoverageTexParam);

	mParamBuffer = gTiledLightingParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
}

void TiledDeferredLightingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredLightingMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const VisibleLightData& lightData, const GBufferTextures& gbuffer, const SPtr<Texture>& inputTexture, const SPtr<Texture>& lightAccumTex, const SPtr<Texture>& lightAccumTexArray, const SPtr<Texture>& msaaCoverage)
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

	mParamBuffer->FlushCache();

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

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numTilesX, numTilesY);
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
	mGPUParameters->GetSampledTextureParameter("gInput", mInputParam);
}

void TextureArrayToMSAATexture::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& inputArray, const SPtr<Texture>& target)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& inputProps = inputArray->GetProperties();
	const TextureProperties& targetProps = target->GetProperties();

	B3D_ASSERT(inputProps.ArraySliceCount == targetProps.SampleCount);
	B3D_ASSERT(inputProps.Width == targetProps.Width);
	B3D_ASSERT(inputProps.Height == targetProps.Height);

	mInputParam.Set(inputArray);

	Bind(commandBuffer);

	Area2 area(0.0f, 0.0f, (float)targetProps.Width, (float)targetProps.Height);
	GetRendererUtility().DrawScreenQuad(commandBuffer, area);
}

ClearLoadStoreParamDef gClearLoadStoreParamDef;

void ClearLoadStoreMat::Initialize()
{
	i32 objType = mVariationParameters.GetI32("OBJ_TYPE");

	if(objType == 0 || objType == 1)
		mGPUParameters->GetStorageTextureParameter("gOutput", mOutputTextureParam);
	else
		mGPUParameters->GetStorageBufferParameter("gOutput", mOutputBufferParam);

	mParamBuffer = gClearLoadStoreParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
}

void ClearLoadStoreMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
	defines.Set("NUM_THREADS", kNumThreads);
}

void ClearLoadStoreMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const Color& clearValue, const TextureSurface& surface)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& props = target->GetProperties();
	PixelFormat pf = props.Format;

	B3D_ASSERT(!PixelUtility::IsCompressed(pf));

	mOutputTextureParam.Set(target, surface);

	u32 width = props.Width;
	u32 height = props.Height;
	gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreParamDef.gFloatClearVal.Set(mParamBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreParamDef.gIntClearVal.Set(mParamBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	Bind(commandBuffer);

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * kTileSize);
	u32 numGroupsY = Math::DivideAndRoundUp(height, kNumThreads * kTileSize);

	commandBuffer.DispatchCompute(numGroupsX, numGroupsY);
}

void ClearLoadStoreMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& target, const Color& clearValue)
{
	BS_RENMAT_PROFILE_BLOCK

	mOutputBufferParam.Set(target);

	const GpuBufferInformation& bufferInformation = target->GetInformation();

	u32 width = 0;
	if(bufferInformation.Type == GpuBufferType::SimpleStorage)
		width = bufferInformation.SimpleStorage.Count;
	else
	{
		B3D_ENSURE(bufferInformation.Type == GpuBufferType::StructuredStorage);
		width = bufferInformation.StructuredStorage.Count;
	}

	u32 height = 1;
	gClearLoadStoreParamDef.gSize.Set(mParamBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreParamDef.gFloatClearVal.Set(mParamBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreParamDef.gIntClearVal.Set(mParamBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	Bind(commandBuffer);

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * (kTileSize * kTileSize));
	commandBuffer.DispatchCompute(numGroupsX, 1);
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
	mSampleCount = mVariationParameters.GetUI32("MSAA_COUNT");

	mGPUParameters->GetSampledTextureParameter("gGBufferATex", mGBufferA);
	mGPUParameters->GetSampledTextureParameter("gGBufferBTex", mGBufferB);
	mGPUParameters->GetSampledTextureParameter("gGBufferCTex", mGBufferC);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mGBufferDepth);

	mGPUParameters->GetSampledTextureParameter("gInColor", mInColorTextureParam);
	mGPUParameters->GetStorageTextureParameter("gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGPUParameters->GetSampledTextureParameter("gMSAACoverage", mMSAACoverageTexParam);

	mParamBuffer = gTiledImageBasedLightingParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);

	mImageBasedParams.Populate(mGPUParameters, GPT_COMPUTE_PROGRAM, false, false, true);

	mGPUParameters->SetUniformBuffer("ReflProbeParams", mReflProbeParamBuffer.Buffer);
}

void TiledDeferredImageBasedLightingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredImageBasedLightingMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SceneInfo& sceneInfo, const VisibleReflProbeData& probeData, const Inputs& inputs)
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

	mParamBuffer->FlushCache();
	mReflProbeParamBuffer.Buffer->FlushCache();

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

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(numTilesX, numTilesY);
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
}} // namespace b3d::render
