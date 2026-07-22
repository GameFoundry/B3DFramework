//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPostProcessing.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "Renderer/B3DRendererUtility.h"
#include "Renderer/B3DRenderer.h"
#include "Components/B3DCamera.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Image/B3DPixelUtility.h"
#include "Utility/B3DBitwise.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "B3DRendererView.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Utility/B3DRendererTextures.h"
#include "GpuBackend/B3DVertexDescription.h"

namespace b3d { namespace render {

void SetSamplerState(const TShared<GpuParameterSet>& params, const String& name, const String& secondaryName, const TShared<SamplerState>& samplerState, bool optional = false)
{
	if(params->HasSamplerState(name))
		params->SetSamplerState(name, samplerState);
	else
	{
		if(optional)
		{
			if(params->HasSamplerState(secondaryName))
				params->SetSamplerState(secondaryName, samplerState);
		}
		else
			params->SetSamplerState(secondaryName, samplerState);
	}
}

DownsampleUniformDefinition gDownsampleUniformDefinition;

void DownsampleMaterial::Initialize()
{
	mGpuParameterSet->TryGetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void DownsampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& input, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Populate parameter buffer
	GpuBufferMappedScope uniforms = gDownsampleUniformDefinition.AllocateTransient().Map();
	const TextureProperties& rtProps = input->GetProperties();

	bool MSAA = mVariationParameters.GetI32("MSAA") > 0;
	if(MSAA)
	{
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(-1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(-1.0f, 1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(1.0f, 1.0f));
	}
	else
	{
		Vector2 invTextureSize(1.0f / rtProps.Width, 1.0f / rtProps.Height);

		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(-1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(-1.0f, 1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(1.0f, 1.0f));
	}

	// Set parameters
	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(input);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGpuParameterSet, RT_DEPTH_STENCIL));

	Bind(commandBuffer);

	if(MSAA)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)rtProps.Width, (float)rtProps.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation DownsampleMaterial::GetOutputDesc(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1, Math::CeilToInt(rtProps.Width * 0.5f));
	u32 height = std::max(1, Math::CeilToInt(rtProps.Height * 0.5f));

	return PooledRenderTextureCreateInformation::Create2D(rtProps.Format, width, height, TextureUsageFlag::RenderTarget);
}

DownsampleMaterial* DownsampleMaterial::GetVariation(u32 quality, bool msaa)
{
	if(quality == 0)
	{
		if(msaa)
			return Get(GetVariation<0, true>());
		else
			return Get(GetVariation<0, false>());
	}
	else
	{
		if(msaa)
			return Get(GetVariation<1, true>());
		else
			return Get(GetVariation<1, false>());
	}
}

EyeAdaptHistogramUniformDefinition gEyeAdaptHistogramUniformDefinition;

void EyeAdaptHistogramMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSceneColorTex", mSceneColorParameter);
	mGpuParameterSet->GetStorageTextureParameter("gOutputTex", mOutputTextureParameter);
}

void EyeAdaptHistogramMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", kThreadGroupSizeY);
	defines.Set("LOOP_COUNT_X", kLoopCountX);
	defines.Set("LOOP_COUNT_Y", kLoopCountY);
}

void EyeAdaptHistogramMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& input, const TShared<Texture>& output, const AutoExposureSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Populate parameter buffer
	GpuBufferMappedScope uniforms = gEyeAdaptHistogramUniformDefinition.AllocateTransient().Map();

	const TextureProperties& props = input->GetProperties();
	Vector4I offsetAndSize(0, 0, (i32)props.Width, (i32)props.Height);

	gEyeAdaptHistogramUniformDefinition.gHistogramParams.Set(uniforms, GetHistogramScaleOffset(settings));
	gEyeAdaptHistogramUniformDefinition.gPixelOffsetAndSize.Set(uniforms, offsetAndSize);

	Vector2I threadGroupCount = GetThreadGroupCount(input);
	gEyeAdaptHistogramUniformDefinition.gThreadGroupCount.Set(uniforms, threadGroupCount);

	// Set parameters
	mUniformBufferParameter.Set(uniforms);
	mSceneColorParameter.Set(input);

	// Dispatch
	mOutputTextureParameter.Set(output);

	Bind(commandBuffer);

	commandBuffer.DispatchCompute(threadGroupCount.X, threadGroupCount.Y);
}

PooledRenderTextureCreateInformation EyeAdaptHistogramMaterial::GetOutputDesc(const TShared<Texture>& target)
{
	Vector2I threadGroupCount = GetThreadGroupCount(target);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, kHistogramNumTexels, numHistograms, TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
}

Vector2I EyeAdaptHistogramMaterial::GetThreadGroupCount(const TShared<Texture>& target)
{
	const u32 texelsPerThreadGroupX = kThreadGroupSizeX * kLoopCountX;
	const u32 texelsPerThreadGroupY = kThreadGroupSizeY * kLoopCountY;

	const TextureProperties& props = target->GetProperties();

	Vector2I threadGroupCount;
	threadGroupCount.X = ((i32)props.Width + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
	threadGroupCount.Y = ((i32)props.Height + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

	return threadGroupCount;
}

Vector2 EyeAdaptHistogramMaterial::GetHistogramScaleOffset(const AutoExposureSettings& settings)
{
	float diff = settings.HistogramLog2Max - settings.HistogramLog2Min;
	float scale = 1.0f / diff;
	float offset = -settings.HistogramLog2Min * scale;

	return Vector2(scale, offset);
}

EyeAdaptHistogramReduceUniformDefinition gEyeAdaptHistogramReduceUniformDefinition;

void EyeAdaptHistogramReduceMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gHistogramTex", mHistogramTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTextureParameter);
}

void EyeAdaptHistogramReduceMaterial::Prepare(const TShared<Texture>& sceneColor, const TShared<Texture>& histogram, const TShared<Texture>& prevFrame)
{
	GpuBufferMappedScope uniforms = gEyeAdaptHistogramReduceUniformDefinition.AllocateTransient().Map();

	mHistogramTextureParameter.Set(histogram);

	TShared<Texture> eyeAdaptationTex;
	if(prevFrame == nullptr) // Could be that this is the first run
		eyeAdaptationTex = Texture::kWhite;
	else
		eyeAdaptationTex = prevFrame;

	mEyeAdaptationTextureParameter.Set(eyeAdaptationTex);

	Vector2I threadGroupCount = EyeAdaptHistogramMaterial::GetThreadGroupCount(sceneColor);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	gEyeAdaptHistogramReduceUniformDefinition.gThreadGroupCount.Set(uniforms, numHistograms);

	mUniformBufferParameter.Set(uniforms);
}

void EyeAdaptHistogramReduceMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet, RT_DEPTH_STENCIL);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);

	Area2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMaterial::kHistogramNumTexels, 2.0f);
	GetRendererUtility().DrawScreenQuad(commandBuffer, drawUV);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptHistogramReduceMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, EyeAdaptHistogramMaterial::kHistogramNumTexels, 2, TextureUsageFlag::RenderTarget);
}

EyeAdaptationUniformDefinition gEyeAdaptationUniformDefinition;

void EyeAdaptationMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("EyeAdaptationParams", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gHistogramTex", mReducedHistogramTex);
}

void EyeAdaptationMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", EyeAdaptHistogramMaterial::kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMaterial::kThreadGroupSizeY);
}

void EyeAdaptationMaterial::Prepare(const TShared<Texture>& reducedHistogram, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	mReducedHistogramTex.Set(reducedHistogram);

	GpuBufferMappedScope uniforms = gEyeAdaptationUniformDefinition.AllocateTransient().Map();
	PopulateUniformBuffer(uniforms, frameDelta, settings, exposureScale);

	mUniformBufferParameter.Set(uniforms);
}

void EyeAdaptationMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet, RT_DEPTH_STENCIL);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TextureUsageFlag::RenderTarget);
}

void EyeAdaptationMaterial::PopulateUniformBuffer(const GpuBufferMappedScope& uniforms, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	Vector2 histogramScaleAndOffset = EyeAdaptHistogramMaterial::GetHistogramScaleOffset(settings);

	Vector4 eyeAdaptationParams[3];
	eyeAdaptationParams[0].X = histogramScaleAndOffset.X;
	eyeAdaptationParams[0].Y = histogramScaleAndOffset.Y;

	float histogramPctHigh = Math::Clamp01(settings.HistogramPctHigh);

	eyeAdaptationParams[0].Z = std::min(Math::Clamp01(settings.HistogramPctLow), histogramPctHigh);
	eyeAdaptationParams[0].W = histogramPctHigh;

	eyeAdaptationParams[1].X = std::min(settings.MinEyeAdaptation, settings.MaxEyeAdaptation);
	eyeAdaptationParams[1].Y = settings.MaxEyeAdaptation;

	eyeAdaptationParams[1].Z = settings.EyeAdaptationSpeedUp;
	eyeAdaptationParams[1].W = settings.EyeAdaptationSpeedDown;

	eyeAdaptationParams[2].X = Math::RaiseToPower(2.0f, exposureScale);
	eyeAdaptationParams[2].Y = frameDelta;

	eyeAdaptationParams[2].Z = Math::RaiseToPower(2.0f, settings.HistogramLog2Min);
	eyeAdaptationParams[2].W = 0.0f; // Unused

	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniforms, eyeAdaptationParams[0], 0);
	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniforms, eyeAdaptationParams[1], 1);
	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniforms, eyeAdaptationParams[2], 2);
}

void EyeAdaptationBasicSetupMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("EyeAdaptationParams", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;

	TShared<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
	SetSamplerState(mGpuParameterSet, "gInputSamp", "gInputTex", samplerState);
}

void EyeAdaptationBasicSetupMaterial::Prepare(const TShared<Texture>& input, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	mInputTextureParameter.Set(input);

	GpuBufferMappedScope uniforms = gEyeAdaptationUniformDefinition.AllocateTransient().Map();
	EyeAdaptationMaterial::PopulateUniformBuffer(uniforms, frameDelta, settings, exposureScale);

	mUniformBufferParameter.Set(uniforms);
}

void EyeAdaptationBasicSetupMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationBasicSetupMaterial::GetOutputDesc(const TShared<Texture>& input)
{
	auto& props = input->GetProperties();
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, props.Width, props.Height, TextureUsageFlag::RenderTarget);
}

EyeAdaptationBasicUniformDefinition gEyeAdaptationBasicUniformDefinition;

void EyeAdaptationBasicMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("EyeAdaptationParams", mEyeAdaptationUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gCurFrameTex", mCurrentFrameTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gPrevFrameTex", mPreviousFrameTextureParameter);
}

void EyeAdaptationBasicMaterial::Prepare(const TShared<Texture>& curFrame, const TShared<Texture>& prevFrame, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	GpuBufferMappedScope eyeAdaptationUniforms = gEyeAdaptationUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope eyeAdaptationBasicUniforms = gEyeAdaptationBasicUniformDefinition.AllocateTransient().Map();

	EyeAdaptationMaterial::PopulateUniformBuffer(eyeAdaptationUniforms, frameDelta, settings, exposureScale);

	auto& texProps = curFrame->GetProperties();
	Vector2I texSize = { (i32)texProps.Width, (i32)texProps.Height };

	gEyeAdaptationBasicUniformDefinition.gInputTexSize.Set(eyeAdaptationBasicUniforms, texSize);

	mEyeAdaptationUniformBufferParameter.Set(eyeAdaptationUniforms);
	mUniformBufferParameter.Set(eyeAdaptationBasicUniforms);
	mCurrentFrameTextureParameter.Set(curFrame);

	if(prevFrame == nullptr) // Could be that this is the first run
		mPreviousFrameTextureParameter.Set(Texture::kWhite);
	else
		mPreviousFrameTextureParameter.Set(prevFrame);
}

void EyeAdaptationBasicMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationBasicMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TextureUsageFlag::RenderTarget);
}

CreateTonemapLUTUniformDefinition gCreateTonemapLUTUniformDefinition;
WhiteBalanceUniformDefinition gWhiteBalanceUniformDefinition;

void CreateTonemap2DLUTMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("WhiteBalanceInput", mWhiteBalanceUniformBufferParameter);
}

void CreateTonemap2DLUTMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", kLutSize);
}

void CreateTonemap2DLUTMaterial::Prepare(const RenderSettings& settings)
{
	GpuBufferMappedScope lutUniforms = gCreateTonemapLUTUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope whiteBalanceUniforms = gWhiteBalanceUniformDefinition.AllocateTransient().Map();

	PopulateTonemappingUniformBuffer(settings, lutUniforms);
	PopulateWhiteBalanceUniformBuffer(settings, whiteBalanceUniforms);

	mUniformBufferParameter.Set(lutUniforms);
	mWhiteBalanceUniformBufferParameter.Set(whiteBalanceUniforms);
}

void CreateTonemap2DLUTMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTexture>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void CreateTonemap2DLUTMaterial::PopulateTonemappingUniformBuffer(const RenderSettings& settings, const GpuBufferMappedScope& uniforms)
{
	// Set parameters
	gCreateTonemapLUTUniformDefinition.gGammaAdjustment.Set(uniforms, 2.2f / settings.Gamma);

	// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
	// gamma when running on Mac (value 2)
	gCreateTonemapLUTUniformDefinition.gGammaCorrectionType.Set(uniforms, 0);

	Vector4 tonemapParams[2];
	tonemapParams[0].X = settings.Tonemapping.FilmicCurveShoulderStrength;
	tonemapParams[0].Y = settings.Tonemapping.FilmicCurveLinearStrength;
	tonemapParams[0].Z = settings.Tonemapping.FilmicCurveLinearAngle;
	tonemapParams[0].W = settings.Tonemapping.FilmicCurveToeStrength;

	tonemapParams[1].X = settings.Tonemapping.FilmicCurveToeNumerator;
	tonemapParams[1].Y = settings.Tonemapping.FilmicCurveToeDenominator;
	tonemapParams[1].Z = settings.Tonemapping.FilmicCurveLinearWhitePoint;
	tonemapParams[1].W = 0.0f; // Unused

	gCreateTonemapLUTUniformDefinition.gTonemapParams.Set(uniforms, tonemapParams[0], 0);
	gCreateTonemapLUTUniformDefinition.gTonemapParams.Set(uniforms, tonemapParams[1], 1);

	// Set color grading params
	gCreateTonemapLUTUniformDefinition.gSaturation.Set(uniforms, settings.ColorGrading.Saturation);
	gCreateTonemapLUTUniformDefinition.gContrast.Set(uniforms, settings.ColorGrading.Contrast);
	gCreateTonemapLUTUniformDefinition.gGain.Set(uniforms, settings.ColorGrading.Gain);
	gCreateTonemapLUTUniformDefinition.gOffset.Set(uniforms, settings.ColorGrading.Offset);
}

void CreateTonemap2DLUTMaterial::PopulateWhiteBalanceUniformBuffer(const RenderSettings& settings, const GpuBufferMappedScope& uniforms)
{
	gWhiteBalanceUniformDefinition.gWhiteTemp.Set(uniforms, settings.WhiteBalance.Temperature);
	gWhiteBalanceUniformDefinition.gWhiteOffset.Set(uniforms, settings.WhiteBalance.Tint);
}

PooledRenderTextureCreateInformation CreateTonemap2DLUTMaterial::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, kLutSize * kLutSize, kLutSize, TextureUsageFlag::RenderTarget);
}

void CreateTonemap3DLUTMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("WhiteBalanceInput", mWhiteBalanceUniformBufferParameter);
	mGpuParameterSet->GetStorageTextureParameter("gOutputTex", mOutputTextureParameter);
}

void CreateTonemap3DLUTMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMaterial::kLutSize);
}

void CreateTonemap3DLUTMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& output, const RenderSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope lutUniforms = gCreateTonemapLUTUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope whiteBalanceUniforms = gWhiteBalanceUniformDefinition.AllocateTransient().Map();

	CreateTonemap2DLUTMaterial::PopulateTonemappingUniformBuffer(settings, lutUniforms);
	CreateTonemap2DLUTMaterial::PopulateWhiteBalanceUniformBuffer(settings, whiteBalanceUniforms);

	mUniformBufferParameter.Set(lutUniforms);
	mWhiteBalanceUniformBufferParameter.Set(whiteBalanceUniforms);

	// Dispatch
	mOutputTextureParameter.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(CreateTonemap2DLUTMaterial::kLutSize / 8, CreateTonemap2DLUTMaterial::kLutSize / 8, CreateTonemap2DLUTMaterial::kLutSize);
}

PooledRenderTextureCreateInformation CreateTonemap3DLUTMaterial::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create3D(PF_RGBA8, CreateTonemap2DLUTMaterial::kLutSize, CreateTonemap2DLUTMaterial::kLutSize, CreateTonemap2DLUTMaterial::kLutSize, TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
}

TonemappingUniformDefinition gTonemappingUniformDefinition;

void TonemappingMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gBloomTex", mBloomTextureParameter);

	if(!mVariationParameters.GetBool("GAMMA_ONLY"))
		mGpuParameterSet->GetSampledTextureParameter("gColorLUT", mColorLUTParameter);
}

void TonemappingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMaterial::kLutSize);
}

void TonemappingMaterial::Prepare(const TShared<Texture>& sceneColor, const TShared<Texture>& eyeAdaptation, const TShared<Texture>& bloom, const TShared<Texture>& colorLUT, const RenderSettings& settings)
{
	const TextureProperties& texProps = sceneColor->GetProperties();

	GpuBufferMappedScope uniforms = gTonemappingUniformDefinition.AllocateTransient().Map();

	gTonemappingUniformDefinition.gRawGamma.Set(uniforms, 1.0f / settings.Gamma);
	gTonemappingUniformDefinition.gManualExposureScale.Set(uniforms, Math::RaiseToPower(2.0f, settings.ExposureScale));
	gTonemappingUniformDefinition.gTexSize.Set(uniforms, Vector2((float)texProps.Width, (float)texProps.Height));
	gTonemappingUniformDefinition.gBloomTint.Set(uniforms, settings.Bloom.Tint);
	gTonemappingUniformDefinition.gNumSamples.Set(uniforms, texProps.SampleCount);

	mUniformBufferParameter.Set(uniforms);

	mInputTextureParameter.Set(sceneColor);
	mColorLUTParameter.Set(colorLUT);
	mEyeAdaptationTextureParameter.Set(eyeAdaptation);
	mBloomTextureParameter.Set(bloom != nullptr ? bloom : Texture::kBlack);
}

void TonemappingMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

TonemappingMaterial* TonemappingMaterial::GetVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA)
{
	if(volumeLUT)
	{
		if(gammaOnly)
		{
			if(autoExposure)
			{
				if(MSAA)
					return Get(GetVariation<true, true, true, true>());
				else
					return Get(GetVariation<true, true, true, false>());
			}
			else
			{
				if(MSAA)
					return Get(GetVariation<true, true, false, true>());
				else
					return Get(GetVariation<true, true, false, false>());
			}
		}
		else
		{
			if(autoExposure)
			{
				if(MSAA)
					return Get(GetVariation<true, false, true, true>());
				else
					return Get(GetVariation<true, false, true, false>());
			}
			else
			{
				if(MSAA)
					return Get(GetVariation<true, false, false, true>());
				else
					return Get(GetVariation<true, false, false, false>());
			}
		}
	}
	else
	{
		if(gammaOnly)
		{
			if(autoExposure)
			{
				if(MSAA)
					return Get(GetVariation<false, true, true, true>());
				else
					return Get(GetVariation<false, true, true, false>());
			}
			else
			{
				if(MSAA)
					return Get(GetVariation<false, true, false, true>());
				else
					return Get(GetVariation<false, true, false, false>());
			}
		}
		else
		{
			if(autoExposure)
			{
				if(MSAA)
					return Get(GetVariation<false, false, true, true>());
				else
					return Get(GetVariation<false, false, true, false>());
			}
			else
			{
				if(MSAA)
					return Get(GetVariation<false, false, false, true>());
				else
					return Get(GetVariation<false, false, false, false>());
			}
		}
	}
}

