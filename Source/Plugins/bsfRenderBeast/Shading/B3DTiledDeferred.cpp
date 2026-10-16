//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DTiledDeferred.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DSkybox.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "FrameGraph/B3DFrameGraph.h"

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
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
}

void TiledDeferredLightingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

TShared<GpuParameterSet> TiledDeferredLightingMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const VisibleLightData& lightData, const GBufferTextures& gbuffer,
	const TShared<Texture>& inputTexture, const TShared<Texture>& lightAccumTex, const TShared<Texture>& lightAccumTexArray, const TShared<Texture>& msaaCoverage)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderSettings& settings = view.GetRenderSettings();

	mLightBufferParam.Set(set, lightData.GetLightBuffer());

	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	GpuBufferMappedScope uniforms = gTiledLightingUniformDefinition.AllocateTransient().Map();

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledLightingUniformDefinition.gFramebufferSize.Set(uniforms, framebufferSize);

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

		gTiledLightingUniformDefinition.gLightCounts.Set(uniforms, lightCounts);
		gTiledLightingUniformDefinition.gLightStrides.Set(uniforms, lightStrides);
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
			gTiledLightingUniformDefinition.gLightCounts.Set(uniforms, lightCounts);
		else
			gTiledLightingUniformDefinition.gLightCounts.Set(uniforms, unshadowedLightCounts);

		gTiledLightingUniformDefinition.gLightStrides.Set(uniforms, lightStrides);
	}

	mUniformBufferParameter.Set(set, uniforms);

	mGBufferParams.Bind(set, gbuffer);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());
	mInColorTextureParam.Set(set, inputTexture);

	if(mSampleCount > 1)
	{
		mOutputTextureParam.Set(set, lightAccumTexArray, TextureSurface::kComplete);
		mMSAACoverageTexParam.Set(set, msaaCoverage);
	}
	else
		mOutputTextureParam.Set(set, lightAccumTex);

	return set;
}

void TiledDeferredLightingMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, u32 width, u32 height) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const u32 numTilesX = (u32)Math::CeilToInt(width / (float)kTileSize);
	const u32 numTilesY = (u32)Math::CeilToInt(height / (float)kTileSize);

	Bind(commandBuffer, set);
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

void TextureArrayToMSAATexture::Prepare(const TShared<Texture>& inputArray, const TShared<Texture>& target)
{
	const TextureProperties& inputProps = inputArray->GetProperties();
	const TextureProperties& targetProps = target->GetProperties();

	B3D_ASSERT(inputProps.ArraySliceCount == targetProps.SampleCount);
	B3D_ASSERT(inputProps.Width == targetProps.Width);
	B3D_ASSERT(inputProps.Height == targetProps.Height);

	mInputParam.Set(inputArray);
}

void TextureArrayToMSAATexture::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& target)
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

void ClearLoadStoreMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& target, const Color& clearValue, const TextureSurface& surface)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& props = target->GetProperties();
	PixelFormat pf = props.Format;

	B3D_ASSERT(!PixelUtility::IsCompressed(pf));

	mOutputTextureParam.Set(target, surface);

	GpuBufferMappedScope uniforms = gClearLoadStoreUniformDefinition.AllocateTransient().Map();

	u32 width = props.Width;
	u32 height = props.Height;
	gClearLoadStoreUniformDefinition.gSize.Set(uniforms, Vector2I((i32)width, (i32)height));
	gClearLoadStoreUniformDefinition.gFloatClearVal.Set(uniforms, Vector4(clearValue.R, clearValue.G, clearValue.B, clearValue.A));
	gClearLoadStoreUniformDefinition.gIntClearVal.Set(uniforms, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.B, *(i32*)&clearValue.A));

	mUniformBufferParameter.Set(uniforms);

	Bind(commandBuffer);

	u32 numGroupsX = Math::DivideAndRoundUp(width, kNumThreads * kTileSize);
	u32 numGroupsY = Math::DivideAndRoundUp(height, kNumThreads * kTileSize);

	commandBuffer.DispatchCompute(numGroupsX, numGroupsY);
}

void ClearLoadStoreMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<GpuBuffer>& target, const Color& clearValue)
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

	GpuBufferMappedScope uniforms = gClearLoadStoreUniformDefinition.AllocateTransient().Map();

	u32 height = 1;
	gClearLoadStoreUniformDefinition.gSize.Set(uniforms, Vector2I((i32)width, (i32)height));
	gClearLoadStoreUniformDefinition.gFloatClearVal.Set(uniforms, Vector4(clearValue.R, clearValue.G, clearValue.B, clearValue.A));
	gClearLoadStoreUniformDefinition.gIntClearVal.Set(uniforms, Vector4I(*(i32*)&clearValue.R, *(i32*)&clearValue.G, *(i32*)&clearValue.B, *(i32*)&clearValue.A));

	mUniformBufferParameter.Set(uniforms);

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

	mImageBasedParams.Initialize(mGpuParameterSet);

	mGpuParameterSet->TryGetUniformBufferParameter("ReflProbeParams", mReflProbeParamsUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
}

void TiledDeferredImageBasedLightingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_SIZE", kTileSize);
}

TShared<GpuParameterSet> TiledDeferredImageBasedLightingMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const RenderBeastScene& scene,
	const VisibleReflectionProbeData& probeData, const Inputs& inputs)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;

	GpuBufferMappedScope uniforms = gTiledImageBasedLightingUniformDefinition.AllocateTransient().Map();

	Vector2I framebufferSize;
	framebufferSize[0] = width;
	framebufferSize[1] = height;
	gTiledImageBasedLightingUniformDefinition.gFramebufferSize.Set(uniforms, framebufferSize);

	mUniformBufferParameter.Set(set, uniforms);

	Skybox* skybox = nullptr;
	if(view.GetRenderSettings().EnableSkybox)
		skybox = scene.GetSkybox();

	GpuBufferSuballocation reflProbeParamsBuffer = gGlobalReflectionProbeUniformBufferDefinition.AllocateTransient();
	ReflectionProbeRenderState::PopulateGlobalReflectionProbeUniformBuffer(reflProbeParamsBuffer, skybox, probeData.GetProbeCount(), scene.GetReflectionProbeCubemapsTex(), viewProps.CapturingReflections);
	mReflProbeParamsUniformBufferParameter.Set(set, reflProbeParamsBuffer);

	mGBufferA.Set(set, inputs.Gbuffer.Albedo);
	mGBufferB.Set(set, inputs.Gbuffer.Normals);
	mGBufferC.Set(set, inputs.Gbuffer.RoughMetal);
	mGBufferDepth.Set(set, inputs.Gbuffer.Depth);

	TShared<Texture> skyFilteredRadiance;
	if(skybox)
		skyFilteredRadiance = skybox->GetFilteredRadiance();

	mImageBasedParams.PreintegratedEnvBrdfParameter.Set(set, inputs.PreIntegratedGf);
	mImageBasedParams.ReflectionProbesParameter.Set(set, probeData.GetProbeBuffer());
	mImageBasedParams.ReflectionProbeCubemapsTexParameter.Set(set, scene.GetReflectionProbeCubemapsTex());
	mImageBasedParams.SkyReflectionsTexParam.Set(set, skyFilteredRadiance);
	mImageBasedParams.AmbientOcclusionTexParam.Set(set, inputs.AmbientOcclusion);
	mImageBasedParams.SsrTexParameter.Set(set, inputs.Ssr);

	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	mInColorTextureParam.Set(set, inputs.LightAccumulation);
	if(mSampleCount > 1)
	{
		mOutputTextureParam.Set(set, inputs.SceneColorTexArray, TextureSurface::kComplete);
		mMSAACoverageTexParam.Set(set, inputs.MsaaCoverage);
	}
	else
		mOutputTextureParam.Set(set, inputs.SceneColorTex);

	return set;
}

void TiledDeferredImageBasedLightingMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, u32 width, u32 height) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const u32 numTilesX = (u32)Math::CeilToInt(width / (float)kTileSize);
	const u32 numTilesY = (u32)Math::CeilToInt(height / (float)kTileSize);

	Bind(commandBuffer, set);
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
