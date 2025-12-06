//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DTiledDeferred.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"
#include "B3DRenderBeast.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d {
namespace render {

TiledLightingUniformDefinition gTiledLightingUniformDefinition;

const u32 TiledDeferredLightingMaterial::kTileSize = 16;

void TiledDeferredLightingMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_COMPUTE_PROGRAM, mGpuParameterSet);
	mSampleCount = mVariationParameters.GetUI32("MSAA_COUNT");

	mGpuParameterSet->GetStorageBufferParameter("gLights", mLightBufferParam);
	mGpuParameterSet->GetSampledTextureParameter("gInColor", mInColorTextureParam);

	if(mGpuParameterSet->HasStorageTexture("gOutput"))
		mGpuParameterSet->GetStorageTextureParameter("gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGpuParameterSet->GetSampledTextureParameter("gMSAACoverage", mMSAACoverageTexParam);

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
}

void TiledDeferredLightingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredLightingMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const VisibleLightData& lightData, const GBufferTextures& gbuffer, const SPtr<Texture>& inputTexture, const SPtr<Texture>& lightAccumTex, const SPtr<Texture>& lightAccumTexArray, const SPtr<Texture>& msaaCoverage)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderSettings& settings = view.GetRenderSettings();

	mLightBufferParam.Set(lightData.GetLightBuffer());

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	GpuBufferSuballocation uniformBuffer = gTiledLightingUniformDefinition.AllocateTransient();

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledLightingUniformDefinition.gFramebufferSize.Set(uniformBuffer, framebufferSize);

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

		gTiledLightingUniformDefinition.gLightCounts.Set(uniformBuffer, lightCounts);
		gTiledLightingUniformDefinition.gLightStrides.Set(uniformBuffer, lightStrides);
	}
	else
	{
		Vector4I unshadowedLightCounts;
		unshadowedLightCounts[0] = lightData.GetUnshadowedLightCount(LightType::Directional);
		unshadowedLightCounts[1] = lightData.GetUnshadowedLightCount(LightType::Radial);
		unshadowedLightCounts[2] = lightData.GetUnshadowedLightCount(LightType::Spot);
		unshadowedLightCounts[3] = unshadowedLightCounts[0] + unshadowedLightCounts[1] + unshadowedLightCounts[2];

		Vector4I lightCounts;
		lightCounts[0] = lightData.GetLightCount(LightType::Directional);
		lightCounts[1] = lightData.GetLightCount(LightType::Radial);
		lightCounts[2] = lightData.GetLightCount(LightType::Spot);
		lightCounts[3] = lightCounts[0] + lightCounts[1] + lightCounts[2];

		Vector2I lightStrides;
		lightStrides[0] = lightCounts[0];
		lightStrides[1] = lightStrides[0] + lightCounts[1];

		if(!settings.EnableShadows)
			gTiledLightingUniformDefinition.gLightCounts.Set(uniformBuffer, lightCounts);
		else
			gTiledLightingUniformDefinition.gLightCounts.Set(uniformBuffer, unshadowedLightCounts);

		gTiledLightingUniformDefinition.gLightStrides.Set(uniformBuffer, lightStrides);
	}

	mUniformBufferParameter.Set(uniformBuffer);

	mGBufferParams.Bind(gbuffer);
	mGpuParameterSet->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());
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

TiledDeferredLightingMaterial* TiledDeferredLightingMaterial::GetVariation(u32 msaaCount)
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
	mGpuParameterSet->GetSampledTextureParameter("gInput", mInputParam);
}

void TextureArrayToMSAATexture::Prepare(const SPtr<Texture>& inputArray, const SPtr<Texture>& target)
{
	const TextureProperties& inputProps = inputArray->GetProperties();
	const TextureProperties& targetProps = target->GetProperties();

	B3D_ASSERT(inputProps.ArraySliceCount == targetProps.SampleCount);
	B3D_ASSERT(inputProps.Width == targetProps.Width);
	B3D_ASSERT(inputProps.Height == targetProps.Height);

	mInputParam.Set(inputArray);
}

void TextureArrayToMSAATexture::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& targetProps = target->GetProperties();

	Bind(commandBuffer);

	Area2 area(0.0f, 0.0f, (float)targetProps.Width, (float)targetProps.Height);
	GetRendererUtility().DrawScreenQuad(commandBuffer, area);
}

ClearLoadStoreUniformDefinition gClearLoadStoreUniformDefinition;

void ClearLoadStoreMaterial::Initialize()
{
	i32 objType = mVariationParameters.GetI32("OBJ_TYPE");

	if(objType == 0 || objType == 1)
		mGpuParameterSet->GetStorageTextureParameter("gOutput", mOutputTextureParam);
	else
		mGpuParameterSet->GetStorageBufferParameter("gOutput", mOutputBufferParam);

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
}

void ClearLoadStoreMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
	defines.Set("NUM_THREADS", kNumThreads);
}

void ClearLoadStoreMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& target, const Color& clearValue, const TextureSurface& surface)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& props = target->GetProperties();
	PixelFormat pf = props.Format;

	B3D_ASSERT(!PixelUtility::IsCompressed(pf));

	mOutputTextureParam.Set(target, surface);

	GpuBufferSuballocation uniformBuffer = gClearLoadStoreUniformDefinition.AllocateTransient();

	u32 width = props.Width;
	u32 height = props.Height;
	gClearLoadStoreUniformDefinition.gSize.Set(uniformBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreUniformDefinition.gFloatClearVal.Set(uniformBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreUniformDefinition.gIntClearVal.Set(uniformBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	mUniformBufferParameter.Set(uniformBuffer);

	Bind(commandBuffer);

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * kTileSize);
	u32 numGroupsY = Math::DivideAndRoundUp(height, kNumThreads * kTileSize);

	commandBuffer.DispatchCompute(numGroupsX, numGroupsY);
}

void ClearLoadStoreMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& target, const Color& clearValue)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	GpuBufferSuballocation uniformBuffer = gClearLoadStoreUniformDefinition.AllocateTransient();

	u32 height = 1;
	gClearLoadStoreUniformDefinition.gSize.Set(uniformBuffer, Vector2I((i32)width, (i32)height));
	gClearLoadStoreUniformDefinition.gFloatClearVal.Set(uniformBuffer, Vector4(clearValue.R, clearValue.G, clearValue.A, clearValue.A));
	gClearLoadStoreUniformDefinition.gIntClearVal.Set(uniformBuffer, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.A, *(i32*)&clearValue.A));

	mUniformBufferParameter.Set(uniformBuffer);

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

ClearLoadStoreMaterial* ClearLoadStoreMaterial::GetVariation(ClearLoadStoreType objType, ClearLoadStoreDataType dataType, u32 numComponents)
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

TiledImageBasedLightingUniformDefinition gTiledImageBasedLightingUniformDefinition;

// Note: Tile size was reduced from 32 to 16 because of macOS limitations. Ideally we should try keeping the larger
// size on non-macOS platforms, but currently where don't have a platform-specific way of setting this.
//
// The theory is that using larger tiles will amortize the cost of computing tile AABB's (which this shader uses,
// compared to the cheaper-to-compute frustums).
const u32 TiledDeferredImageBasedLightingMaterial::kTileSize = 16;

void TiledDeferredImageBasedLightingMaterial::Initialize()
{
	mSampleCount = mVariationParameters.GetUI32("MSAA_COUNT");

	mGpuParameterSet->GetSampledTextureParameter("gGBufferATex", mGBufferA);
	mGpuParameterSet->GetSampledTextureParameter("gGBufferBTex", mGBufferB);
	mGpuParameterSet->GetSampledTextureParameter("gGBufferCTex", mGBufferC);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mGBufferDepth);

	mGpuParameterSet->GetSampledTextureParameter("gInColor", mInColorTextureParam);
	mGpuParameterSet->GetStorageTextureParameter("gOutput", mOutputTextureParam);

	if(mSampleCount > 1)
		mGpuParameterSet->GetSampledTextureParameter("gMSAACoverage", mMSAACoverageTexParam);

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);

	mImageBasedParams.Initialize(mGpuParameterSet, GPT_COMPUTE_PROGRAM, false, false, true);

	mGpuParameterSet->SetUniformBuffer("ReflProbeParams", mReflProbeParamBuffer.Buffer);
}

void TiledDeferredImageBasedLightingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

void TiledDeferredImageBasedLightingMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SceneInfo& sceneInfo, const VisibleReflectionProbeData& probeData, const Inputs& inputs)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	GpuBufferSuballocation uniformBuffer = gTiledImageBasedLightingUniformDefinition.AllocateTransient();

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledImageBasedLightingUniformDefinition.gFramebufferSize.Set(uniformBuffer, framebufferSize);

	mUniformBufferParameter.Set(uniformBuffer);

	Skybox* skybox = nullptr;
	if(view.GetRenderSettings().EnableSkybox)
		skybox = sceneInfo.Skybox;

	mReflProbeParamBuffer.Populate(skybox, probeData.GetProbeCount(), sceneInfo.ReflProbeCubemapsTex, viewProps.CapturingReflections);

	mReflProbeParamBuffer.Buffer->FlushCache();

	mGBufferA.Set(inputs.Gbuffer.Albedo);
	mGBufferB.Set(inputs.Gbuffer.Normals);
	mGBufferC.Set(inputs.Gbuffer.RoughMetal);
	mGBufferDepth.Set(inputs.Gbuffer.Depth);

	SPtr<Texture> skyFilteredRadiance;
	if(skybox)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	mImageBasedParams.PreintegratedEnvBrdfParameter.Set(inputs.PreIntegratedGf);
	mImageBasedParams.ReflectionProbesParameter.Set(probeData.GetProbeBuffer());
	mImageBasedParams.ReflectionProbeCubemapsTexParameter.Set(sceneInfo.ReflProbeCubemapsTex);
	mImageBasedParams.SkyReflectionsTexParam.Set(skyFilteredRadiance);
	mImageBasedParams.AmbientOcclusionTexParam.Set(inputs.AmbientOcclusion);
	mImageBasedParams.SsrTexParameter.Set(inputs.Ssr);

	mGpuParameterSet->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());

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

TiledDeferredImageBasedLightingMaterial* TiledDeferredImageBasedLightingMaterial::GetVariation(u32 msaaCount)
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