BloomClipUniformDefinition gBloomClipUniformDefinition;

void BloomClipMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void BloomClipMaterial::Prepare(const TShared<Texture>& input, float threshold, const TShared<Texture>& eyeAdaptation, const RenderSettings& settings)
{
	GpuBufferMappedScope uniforms = gBloomClipUniformDefinition.AllocateTransient().Map();

	gBloomClipUniformDefinition.gThreshold.Set(uniforms, threshold);
	gBloomClipUniformDefinition.gManualExposureScale.Set(uniforms, Math::RaiseToPower(2.0f, settings.ExposureScale));

	mUniformBufferParameter.Set(uniforms);
	mInputTextureParameter.Set(input);
	mEyeAdaptationTextureParameter.Set(eyeAdaptation);
}

void BloomClipMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BloomClipMaterial* BloomClipMaterial::GetVariation(bool autoExposure)
{
	if(autoExposure)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

ScreenSpaceLensFlareUniformDefinition gScreenSpaceLensFlareUniformDefinition;

void ScreenSpaceLensFlareMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gGradientTex", mGradientTextureParameter);
}

void ScreenSpaceLensFlareMaterial::Prepare(const TShared<Texture>& input, const ScreenSpaceLensFlareSettings& settings)
{
	GpuBufferMappedScope uniforms = gScreenSpaceLensFlareUniformDefinition.AllocateTransient().Map();

	gScreenSpaceLensFlareUniformDefinition.gThreshold.Set(uniforms, settings.Threshold);
	gScreenSpaceLensFlareUniformDefinition.gGhostCount.Set(uniforms, settings.GhostCount);
	gScreenSpaceLensFlareUniformDefinition.gGhostSpacing.Set(uniforms, settings.GhostSpacing);
	gScreenSpaceLensFlareUniformDefinition.gHaloRadius.Set(uniforms, settings.HaloRadius);
	gScreenSpaceLensFlareUniformDefinition.gHaloThickness.Set(uniforms, settings.HaloThickness);
	gScreenSpaceLensFlareUniformDefinition.gHaloThreshold.Set(uniforms, settings.HaloThreshold);
	gScreenSpaceLensFlareUniformDefinition.gHaloAspectRatio.Set(uniforms, settings.HaloAspectRatio);
	gScreenSpaceLensFlareUniformDefinition.gChromaticAberration.Set(uniforms, settings.ChromaticAberrationOffset);

	mUniformBufferParameter.Set(uniforms);

	mInputTextureParameter.Set(input);
	mGradientTextureParameter.Set(RendererTextures::lensFlareGradient);
}

void ScreenSpaceLensFlareMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

ScreenSpaceLensFlareMaterial* ScreenSpaceLensFlareMaterial::GetVariation(bool halo, bool haloAspect, bool chromaticAberration)
{
	if(halo)
	{
		if(haloAspect)
		{
			if(chromaticAberration)
				return Get(GetVariation<1, true>());

			return Get(GetVariation<1, false>());
		}
		else
		{
			if(chromaticAberration)
				return Get(GetVariation<2, true>());

			return Get(GetVariation<2, false>());
		}
	}
	else
	{
		if(chromaticAberration)
			return Get(GetVariation<0, true>());

		return Get(GetVariation<0, false>());
	}
}

ChromaticAberrationUniformDefinition gChromaticAberrationUniformDefinition;

constexpr int ChromaticAberrationMaterial::kMaxSamples;

void ChromaticAberrationMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gFringeTex", mFringeTextureParameter);
}

void ChromaticAberrationMaterial::Prepare(const TShared<Texture>& input, const ChromaticAberrationSettings& settings)
{
	const TextureProperties& texProps = input->GetProperties();

	GpuBufferMappedScope uniforms = gChromaticAberrationUniformDefinition.AllocateTransient().Map();

	gChromaticAberrationUniformDefinition.gInputSize.Set(uniforms, Vector2((float)texProps.Width, (float)texProps.Height));
	gChromaticAberrationUniformDefinition.gShiftAmount.Set(uniforms, settings.ShiftAmount);

	mUniformBufferParameter.Set(uniforms);

	TShared<Texture> fringeTex;
	if(settings.FringeTexture)
		fringeTex = settings.FringeTexture;
	else
		fringeTex = RendererTextures::chromaticAberrationFringe;

	mInputTextureParameter.Set(input);
	mFringeTextureParameter.Set(fringeTex);
}

void ChromaticAberrationMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

ChromaticAberrationMaterial* ChromaticAberrationMaterial::GetVariation(ChromaticAberrationType type)
{
	if(type == ChromaticAberrationType::Complex)
		return Get(GetVariation<false>());

	return Get(GetVariation<true>());
}

void ChromaticAberrationMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_SAMPLES", kMaxSamples);
}

FilmGrainUniformDefinition gFilmGrainUniformDefinition;

void FilmGrainMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void FilmGrainMaterial::Prepare(const TShared<Texture>& input, float time, const FilmGrainSettings& settings)
{
	GpuBufferMappedScope uniforms = gFilmGrainUniformDefinition.AllocateTransient().Map();

	gFilmGrainUniformDefinition.gIntensity.Set(uniforms, settings.Intensity);
	gFilmGrainUniformDefinition.gTime.Set(uniforms, settings.Speed * time);

	mUniformBufferParameter.Set(uniforms);

	mInputTextureParameter.Set(input);
}

void FilmGrainMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

GaussianBlurUniformDefinition gGaussianBlurUniformDefinition;

void GaussianBlurMaterial::Initialize()
{
	mIsAdditive = mVariationParameters.GetBool("ADDITIVE");

	mGpuParameterSet->GetUniformBufferParameter("GaussianBlurParams", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	if(mIsAdditive)
		mGpuParameterSet->GetSampledTextureParameter("gAdditiveTex", mAdditiveTextureParameter);
}

void GaussianBlurMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_NUM_SAMPLES", kMaxBlurSamples);
}

void GaussianBlurMaterial::PrepareDirection(Direction direction, const TShared<Texture>& source, float filterSize, const Color& tint, const TShared<Texture>& additive)
{
	GpuBufferMappedScope uniforms = gGaussianBlurUniformDefinition.AllocateTransient().Map();
	PopulateUniformBuffer(uniforms, direction, source, filterSize, tint);
	mUniformBufferParameter.Set(uniforms);

	mInputTextureParameter.Set(source);

	if(mIsAdditive)
	{
		if(additive)
			mAdditiveTextureParameter.Set(additive);
		else
			mAdditiveTextureParameter.Set(Texture::kBlack);
	}
}

void GaussianBlurMaterial::ExecutePass(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void GaussianBlurMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, float filterSize, const TShared<RenderTexture>& destination, const Color& tint, const TShared<Texture>& additive)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = source->GetProperties();
	const RenderTargetProperties& dstProps = destination->GetProperties();

	PooledRenderTextureCreateInformation tempTextureDesc = PooledRenderTextureCreateInformation::Create2D(srcProps.Format, dstProps.Width, dstProps.Height, TextureUsageFlag::RenderTarget);
	TShared<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTextureDesc);

	// Horizontal pass
	PrepareDirection(DirHorizontal, source, filterSize, Color::kWhite);
	ExecutePass(commandBuffer, tempTexture->RenderTexture);

	// Vertical pass
	PrepareDirection(DirVertical, tempTexture->Texture, filterSize, tint, additive);
	ExecutePass(commandBuffer, destination);
}

u32 GaussianBlurMaterial::CalcStdDistribution(float filterRadius, std::array<float, kMaxBlurSamples>& weights, std::array<float, kMaxBlurSamples>& offsets)
{
	filterRadius = Math::Clamp(filterRadius, 0.00001f, (float)(kMaxBlurSamples - 1));
	i32 intFilterRadius = std::min(Math::CeilToInt(filterRadius), kMaxBlurSamples - 1);

	// Note: Does not include the scaling factor since we normalize later anyway
	auto normalDistribution = [](int i, float scale)
	{
		// Higher value gives more weight to samples near the center
		constexpr float CENTER_BIAS = 30;

		// Mathematica visualization: Manipulate[Plot[E^(-0.5*centerBias*(Abs[x]*(1/radius))^2), {x, -radius, radius}],
		//	{centerBias, 1, 30}, {radius, 1, 72}]
		float samplePos = fabs((float)i) * scale;
		return exp(-0.5f * CENTER_BIAS * samplePos * samplePos);
	};

	// We make use of the hardware linear filtering, and therefore only generate half the number of samples.
	// The weights and the sampling location needs to be adjusted in order to get the same results as if we
	// perform two samples separately:
	//
	// Original formula is: t1*w1 + t2*w2
	// With hardware filtering it's: (t1 + (t2 - t1) * o) * w3
	//	Or expanded: t1*w3 - t1*o*w3 + t2*o*w3 = t1 * (w3 - o*w3) + t2 * (o*w3)
	//
	// These two need to equal, which means this follows:
	// w1 = w3 - o*w3
	// w2 = o*w3
	//
	// From the second equation get the offset o:
	// o = w2/w3
	//
	// From the first equation and o, get w3:
	// w1 = w3 - w2
	// w3 = w1 + w2

	float scale = 1.0f / filterRadius;
	u32 numSamples = 0;
	float totalWeight = 0.0f;
	for(int i = -intFilterRadius; i < intFilterRadius; i += 2)
	{
		float w1 = normalDistribution(i, scale);
		float w2 = normalDistribution(i + 1, scale);

		float w3 = w1 + w2;
		float o = (float)i + w2 / w3; // Relative to first sample

		weights[numSamples] = w3;
		offsets[numSamples] = o;

		numSamples++;
		totalWeight += w3;
	}

	// Special case for last weight, as it doesn't have a matching pair
	float w = normalDistribution(intFilterRadius, scale);
	weights[numSamples] = w;
	offsets[numSamples] = (float)(intFilterRadius - 1);

	numSamples++;
	totalWeight += w;

	// Normalize weights
	float invTotalWeight = 1.0f / totalWeight;
	for(u32 i = 0; i < numSamples; i++)
		weights[i] *= invTotalWeight;

	return numSamples;
}

float GaussianBlurMaterial::CalcKernelRadius(const TShared<Texture>& source, float scale, Direction filterDir)
{
	scale = Math::Clamp01(scale);

	u32 length;
	if(filterDir == DirHorizontal)
		length = source->GetProperties().Width;
	else
		length = source->GetProperties().Height;

	// Divide by two because we need the radius
	return std::min(length * scale / 2, (float)kMaxBlurSamples - 1);
}

void GaussianBlurMaterial::PopulateUniformBuffer(const GpuBufferMappedScope& uniforms, Direction direction, const TShared<Texture>& source, float filterSize, const Color& tint)
{
	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	std::array<float, kMaxBlurSamples> sampleOffsets;
	std::array<float, kMaxBlurSamples> sampleWeights;

	const float kernelRadius = CalcKernelRadius(source, filterSize, direction);
	const u32 numSamples = CalcStdDistribution(kernelRadius, sampleWeights, sampleOffsets);

	for(u32 i = 0; i < numSamples; ++i)
	{
		Vector4 weight(tint.R, tint.G, tint.B, tint.A);
		weight *= sampleWeights[i];

		gGaussianBlurUniformDefinition.gSampleWeights.Set(uniforms, weight, i);
	}

	u32 axis0 = direction == DirHorizontal ? 0 : 1;
	u32 axis1 = (axis0 + 1) % 2;

	for(u32 i = 0; i < (numSamples + 1) / 2; ++i)
	{
		u32 remainder = std::min(2U, numSamples - i * 2);

		Vector4 offset;
		offset[axis0] = sampleOffsets[i * 2 + 0] * invTexSize[axis0];
		offset[axis1] = 0.0f;

		if(remainder == 2)
		{
			offset[axis0 + 2] = sampleOffsets[i * 2 + 1] * invTexSize[axis0];
			offset[axis1 + 2] = 0.0f;
		}
		else
		{
			offset[axis0 + 2] = 0.0f;
			offset[axis1 + 2] = 0.0f;
		}

		gGaussianBlurUniformDefinition.gSampleOffsets.Set(uniforms, offset, i);
	}

	gGaussianBlurUniformDefinition.gNumSamples.Set(uniforms, numSamples);
}

GaussianBlurMaterial* GaussianBlurMaterial::GetVariation(bool additive)
{
	if(additive)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

GaussianDOFUniformDefinition gGaussianDOFUniformDefinition;

void GaussianDOFSeparateMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gColorTex", mColorTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;
	samplerStateCreateInformation.AddressMode.U = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.V = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
	SetSamplerState(mGpuParameterSet, "gColorSamp", "gColorTex", samplerState);
}

void GaussianDOFSeparateMaterial::Prepare(const TShared<Texture>& color, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = color->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gGaussianDOFUniformDefinition.AllocateTransient().Map();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniforms, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniforms, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniforms, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniforms, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniforms, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(uniforms);

	mColorTextureParameter.Set(color);
	mDepthTextureParameter.Set(depth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void GaussianDOFSeparateMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& color)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = color->GetProperties();

	u32 outputWidth = std::max(1U, srcProps.Width / 2);
	u32 outputHeight = std::max(1U, srcProps.Height / 2);

	PooledRenderTextureCreateInformation outputTexDesc = PooledRenderTextureCreateInformation::Create2D(srcProps.Format, outputWidth, outputHeight, TextureUsageFlag::RenderTarget);
	mOutput0 = GetGpuResourcePool().Get(outputTexDesc);

	bool near = mVariationParameters.GetBool("NEAR");
	bool far = mVariationParameters.GetBool("FAR");

	TShared<RenderTexture> rt;
	if(near && far)
	{
		mOutput1 = GetGpuResourcePool().Get(outputTexDesc);

		RenderTextureCreateInformation rtDesc;
		rtDesc.ColorSurfaces[0].Texture = mOutput0->Texture;
		rtDesc.ColorSurfaces[1].Texture = mOutput1->Texture;

		rt = RenderTexture::Create(rtDesc);
	}
	else
		rt = mOutput0->RenderTexture;

	RenderPassCreateInformation info(rt, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

TShared<PooledRenderTexture> GaussianDOFSeparateMaterial::GetOutput(u32 idx)
{
	if(idx == 0)
		return mOutput0;
	else if(idx == 1)
		return mOutput1;

	return nullptr;
}

void GaussianDOFSeparateMaterial::Release()
{
	mOutput0 = nullptr;
	mOutput1 = nullptr;
}

GaussianDOFSeparateMaterial* GaussianDOFSeparateMaterial::GetVariation(bool near, bool far)
{
	if(near)
	{
		if(far)
			return Get(GetVariation<true, true>());
		else
			return Get(GetVariation<true, false>());
	}
	else
		return Get(GetVariation<false, true>());
}

void GaussianDOFCombineMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);

	mGpuParameterSet->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	if(mGpuParameterSet->HasSampledTexture("gNearTex"))
		mGpuParameterSet->GetSampledTextureParameter("gNearTex", mNearTextureParameter);

	if(mGpuParameterSet->HasSampledTexture("gFarTex"))
		mGpuParameterSet->GetSampledTextureParameter("gFarTex", mFarTextureParameter);
}

void GaussianDOFCombineMaterial::Prepare(const TShared<Texture>& focused, const TShared<Texture>& near, const TShared<Texture>& far, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = focused->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gGaussianDOFUniformDefinition.AllocateTransient().Map();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniforms, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniforms, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniforms, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniforms, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniforms, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(uniforms);

	mFocusedTextureParameter.Set(focused);
	mNearTextureParameter.Set(near);
	mFarTextureParameter.Set(far);
	mDepthTextureParameter.Set(depth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void GaussianDOFCombineMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

GaussianDOFCombineMaterial* GaussianDOFCombineMaterial::GetVariation(bool near, bool far)
{
	if(near)
	{
		if(far)
			return Get(GetVariation<true, true>());
		else
			return Get(GetVariation<true, false>());
	}
	else
		return Get(GetVariation<false, true>());
}

DepthOfFieldCommonUniformDefinition gDepthOfFieldCommonUniformDefinition;
BokehDOFPrepareUniformDefinition gBokehDOFPrepareUniformDefinition;

void BokehDOFPrepareMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

void BokehDOFPrepareMaterial::Prepare(const TShared<Texture>& input, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = input->GetProperties();

	GpuBufferMappedScope prepareUniforms = gBokehDOFPrepareUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope commonUniforms = gDepthOfFieldCommonUniformDefinition.AllocateTransient().Map();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gBokehDOFPrepareUniformDefinition.gInvInputSize.Set(prepareUniforms, invTexSize);

	BokehDOFMaterial::PopulateDofCommonParams(commonUniforms, settings, view);

	mUniformBufferParameter.Set(prepareUniforms);
	mCommonUniformBufferParameter.Set(commonUniforms);
	mInputTextureParameter.Set(input);
	mDepthTextureParameter.Set(depth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFPrepareMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);

	bool MSAA = mVariationParameters.GetI32("MSAA_COUNT") > 1;
	if(MSAA)
	{
		const TextureProperties& srcProps = mInputTextureParameter.Get()->GetProperties();
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)srcProps.Width, (float)srcProps.Height));
	}
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation BokehDOFPrepareMaterial::GetOutputDesc(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.Width, 2U));
	u32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.Height, 2U));

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget);
}

BokehDOFPrepareMaterial* BokehDOFPrepareMaterial::GetVariation(bool msaa)
{
	if(msaa)
		return Get(GetVariation<true>());
	else
		return Get(GetVariation<false>());
}

BokehDOFUniformDefinition gBokehDOFUniformDefinition;

constexpr u32 BokehDOFMaterial::kNearFarPadding;
constexpr u32 BokehDOFMaterial::kQuadsPerTile;

void BokehDOFMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureVSParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureFSParameter);
	mGpuParameterSet->GetSampledTextureParameter("gBokehTex", mBokehTextureParameter);

	// Prepare vertex declaration for rendering tiles
	TInlineArray<VertexElement, 8> tileVertexElements;
	tileVertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

	mTileVertexDescription = B3DMakeShared<VertexDescription>(tileVertexElements);

	// Prepare vertex buffer for rendering tiles
	GpuBufferCreateInformation tileVertexBufferCreateInformation;
	tileVertexBufferCreateInformation.Type = GpuBufferType::Vertex;
	tileVertexBufferCreateInformation.Vertex.Count = kQuadsPerTile * 4;
	tileVertexBufferCreateInformation.Vertex.ElementSize = mTileVertexDescription->GetVertexStride();

	mTileVertexBuffer = mGpuDevice->CreateGpuBuffer(tileVertexBufferCreateInformation);

	auto* const vertexData = (Vector2*)B3DStackAllocate(mTileVertexBuffer->GetTotalSize());
	for(u32 i = 0; i < kQuadsPerTile; i++)
	{
		vertexData[i * 4 + 0] = Vector2(0.0f, 0.0f);
		vertexData[i * 4 + 1] = Vector2(1.0f, 0.0f);
		vertexData[i * 4 + 2] = Vector2(0.0f, 1.0f);
		vertexData[i * 4 + 3] = Vector2(1.0f, 1.0f);
	}

	GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();
	GpuBufferUtility::Write(gpuContext, mTileVertexBuffer, 0, mTileVertexBuffer->GetTotalSize(), vertexData);
	B3DStackFree(vertexData);

	// Prepare indices for rendering tiles
	GpuBufferCreateInformation tileIndexBufferCreateInformation;
	tileIndexBufferCreateInformation.Type = GpuBufferType::Index;
	tileIndexBufferCreateInformation.Index.Type = IT_16BIT;
	tileIndexBufferCreateInformation.Index.Count = kQuadsPerTile * 6;

	mTileIndexBuffer = mGpuDevice->CreateGpuBuffer(tileIndexBufferCreateInformation);

	auto* const indices = (u16*)B3DStackAllocate(mTileIndexBuffer->GetTotalSize());

	const GpuBackendConventions& gpuBackendConventions = mGpuDevice->GetCapabilities().Conventions;
	for(u32 i = 0; i < kQuadsPerTile; i++)
	{
		// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
		// get culled.
		if(gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up)
		{
			indices[i * 6 + 0] = i * 4 + 2;
			indices[i * 6 + 1] = i * 4 + 1;
			indices[i * 6 + 2] = i * 4 + 0;
			indices[i * 6 + 3] = i * 4 + 2;
			indices[i * 6 + 4] = i * 4 + 3;
			indices[i * 6 + 5] = i * 4 + 1;
		}
		else
		{
			indices[i * 6 + 0] = i * 4 + 0;
			indices[i * 6 + 1] = i * 4 + 1;
			indices[i * 6 + 2] = i * 4 + 2;
			indices[i * 6 + 3] = i * 4 + 1;
			indices[i * 6 + 4] = i * 4 + 3;
			indices[i * 6 + 5] = i * 4 + 2;
		}
	}

	GpuBufferUtility::Write(gpuContext, mTileIndexBuffer, 0, mTileIndexBuffer->GetTotalSize(), indices);
	B3DStackFree(indices);
}

void BokehDOFMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("QUADS_PER_TILE", kQuadsPerTile);
}

void BokehDOFMaterial::Prepare(const TShared<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const TShared<RenderTarget>& output)
{
	const TextureProperties& srcProps = input->GetProperties();
	const RenderTargetProperties& dstProps = output->GetProperties();

	GpuBufferMappedScope bokehUniforms = gBokehDOFUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope depthOfFieldCommonUniforms = gDepthOfFieldCommonUniformDefinition.AllocateTransient().Map();

	Vector2 inputInvTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	Vector2 outputInvTexSize(1.0f / dstProps.Width, 1.0f / dstProps.Height);
	gBokehDOFUniformDefinition.gInvInputSize.Set(bokehUniforms, inputInvTexSize);
	gBokehDOFUniformDefinition.gInvOutputSize.Set(bokehUniforms, outputInvTexSize);
	gBokehDOFUniformDefinition.gAdaptiveThresholdCOC.Set(bokehUniforms, settings.AdaptiveRadiusThreshold);
	gBokehDOFUniformDefinition.gAdaptiveThresholdColor.Set(bokehUniforms, settings.AdaptiveColorThreshold);
	gBokehDOFUniformDefinition.gLayerPixelOffset.Set(bokehUniforms, (i32)srcProps.Height + (i32)kNearFarPadding);
	gBokehDOFUniformDefinition.gInvDepthRange.Set(bokehUniforms, 1.0f / settings.OcclusionDepthRange);

	float bokehSize = settings.MaxBokehSize * srcProps.Width;
	gBokehDOFUniformDefinition.gBokehSize.Set(bokehUniforms, Vector2(bokehSize, bokehSize));

	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	Vector2I tileCount = imageSize / 1;
	gBokehDOFUniformDefinition.gTileCount.Set(bokehUniforms, tileCount);

	PopulateDofCommonParams(depthOfFieldCommonUniforms, settings, view);

	mUniformBufferParameter.Set(bokehUniforms);
	mCommonUniformBufferParameter.Set(depthOfFieldCommonUniforms);
	mInputTextureVSParameter.Set(input);
	mInputTextureFSParameter.Set(input);

	TShared<Texture> bokehTexture = settings.BokehShape;
	if(bokehTexture == nullptr)
		bokehTexture = RendererTextures::bokehFlare;

	mBokehTextureParameter.Set(bokehTexture);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& input, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = input->GetProperties();
	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	const Vector2I tileCount = imageSize / 1;

	RenderPassCreateInformation info(output, mGpuParameterSet, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
	info.ClearMask = RT_COLOR_ALL;
	info.ClearColor = Color::kZero;

	commandBuffer.BeginRenderPass(info);
	commandBuffer.SetVertexDescription(mTileVertexDescription);

	TShared<GpuBuffer> buffers[] = { mTileVertexBuffer };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(mTileIndexBuffer);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	Bind(commandBuffer);
	const u32 numInstances = Math::DivideAndRoundUp((u32)(tileCount.X * tileCount.Y), kQuadsPerTile);
	commandBuffer.DrawIndexed(0, kQuadsPerTile * 6, 0, kQuadsPerTile * 4, numInstances);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation BokehDOFMaterial::GetOutputDesc(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = rtProps.Width;
	u32 height = rtProps.Height * 2 + kNearFarPadding;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget);
}

void BokehDOFMaterial::PopulateDofCommonParams(const GpuBufferMappedScope& uniforms, const DepthOfFieldSettings& settings, const RendererView& view)
{
	gDepthOfFieldCommonUniformDefinition.gFocalPlaneDistance.Set(uniforms, settings.FocalDistance);
	gDepthOfFieldCommonUniformDefinition.gApertureSize.Set(uniforms, settings.ApertureSize * 0.001f); // mm to m
	gDepthOfFieldCommonUniformDefinition.gFocalLength.Set(uniforms, settings.FocalLength * 0.001f); // mm to m
	gDepthOfFieldCommonUniformDefinition.gInFocusRange.Set(uniforms, settings.FocalRange);
	gDepthOfFieldCommonUniformDefinition.gNearTransitionRegion.Set(uniforms, settings.NearTransitionRange);
	gDepthOfFieldCommonUniformDefinition.gFarTransitionRegion.Set(uniforms, settings.FarTransitionRange);

	float sensorSize, imageSize;
	if(settings.SensorSize.X < settings.SensorSize.Y)
	{
		sensorSize = settings.SensorSize.X;
		imageSize = (float)view.GetProperties().Target.TargetWidth;
	}
	else
	{
		sensorSize = settings.SensorSize.Y;
		imageSize = (float)view.GetProperties().Target.TargetHeight;
	}

	gDepthOfFieldCommonUniformDefinition.gSensorSize.Set(uniforms, sensorSize);
	gDepthOfFieldCommonUniformDefinition.gImageSize.Set(uniforms, imageSize);
	gDepthOfFieldCommonUniformDefinition.gMaxBokehSize.Set(uniforms, Math::Clamp01(settings.MaxBokehSize) * imageSize);
}

BokehDOFMaterial* BokehDOFMaterial::GetVariation(bool depthOcclusion)
{
	if(depthOcclusion)
		return Get(GetVariation<true>());
	else
		return Get(GetVariation<false>());
}

BokehDOFCombineUniformDefinition gBokehDOFCombineUniformDefinition;

void BokehDOFCombineMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gUnfocusedTex", mUnfocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

void BokehDOFCombineMaterial::Prepare(const TShared<Texture>& unfocused, const TShared<Texture>& focused, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& focusedProps = focused->GetProperties();
	const TextureProperties& unfocusedProps = unfocused->GetProperties();
	u32 halfHeight = std::max(1U, Math::DivideAndRoundUp(focusedProps.Height, 2U));

	GpuBufferMappedScope bokehCombineUniforms = gBokehDOFCombineUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope depthOfFieldCommonUniforms = gDepthOfFieldCommonUniformDefinition.AllocateTransient().Map();

	float uvScale = halfHeight / (float)unfocusedProps.Height;
	float uvOffset = (halfHeight + BokehDOFMaterial::kNearFarPadding) / (float)unfocusedProps.Height;

	Vector2 layerScaleOffset(uvScale, uvOffset);
	Vector2 focusedImageSize((float)focusedProps.Width, (float)focusedProps.Height);
	gBokehDOFCombineUniformDefinition.gLayerAndScaleOffset.Set(bokehCombineUniforms, layerScaleOffset);
	gBokehDOFCombineUniformDefinition.gFocusedImageSize.Set(bokehCombineUniforms, focusedImageSize);

	BokehDOFMaterial::PopulateDofCommonParams(depthOfFieldCommonUniforms, settings, view);

	mUniformBufferParameter.Set(bokehCombineUniforms);
	mCommonUniformBufferParameter.Set(depthOfFieldCommonUniforms);
	mUnfocusedTextureParameter.Set(unfocused);
	mFocusedTextureParameter.Set(focused);
	mDepthTextureParameter.Set(depth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFCombineMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BokehDOFCombineMaterial* BokehDOFCombineMaterial::GetVariation(MSAAMode msaaMode)
{
	switch(msaaMode)
	{
	default:
	case MSAAMode::None:
		return Get(GetVariation<MSAAMode::None>());
	case MSAAMode::Single:
		return Get(GetVariation<MSAAMode::Single>());
	case MSAAMode::Full:
		return Get(GetVariation<MSAAMode::Full>());
	}
}

MotionBlurUniformDefinition gMotionBlurUniformDefinition;

void MotionBlurMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTexture);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthTexture);

	SamplerStateInformation pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> pointSampState = mGpuDevice->FindOrCreateSamplerState(pointSampDesc);

	if(mGpuParameterSet->HasSamplerState("gDepthBufferSamp"))
		mGpuParameterSet->SetSamplerState("gDepthBufferSamp", pointSampState);
}

void MotionBlurMaterial::Prepare(const TShared<Texture>& input, const TShared<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings)
{
	u32 numSamples;
	switch(settings.Quality)
	{
	default:
	case MotionBlurQuality::VeryLow: numSamples = 4; break;
	case MotionBlurQuality::Low: numSamples = 6; break;
	case MotionBlurQuality::Medium: numSamples = 8; break;
	case MotionBlurQuality::High: numSamples = 12; break;
	case MotionBlurQuality::Ultra: numSamples = 16; break;
	}

	GpuBufferMappedScope uniforms = gMotionBlurUniformDefinition.AllocateTransient().Map();
	gMotionBlurUniformDefinition.gHalfNumSamples.Set(uniforms, numSamples / 2);

	mUniformBufferParameter.Set(uniforms);
	mInputTexture.Set(input);
	mDepthTexture.Set(depth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void MotionBlurMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BuildHiZUniformDefinition gBuildHiZUniformDefinition;

void BuildHiZMaterial::Initialize()
{
	mNoTextureViews = mVariationParameters.GetBool("NO_TEXTURE_VIEWS");

	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mInputTexture);

	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);

		SamplerStateInformation inputSampDesc;
		inputSampDesc.MinFilter = FO_POINT;
		inputSampDesc.MagFilter = FO_POINT;
		inputSampDesc.MipFilter = FO_POINT;

		TShared<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
		SetSamplerState(mGpuParameterSet, "gDepthSamp", "gDepthTex", inputSampState);
	}
}

void BuildHiZMaterial::Prepare(const TShared<Texture>& source, u32 srcMip)
{
	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mInputTexture.Set(source);

		auto& props = source->GetProperties();
		float pixelWidth = (float)props.Width;
		float pixelHeight = (float)props.Height;

		Vector2 halfPixelOffset(0.5f / pixelWidth, 0.5f / pixelHeight);

		GpuBufferMappedScope uniforms = gBuildHiZUniformDefinition.AllocateTransient().Map();
		gBuildHiZUniformDefinition.gHalfPixelOffset.Set(uniforms, halfPixelOffset);
		gBuildHiZUniformDefinition.gMipLevel.Set(uniforms, srcMip);

		mUniformBufferParameter.Set(uniforms);
	}
	else
		mInputTexture.Set(source, TextureSurface(srcMip));
}

void BuildHiZMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTexture>& output, const Area2& srcRect, const Area2& dstRect)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	commandBuffer.SetViewport(dstRect);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, srcRect);

	commandBuffer.SetViewport(Area2(0, 0, 1, 1));
	commandBuffer.EndRenderPass();
}

BuildHiZMaterial* BuildHiZMaterial::GetVariation(bool noTextureViews)
{
	if(noTextureViews)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

FXAAUniformDefinition gFXAAUniformDefinition;

void FXAAMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void FXAAMaterial::Prepare(const TShared<Texture>& source)
{
	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gFXAAUniformDefinition.AllocateTransient().Map();
	gFXAAUniformDefinition.gInvTexSize.Set(uniforms, invTexSize);

	mUniformBufferParameter.Set(uniforms);
	mInputTexture.Set(source);
}

void FXAAMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOUniformDefinition gSSAOUniformDefinition;

void SSAOMaterial::Initialize()
{
	bool isFinal = mVariationParameters.GetBool("FINAL_AO");
	bool mixWithUpsampled = mVariationParameters.GetBool("MIX_WITH_UPSAMPLED");

	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);

	if(isFinal)
	{
		mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTexture);
		mGpuParameterSet->GetSampledTextureParameter("gNormalsTex", mNormalsTexture);
	}

	if(!isFinal || mixWithUpsampled)
		mGpuParameterSet->GetSampledTextureParameter("gSetupAO", mSetupAOTexture);

	if(mixWithUpsampled)
		mGpuParameterSet->GetSampledTextureParameter("gDownsampledAO", mDownsampledAOTexture);

	mGpuParameterSet->GetSampledTextureParameter("gRandomTex", mRandomTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		mGpuParameterSet->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		if(isFinal)
		{
			mGpuParameterSet->SetSamplerState("gDepthTex", inputSampState);
			mGpuParameterSet->SetSamplerState("gNormalsTex", inputSampState);
		}

		if(!isFinal || mixWithUpsampled)
			mGpuParameterSet->SetSamplerState("gSetupAO", inputSampState);

		if(mixWithUpsampled)
			mGpuParameterSet->SetSamplerState("gDownsampledAO", inputSampState);
	}

	SamplerStateInformation randomSampDesc;
	randomSampDesc.MinFilter = FO_POINT;
	randomSampDesc.MagFilter = FO_POINT;
	randomSampDesc.MipFilter = FO_POINT;
	randomSampDesc.AddressMode.U = TAM_WRAP;
	randomSampDesc.AddressMode.V = TAM_WRAP;
	randomSampDesc.AddressMode.W = TAM_WRAP;

	TShared<SamplerState> randomSampState = mGpuDevice->FindOrCreateSamplerState(randomSampDesc);
	SetSamplerState(mGpuParameterSet, "gRandomSamp", "gRandomTex", randomSampState);
}

void SSAOMaterial::Prepare(const RendererView& view, const SSAOTextureInputs& textures, const TShared<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
{
	// Scale that can be used to adjust how quickly does AO radius increase with downsampled AO. This yields a very
	// small AO radius at highest level, and very large radius at lowest level
	static const float kDownsampleScale = 4.0f;

	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderTargetProperties& rtProps = destination->GetProperties();

	Vector2 tanHalfFOV;
	tanHalfFOV.X = 1.0f / viewProps.ProjTransform[0][0];
	tanHalfFOV.Y = 1.0f / viewProps.ProjTransform[1][1];

	float cotHalfFOV = viewProps.ProjTransform[0][0];

	// Downsampled AO uses a larger AO radius (in higher resolutions this would cause too much cache trashing). This
	// means if only full res AO is used, then only AO from nearby geometry will be calculated.
	float viewScale = viewProps.Target.ViewRect.Width / (float)rtProps.Width;

	// Ramp up the radius exponentially. c^log2(x) function chosen arbitrarily, as it ramps up the radius in a nice way
	float scale = pow(kDownsampleScale, Math::Log2(viewScale));

	// Determine maximum radius scale (division by 4 because we don't downsample more than quarter-size)
	float maxScale = pow(kDownsampleScale, Math::Log2(4.0f));

	// Normalize the scale in [0, 1] range
	scale /= maxScale;

	float radius = settings.Radius * scale;

	// Factors used for scaling the AO contribution with range
	Vector2 fadeMultiplyAdd;
	fadeMultiplyAdd.X = 1.0f / settings.FadeRange;
	fadeMultiplyAdd.Y = -settings.FadeDistance / settings.FadeRange;

	GpuBufferMappedScope uniforms = gSSAOUniformDefinition.AllocateTransient().Map();

	gSSAOUniformDefinition.gSampleRadius.Set(uniforms, radius);
	gSSAOUniformDefinition.gCotHalfFOV.Set(uniforms, cotHalfFOV);
	gSSAOUniformDefinition.gTanHalfFOV.Set(uniforms, tanHalfFOV);
	gSSAOUniformDefinition.gWorldSpaceRadiusMask.Set(uniforms, 1.0f);
	gSSAOUniformDefinition.gBias.Set(uniforms, (settings.Bias * viewScale) / 1000.0f);
	gSSAOUniformDefinition.gFadeMultiplyAdd.Set(uniforms, fadeMultiplyAdd);
	gSSAOUniformDefinition.gPower.Set(uniforms, settings.Power);
	gSSAOUniformDefinition.gIntensity.Set(uniforms, settings.Intensity);

	bool upsample = mVariationParameters.GetBool("MIX_WITH_UPSAMPLED");
	if(upsample)
	{
		const TextureProperties& props = textures.AoDownsampled->GetProperties();

		Vector2 downsampledPixelSize;
		downsampledPixelSize.X = 1.0f / props.Width;
		downsampledPixelSize.Y = 1.0f / props.Height;

		gSSAOUniformDefinition.gDownsampledPixelSize.Set(uniforms, downsampledPixelSize);
	}

	// Generate a scale which we need to use in order to achieve tiling
	const TextureProperties& rndProps = textures.RandomRotations->GetProperties();
	u32 rndWidth = rndProps.Width;
	u32 rndHeight = rndProps.Height;

	//// Multiple of random texture size, rounded up
	u32 scaleWidth = (rtProps.Width + rndWidth - 1) / rndWidth;
	u32 scaleHeight = (rtProps.Height + rndHeight - 1) / rndHeight;

	Vector2 randomTileScale((float)scaleWidth, (float)scaleHeight);
	gSSAOUniformDefinition.gRandomTileScale.Set(uniforms, randomTileScale);

	mUniformBufferParameter.Set(uniforms);

	mSetupAOTexture.Set(textures.AoSetup);

	bool finalPass = mVariationParameters.GetBool("FINAL_AO");
	if(finalPass)
	{
		mDepthTexture.Set(textures.SceneDepth);
		mNormalsTexture.Set(textures.SceneNormals);
	}

	if(upsample)
		mDownsampledAOTexture.Set(textures.AoDownsampled);

	mRandomTexture.Set(textures.RandomRotations);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void SSAOMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOMaterial* SSAOMaterial::GetVariation(bool upsample, bool finalPass, int quality)
{
#define PICK_MATERIAL(QUALITY)                                \
	if(upsample)                                              \
		if(finalPass)                                         \
			return Get(GetVariation<true, true, QUALITY>());  \
		else                                                  \
			return Get(GetVariation<true, false, QUALITY>()); \
	else if(finalPass)                                        \
		return Get(GetVariation<false, true, QUALITY>());     \
	else                                                      \
		return Get(GetVariation<false, false, QUALITY>());

	switch(quality)
	{
	case 0:
		PICK_MATERIAL(0)
	case 1:
		PICK_MATERIAL(1)
	case 2:
		PICK_MATERIAL(2)
	case 3:
		PICK_MATERIAL(3)
	default:
	case 4:
		PICK_MATERIAL(4)
	}

#undef PICK_MATERIAL
}

SSAODownsampleUniformDefinition gSSAODownsampleUniformDefinition;

void SSAODownsampleMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTexture);
	mGpuParameterSet->GetSampledTextureParameter("gNormalsTex", mNormalsTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_LINEAR;
	inputSampDesc.MagFilter = FO_LINEAR;
	inputSampDesc.MipFilter = FO_LINEAR;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);

	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		mGpuParameterSet->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		mGpuParameterSet->SetSamplerState("gDepthTex", inputSampState);
		mGpuParameterSet->SetSamplerState("gNormalsTex", inputSampState);
	}
}

void SSAODownsampleMaterial::Prepare(const RendererView& view, const TShared<Texture>& depth, const TShared<Texture>& normals, const TShared<RenderTexture>& destination, float depthRange)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderTargetProperties& rtProps = destination->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / rtProps.Width;
	pixelSize.Y = 1.0f / rtProps.Height;

	float scale = viewProps.Target.ViewRect.Width / (float)rtProps.Width;

	GpuBufferMappedScope uniforms = gSSAODownsampleUniformDefinition.AllocateTransient().Map();
	gSSAODownsampleUniformDefinition.gPixelSize.Set(uniforms, pixelSize);
	gSSAODownsampleUniformDefinition.gInvDepthThreshold.Set(uniforms, (1.0f / depthRange) / scale);

	mUniformBufferParameter.Set(uniforms);
	mDepthTexture.Set(depth);
	mNormalsTexture.Set(normals);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void SSAODownsampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOBlurUniformDefinition gSSAOBlurUniformDefinition;

void SSAOBlurMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mAOTexture);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		mGpuParameterSet->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		mGpuParameterSet->SetSamplerState("gInputTex", inputSampState);
		mGpuParameterSet->SetSamplerState("gDepthTex", inputSampState);
	}
}

void SSAOBlurMaterial::Prepare(const RendererView& view, const TShared<Texture>& ao, const TShared<Texture>& sceneDepth, float depthRange)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& texProps = ao->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / texProps.Width;
	pixelSize.Y = 1.0f / texProps.Height;

	Vector2 pixelOffset(kZeroTag);
	if(mVariationParameters.GetBool("DIR_HORZ"))
		pixelOffset.X = pixelSize.X;
	else
		pixelOffset.Y = pixelSize.Y;

	float scale = viewProps.Target.ViewRect.Width / (float)texProps.Width;

	GpuBufferMappedScope uniforms = gSSAOBlurUniformDefinition.AllocateTransient().Map();
	gSSAOBlurUniformDefinition.gPixelSize.Set(uniforms, pixelSize);
	gSSAOBlurUniformDefinition.gPixelOffset.Set(uniforms, pixelOffset);
	gSSAOBlurUniformDefinition.gInvDepthThreshold.Set(uniforms, (1.0f / depthRange) / scale);

	mUniformBufferParameter.Set(uniforms);
	mAOTexture.Set(ao);
	mDepthTexture.Set(sceneDepth);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void SSAOBlurMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOBlurMaterial* SSAOBlurMaterial::GetVariation(bool horizontal)
{
	if(horizontal)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

SSRStencilUniformDefinition gSSRStencilUniformDefinition;

void SSRStencilMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
}

void SSRStencilMaterial::Prepare(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings)
{
	mGBufferParams.Bind(gbuffer);

	Vector2 roughnessScaleBias = SSRTraceMaterial::CalcRoughnessFadeScaleBias(settings.MaxRoughness);

	GpuBufferMappedScope uniforms = gSSRStencilUniformDefinition.AllocateTransient().Map();
	gSSRStencilUniformDefinition.gRoughnessScaleBias.Set(uniforms, roughnessScaleBias);
	mUniformBufferParameter.Set(uniforms);

	// Not all variations of the shader reference PerCamera, and backends whose shader reflection strips unused blocks
	// (e.g. DirectX) then lack it in the layout
	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->TrySetUniformBuffer("PerCamera", perView);
}

void SSRStencilMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer);

	const RendererViewProperties& viewProps = view.GetProperties();
	Area2I viewRect = viewProps.Target.ViewRect;

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

SSRStencilMaterial* SSRStencilMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
		return Get(GetVariation<false, false>());
}

SSRTraceUniformDefinition gSSRTraceUniformDefinition;

void SSRTraceMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);

	mGpuParameterSet->TryGetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSceneColor", mSceneColorTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gHiZ", mHiZTextureParameter);

	SamplerStateInformation desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> hiZSamplerState = mGpuDevice->FindOrCreateSamplerState(desc);
	if(mGpuParameterSet->HasSamplerState("gHiZSamp"))
		mGpuParameterSet->SetSamplerState("gHiZSamp", hiZSamplerState);
	else if(mGpuParameterSet->HasSamplerState("gHiZ"))
		mGpuParameterSet->SetSamplerState("gHiZ", hiZSamplerState);
}

void SSRTraceMaterial::Prepare(const RendererView& view, GBufferTextures gbuffer, const TShared<Texture>& sceneColor, const TShared<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& hiZProps = hiZ->GetProperties();

	mGBufferParams.Bind(gbuffer);
	mSceneColorTextureParameter.Set(sceneColor);
	mHiZTextureParameter.Set(hiZ);

	Area2I viewRect = viewProps.Target.ViewRect;

	// Maps from NDC to UV [0, 1]
	Vector4 ndcToHiZUV;
	ndcToHiZUV.X = 0.5f;
	ndcToHiZUV.Y = -0.5f;
	ndcToHiZUV.Z = 0.5f;
	ndcToHiZUV.W = 0.5f;

	// Either of these flips the Y axis, but if they're both true they cancel out
	const GpuBackendConventions& gpuBackendConventions = mGpuDevice->GetCapabilities().Conventions;

	if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
		ndcToHiZUV.Y = -ndcToHiZUV.Y;

	// Maps from [0, 1] to area of HiZ where depth is stored in
	ndcToHiZUV.X *= (float)viewRect.Width / hiZProps.Width;
	ndcToHiZUV.Y *= (float)viewRect.Height / hiZProps.Height;
	ndcToHiZUV.Z *= (float)viewRect.Width / hiZProps.Width;
	ndcToHiZUV.W *= (float)viewRect.Height / hiZProps.Height;

	// Maps from HiZ UV to [0, 1] UV
	Vector2 HiZUVToScreenUV;
	HiZUVToScreenUV.X = hiZProps.Width / (float)viewRect.Width;
	HiZUVToScreenUV.Y = hiZProps.Height / (float)viewRect.Height;

	// Used for roughness fading
	Vector2 roughnessScaleBias = CalcRoughnessFadeScaleBias(settings.MaxRoughness);

	u32 temporalJitter = (viewProps.FrameIdx % 8) * 1503;

	GpuBufferMappedScope uniforms = gSSRTraceUniformDefinition.AllocateTransient().Map();

	Vector2I bufferSize(viewRect.Width, viewRect.Height);
	gSSRTraceUniformDefinition.gHiZSize.Set(uniforms, bufferSize);
	gSSRTraceUniformDefinition.gHiZNumMips.Set(uniforms, hiZProps.MipMapCount);
	gSSRTraceUniformDefinition.gNDCToHiZUV.Set(uniforms, ndcToHiZUV);
	gSSRTraceUniformDefinition.gHiZUVToScreenUV.Set(uniforms, HiZUVToScreenUV);
	gSSRTraceUniformDefinition.gIntensity.Set(uniforms, settings.Intensity);
	gSSRTraceUniformDefinition.gRoughnessScaleBias.Set(uniforms, roughnessScaleBias);
	gSSRTraceUniformDefinition.gTemporalJitter.Set(uniforms, temporalJitter);

	mUniformBufferParameter.Set(uniforms);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void SSRTraceMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& destination, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGpuParameterSet, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
	info.ClearMask = RT_COLOR_ALL;
	info.ClearColor = Color::kZero;

	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);

	const RendererViewProperties& viewProps = view.GetProperties();
	if(viewProps.Target.NumSamples > 1)
	{
		const Area2I& viewRect = viewProps.Target.ViewRect;
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	}
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

Vector2 SSRTraceMaterial::CalcRoughnessFadeScaleBias(float maxRoughness)
{
	const static float kRangeScale = 2.0f;

	Vector2 scaleBias;
	scaleBias.X = -kRangeScale / (-1.0f + maxRoughness);
	scaleBias.Y = (kRangeScale * maxRoughness) / (-1.0f + maxRoughness);

	return scaleBias;
}

SSRTraceMaterial* SSRTraceMaterial::GetVariation(u32 quality, bool msaa, bool singleSampleMSAA)
{
#define PICK_MATERIAL(QUALITY)                                \
	if(msaa)                                                  \
		if(singleSampleMSAA)                                  \
			return Get(GetVariation<QUALITY, true, true>());  \
		else                                                  \
			return Get(GetVariation<QUALITY, true, false>()); \
	else                                                      \
		return Get(GetVariation<QUALITY, false, false>());

	switch(quality)
	{
	case 0:
		PICK_MATERIAL(0)
	case 1:
		PICK_MATERIAL(1)
	case 2:
		PICK_MATERIAL(2)
	case 3:
		PICK_MATERIAL(3)
	default:
	case 4:
		PICK_MATERIAL(4)
	}

#undef PICK_MATERIAL
}

TemporalResolveUniformDefinition gTemporalResolveUniformDefinition;
TemporalFilteringUniformDefinition gTemporalFilteringUniformDefinition;

void TemporalFilteringMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("TemporalInput", mTemporalUniformBufferParameter);

	mGpuParameterSet->GetSampledTextureParameter("gSceneDepth", mSceneDepthTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSceneColor", mSceneColorTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gPrevColor", mPreviousColorTextureParameter);

	mHasVelocityTexture = mVariationParameters.GetBool("PER_PIXEL_VELOCITY");
	if(mHasVelocityTexture)
		mGpuParameterSet->GetSampledTextureParameter("gVelocity", mVelocityTextureParameter);

	SamplerStateInformation pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> pointSampState = mGpuDevice->FindOrCreateSamplerState(pointSampDesc);

	if(mGpuParameterSet->HasSamplerState("gPointSampler"))
		mGpuParameterSet->SetSamplerState("gPointSampler", pointSampState);
	else
		mGpuParameterSet->SetSamplerState("gSceneDepth", pointSampState);

	SamplerStateInformation linearSampDesc;
	linearSampDesc.MinFilter = FO_POINT;
	linearSampDesc.MagFilter = FO_POINT;
	linearSampDesc.MipFilter = FO_POINT;
	linearSampDesc.AddressMode.U = TAM_CLAMP;
	linearSampDesc.AddressMode.V = TAM_CLAMP;
	linearSampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> linearSampState = mGpuDevice->FindOrCreateSamplerState(linearSampDesc);
	if(mGpuParameterSet->HasSamplerState("gLinearSampler"))
		mGpuParameterSet->SetSamplerState("gLinearSampler", linearSampState);
	else
	{
		mGpuParameterSet->SetSamplerState("gSceneColor", linearSampState);
		mGpuParameterSet->SetSamplerState("gPrevColor", linearSampState);
	}
}

void TemporalFilteringMaterial::Prepare(const RendererView& view, const TShared<Texture>& prevFrame, const TShared<Texture>& curFrame, const TShared<Texture>& velocity, const TShared<Texture>& sceneDepth, const Vector2& jitter, float exposure)
{
	GpuBufferMappedScope filteringUniforms = gTemporalFilteringUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope resolveUniforms = gTemporalResolveUniformDefinition.AllocateTransient().Map();

	TShared<Texture> velocityTex = velocity;
	if(!velocityTex)
		velocityTex = Texture::kBlack;

	mPreviousColorTextureParameter.Set(prevFrame);
	mSceneColorTextureParameter.Set(curFrame);
	mSceneDepthTextureParameter.Set(sceneDepth);

	if(mHasVelocityTexture)
		mVelocityTextureParameter.Set(velocityTex);

	auto& colorProps = curFrame->GetProperties(); // Assuming prev and current frame are the same size
	auto& depthProps = sceneDepth->GetProperties();

	Vector4 colorPixelSize(1.0f / colorProps.Width, 1.0f / colorProps.Height, (float)colorProps.Width, (float)colorProps.Height);
	Vector4 depthPixelSize(1.0f / depthProps.Width, 1.0f / depthProps.Height, (float)depthProps.Width, (float)depthProps.Height);

	Vector4 velocityPixelSize(1.0f, 1.0f, 1.0f, 1.0f);
	if(mHasVelocityTexture)
	{
		auto& velocityProps = velocityTex->GetProperties();
		velocityPixelSize = Vector4(1.0f / velocityProps.Width, 1.0f / velocityProps.Height, (float)velocityProps.Width, (float)velocityProps.Height);
	}

	gTemporalFilteringUniformDefinition.gSceneColorTexelSize.Set(filteringUniforms, colorPixelSize);
	gTemporalFilteringUniformDefinition.gSceneDepthTexelSize.Set(filteringUniforms, depthPixelSize);
	gTemporalFilteringUniformDefinition.gVelocityTexelSize.Set(filteringUniforms, velocityPixelSize);
	gTemporalFilteringUniformDefinition.gManualExposure.Set(filteringUniforms, 1.0f / exposure);

	const GpuBackendConventions& gpuBackendConventions = mGpuDevice->GetCapabilities().Conventions;

	Vector2 jitterUV;
	jitterUV.X = jitter.X * 0.5f;

	if((gpuBackendConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
		jitterUV.Y = jitter.Y * 0.5f;
	else
		jitterUV.Y = jitter.Y * -0.5f;

	// Generate samples
	// Note: Move this code to a more general spot where it can be used by other temporal shaders.

	float sampleWeights[9];
	float sampleWeightsLowPass[9];

	float totalWeights = 0.0f;
	float totalWeightsLowPass = 0.0f;

	// Weights are generated using an exponential fit to Blackman-Harris 3.3
	bool useYCoCg = false; // Only relevant for general case, not using it for SSR
	float sharpness = 1.0f; // Make this a customizable parameter eventually
	if(useYCoCg)
	{
		static const Vector2 kSampleOffsets[] = {
			{ 0.0f, -1.0f },
			{ -1.0f, 0.0f },
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ 0.0f, 1.0f },
		};

		for(u32 i = 0; i < 5; ++i)
		{
			// Get rid of jitter introduced by the projection matrix
			Vector2 offset = kSampleOffsets[i] - jitterUV * Vector2(0.5f, -0.5f);

			offset *= 1.0f + sharpness * 0.5f;
			sampleWeights[i] = exp(-2.29f * offset.Dot(offset));
			totalWeights += sampleWeights[i];
		}

		for(u32 i = 5; i < 9; ++i)
			sampleWeights[i] = 0.0f;

		memset(sampleWeightsLowPass, 0, sizeof(sampleWeightsLowPass));
		totalWeightsLowPass = 1.0f;
	}
	else
	{
		static const Vector2 kSampleOffsets[] = {
			{ -1.0f, -1.0f },
			{ 0.0f, -1.0f },
			{ 1.0f, -1.0f },
			{ -1.0f, 0.0f },
			{ 0.0f, 0.0f },
			{ 1.0f, 0.0f },
			{ -1.0f, 1.0f },
			{ 0.0f, 1.0f },
			{ 1.0f, 1.0f },
		};

		for(u32 i = 0; i < 9; ++i)
		{
			// Get rid of jitter introduced by the projection matrix
			Vector2 offset = kSampleOffsets[i] - jitterUV;

			offset *= 1.0f + sharpness * 0.5f;
			sampleWeights[i] = exp(-2.29f * offset.Dot(offset));
			totalWeights += sampleWeights[i];

			// Low pass
			offset *= 0.25f;
			sampleWeightsLowPass[i] = exp(-2.29f * offset.Dot(offset));
			totalWeightsLowPass += sampleWeightsLowPass[i];
		}
	}

	for(u32 i = 0; i < 9; ++i)
	{
		gTemporalResolveUniformDefinition.gSampleWeights.Set(resolveUniforms, sampleWeights[i] / totalWeights, i);
		gTemporalResolveUniformDefinition.gSampleWeightsLowpass.Set(resolveUniforms, sampleWeightsLowPass[i] / totalWeightsLowPass, i);
	}

	mUniformBufferParameter.Set(filteringUniforms);
	mTemporalUniformBufferParameter.Set(resolveUniforms);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void TemporalFilteringMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const TShared<RenderTarget>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGpuParameterSet);
	info.ClearMask = RT_COLOR_ALL;

	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);

	const RendererViewProperties& viewProps = view.GetProperties();
	Area2I viewRect = viewProps.Target.ViewRect;

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

TemporalFilteringMaterial* TemporalFilteringMaterial::GetVariation(TemporalFilteringType type, bool velocity, bool msaa)
{
	switch(type)
	{
	default:
	case TemporalFilteringType::FullScreenAA:
		if(velocity)
		{
			if(msaa)
				return Get(GetVariation<TemporalFilteringType::FullScreenAA, true, true>());

			return Get(GetVariation<TemporalFilteringType::FullScreenAA, true, false>());
		}

		if(msaa)
			return Get(GetVariation<TemporalFilteringType::FullScreenAA, false, true>());

		return Get(GetVariation<TemporalFilteringType::FullScreenAA, false, false>());
	case TemporalFilteringType::SSR:
		if(velocity)
		{
			if(msaa)
				return Get(GetVariation<TemporalFilteringType::SSR, true, true>());

			return Get(GetVariation<TemporalFilteringType::SSR, true, false>());
		}

		if(msaa)
			return Get(GetVariation<TemporalFilteringType::SSR, false, true>());

		return Get(GetVariation<TemporalFilteringType::SSR, false, false>());
	}
}

EncodeDepthUniformDefinition gEncodeDepthUniformDefinition;

void EncodeDepthMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	SamplerStateInformation sampDesc;
	sampDesc.MinFilter = FO_POINT;
	sampDesc.MagFilter = FO_POINT;
	sampDesc.MipFilter = FO_POINT;
	sampDesc.AddressMode.U = TAM_CLAMP;
	sampDesc.AddressMode.V = TAM_CLAMP;
	sampDesc.AddressMode.W = TAM_CLAMP;

	TShared<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(sampDesc);
	SetSamplerState(mGpuParameterSet, "gInputSamp", "gInputTex", samplerState);
}

void EncodeDepthMaterial::Prepare(const TShared<Texture>& depth, float near, float far)
{
	GpuBufferMappedScope uniforms = gEncodeDepthUniformDefinition.AllocateTransient().Map();
	gEncodeDepthUniformDefinition.gNear.Set(uniforms, near);
	gEncodeDepthUniformDefinition.gFar.Set(uniforms, far);

	mInputTextureParameter.Set(depth);
	mUniformBufferParameter.Set(uniforms);
}

void EncodeDepthMaterial::Execute(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet, RT_NONE, RT_COLOR0);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void MSAACoverageMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
}

void MSAACoverageMaterial::Prepare(const RendererView& view, GBufferTextures gbuffer)
{
	mGBufferParams.Bind(gbuffer);

	const GpuBufferSuballocation& perView = view.GetPerViewBuffer();
	mGpuParameterSet->SetUniformBuffer("PerCamera", perView);
}

void MSAACoverageMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}

MSAACoverageMaterial* MSAACoverageMaterial::GetVariation(u32 msaaCount)
{
	switch(msaaCount)
	{
	case 2:
		return Get(GetVariation<2>());
	case 4:
		return Get(GetVariation<4>());
	case 8:
	default:
		return Get(GetVariation<8>());
	}
}

void MSAACoverageStencilMaterial::Initialize()
{
	mGpuParameterSet->GetSampledTextureParameter("gMSAACoverage", mCoverageTextureParameter);
}

void MSAACoverageStencilMaterial::Prepare(const TShared<Texture>& coverage)
{
	mCoverageTextureParameter.Set(coverage);
}

void MSAACoverageStencilMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}
}}
