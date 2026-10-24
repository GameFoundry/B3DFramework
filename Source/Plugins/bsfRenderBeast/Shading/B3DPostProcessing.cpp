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
#include "B3DRendererView.h"
#include "B3DRenderBeast.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "Utility/B3DRendererTextures.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "Image/B3DTexture.h"

namespace b3d { namespace render {

/** Adds the handle of sampler @p name of @p params to @p outHandles, if @p params has it. */
template<u32 Count>
void AddSamplerHandle(const TShared<GpuParameterSet>& params, const char* name, TInlineArray<GpuParameterSampler, Count>& outHandles)
{
	if(!params->HasSamplerState(name))
		return;

	GpuParameterSampler handle;
	params->GetSamplerStateParameter(name, handle);
	outHandles.Add(handle);
}

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
	mMsaa = mVariationParameters.GetI32("MSAA") > 0;

	mGpuParameterSet->TryGetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

TShared<GpuParameterSet> DownsampleMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	// Populate parameter buffer
	GpuBufferMappedScope uniforms = gDownsampleUniformDefinition.AllocateTransient().Map();
	const TextureProperties& rtProps = input->GetProperties();

	if(mMsaa)
	{
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(-1.0f, -1.0f), 0);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(1.0f, -1.0f), 1);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(-1.0f, 1.0f), 2);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, Vector2(1.0f, 1.0f), 3);
	}
	else
	{
		Vector2 invTextureSize(1.0f / rtProps.Width, 1.0f / rtProps.Height);

		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(-1.0f, -1.0f), 0);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(1.0f, -1.0f), 1);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(-1.0f, 1.0f), 2);
		gDownsampleUniformDefinition.gOffsets.Set(uniforms, invTextureSize * Vector2(1.0f, 1.0f), 3);
	}

	// Set parameters
	mUniformBufferParameter.Set(set, uniforms);
	mInputTextureParameter.Set(set, input);

	return set;
}

void DownsampleMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& input) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

	if(mMsaa)
	{
		const TextureProperties& rtProps = input->GetProperties();
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)rtProps.Width, (float)rtProps.Height));
	}
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TextureCreateInformation DownsampleMaterial::GetOutputCreateInformation(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1, Math::CeilToInt(rtProps.Width * 0.5f));
	u32 height = std::max(1, Math::CeilToInt(rtProps.Height * 0.5f));

	return TextureCreateInformation::Create2D(rtProps.Format, width, height, TextureUsageFlag::RenderTarget);
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

TShared<GpuParameterSet> EyeAdaptHistogramMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, const TShared<Texture>& output, const AutoExposureSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	// Populate parameter buffer
	GpuBufferMappedScope uniforms = gEyeAdaptHistogramUniformDefinition.AllocateTransient().Map();

	const TextureProperties& props = input->GetProperties();
	Vector4I offsetAndSize(0, 0, (i32)props.Width, (i32)props.Height);

	gEyeAdaptHistogramUniformDefinition.gHistogramParams.Set(uniforms, GetHistogramScaleOffset(settings));
	gEyeAdaptHistogramUniformDefinition.gPixelOffsetAndSize.Set(uniforms, offsetAndSize);

	Vector2I threadGroupCount = GetThreadGroupCount(input);
	gEyeAdaptHistogramUniformDefinition.gThreadGroupCount.Set(uniforms, threadGroupCount);

	// Set parameters
	mUniformBufferParameter.Set(set, uniforms);
	mSceneColorParameter.Set(set, input);
	mOutputTextureParameter.Set(set, output);

	return set;
}

void EyeAdaptHistogramMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& input) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const Vector2I threadGroupCount = GetThreadGroupCount(input);

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(threadGroupCount.X, threadGroupCount.Y);
}

TextureCreateInformation EyeAdaptHistogramMaterial::GetOutputCreateInformation(const TShared<Texture>& target)
{
	Vector2I threadGroupCount = GetThreadGroupCount(target);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	return TextureCreateInformation::Create2D(PF_RGBA16F, kHistogramNumTexels, numHistograms, TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
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

TShared<GpuParameterSet> EyeAdaptHistogramReduceMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& sceneColor, const TShared<Texture>& histogram, const TShared<Texture>& prevFrame)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gEyeAdaptHistogramReduceUniformDefinition.AllocateTransient().Map();

	mHistogramTextureParameter.Set(set, histogram);

	TShared<Texture> eyeAdaptationTex;
	if(prevFrame == nullptr) // Could be that this is the first run
		eyeAdaptationTex = Texture::kWhite;
	else
		eyeAdaptationTex = prevFrame;

	mEyeAdaptationTextureParameter.Set(set, eyeAdaptationTex);

	Vector2I threadGroupCount = EyeAdaptHistogramMaterial::GetThreadGroupCount(sceneColor);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	gEyeAdaptHistogramReduceUniformDefinition.gThreadGroupCount.Set(uniforms, numHistograms);

	mUniformBufferParameter.Set(set, uniforms);

	return set;
}

void EyeAdaptHistogramReduceMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

	Area2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMaterial::kHistogramNumTexels, 2.0f);
	GetRendererUtility().DrawScreenQuad(commandBuffer, drawUV);
}

TextureCreateInformation EyeAdaptHistogramReduceMaterial::GetOutputCreateInformation()
{
	return TextureCreateInformation::Create2D(PF_RGBA16F, EyeAdaptHistogramMaterial::kHistogramNumTexels, 2, TextureUsageFlag::RenderTarget);
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

TShared<GpuParameterSet> EyeAdaptationMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& reducedHistogram, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	mReducedHistogramTex.Set(set, reducedHistogram);

	GpuBufferMappedScope uniforms = gEyeAdaptationUniformDefinition.AllocateTransient().Map();
	PopulateUniformBuffer(uniforms, frameDelta, settings, exposureScale);

	mUniformBufferParameter.Set(set, uniforms);

	return set;
}

void EyeAdaptationMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TextureCreateInformation EyeAdaptationMaterial::GetOutputCreateInformation()
{
	return TextureCreateInformation::Create2D(PF_R32F, 1, 1, TextureUsageFlag::RenderTarget);
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

CreateTonemapLUTUniformDefinition gCreateTonemapLUTUniformDefinition;
WhiteBalanceUniformDefinition gWhiteBalanceUniformDefinition;

void CreateTonemap3DLUTMaterial::PopulateTonemappingUniformBuffer(const RenderSettings& settings, const GpuBufferMappedScope& uniforms)
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

void CreateTonemap3DLUTMaterial::PopulateWhiteBalanceUniformBuffer(const RenderSettings& settings, const GpuBufferMappedScope& uniforms)
{
	gWhiteBalanceUniformDefinition.gWhiteTemp.Set(uniforms, settings.WhiteBalance.Temperature);
	gWhiteBalanceUniformDefinition.gWhiteOffset.Set(uniforms, settings.WhiteBalance.Tint);
}

void CreateTonemap3DLUTMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("WhiteBalanceInput", mWhiteBalanceUniformBufferParameter);
	mGpuParameterSet->GetStorageTextureParameter("gOutputTex", mOutputTextureParameter);
}

void CreateTonemap3DLUTMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", kLutSize);
}

TShared<GpuParameterSet> CreateTonemap3DLUTMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& output, const RenderSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope lutUniforms = gCreateTonemapLUTUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope whiteBalanceUniforms = gWhiteBalanceUniformDefinition.AllocateTransient().Map();

	PopulateTonemappingUniformBuffer(settings, lutUniforms);
	PopulateWhiteBalanceUniformBuffer(settings, whiteBalanceUniforms);

	mUniformBufferParameter.Set(set, lutUniforms);
	mWhiteBalanceUniformBufferParameter.Set(set, whiteBalanceUniforms);
	mOutputTextureParameter.Set(set, output);

	return set;
}

void CreateTonemap3DLUTMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	commandBuffer.DispatchCompute(kLutSize / 8, kLutSize / 8, kLutSize);
}

TextureCreateInformation CreateTonemap3DLUTMaterial::GetOutputCreateInformation()
{
	TextureCreateInformation information;
	information.Type = TEX_TYPE_3D;
	information.Format = PF_RGBA8;
	information.Width = kLutSize;
	information.Height = kLutSize;
	information.Depth = kLutSize;
	information.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;
	information.SampleCount = 1;

	return information;
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
	defines.Set("LUT_SIZE", CreateTonemap3DLUTMaterial::kLutSize);
}

TShared<GpuParameterSet> TonemappingMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& sceneColor, const TShared<Texture>& eyeAdaptation, const TShared<Texture>& bloom,
	const TShared<Texture>& colorLUT, const RenderSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& texProps = sceneColor->GetProperties();

	GpuBufferMappedScope uniforms = gTonemappingUniformDefinition.AllocateTransient().Map();

	gTonemappingUniformDefinition.gRawGamma.Set(uniforms, 1.0f / settings.Gamma);
	gTonemappingUniformDefinition.gManualExposureScale.Set(uniforms, Math::RaiseToPower(2.0f, settings.ExposureScale));
	gTonemappingUniformDefinition.gTexSize.Set(uniforms, Vector2((float)texProps.Width, (float)texProps.Height));
	gTonemappingUniformDefinition.gBloomTint.Set(uniforms, settings.Bloom.Tint);
	gTonemappingUniformDefinition.gNumSamples.Set(uniforms, texProps.SampleCount);

	mUniformBufferParameter.Set(set, uniforms);

	mInputTextureParameter.Set(set, sceneColor);
	mColorLUTParameter.Set(set, colorLUT);
	mEyeAdaptationTextureParameter.Set(set, eyeAdaptation);
	mBloomTextureParameter.Set(set, bloom != nullptr ? bloom : Texture::kBlack);

	return set;
}

void TonemappingMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TonemappingMaterial* TonemappingMaterial::GetVariation(bool gammaOnly, bool autoExposure, bool MSAA)
{
	if(gammaOnly)
	{
		if(autoExposure)
		{
			if(MSAA)
				return Get(GetVariation<true, true, true>());
			else
				return Get(GetVariation<true, true, false>());
		}
		else
		{
			if(MSAA)
				return Get(GetVariation<true, false, true>());
			else
				return Get(GetVariation<true, false, false>());
		}
	}
	else
	{
		if(autoExposure)
		{
			if(MSAA)
				return Get(GetVariation<false, true, true>());
			else
				return Get(GetVariation<false, true, false>());
		}
		else
		{
			if(MSAA)
				return Get(GetVariation<false, false, true>());
			else
				return Get(GetVariation<false, false, false>());
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

TShared<GpuParameterSet> BloomClipMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, float threshold, const TShared<Texture>& eyeAdaptation, const RenderSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gBloomClipUniformDefinition.AllocateTransient().Map();

	gBloomClipUniformDefinition.gThreshold.Set(uniforms, threshold);
	gBloomClipUniformDefinition.gManualExposureScale.Set(uniforms, Math::RaiseToPower(2.0f, settings.ExposureScale));

	mUniformBufferParameter.Set(set, uniforms);
	mInputTextureParameter.Set(set, input);
	mEyeAdaptationTextureParameter.Set(set, eyeAdaptation);

	return set;
}

void BloomClipMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> ScreenSpaceLensFlareMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, const ScreenSpaceLensFlareSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gScreenSpaceLensFlareUniformDefinition.AllocateTransient().Map();

	gScreenSpaceLensFlareUniformDefinition.gThreshold.Set(uniforms, settings.Threshold);
	gScreenSpaceLensFlareUniformDefinition.gGhostCount.Set(uniforms, settings.GhostCount);
	gScreenSpaceLensFlareUniformDefinition.gGhostSpacing.Set(uniforms, settings.GhostSpacing);
	gScreenSpaceLensFlareUniformDefinition.gHaloRadius.Set(uniforms, settings.HaloRadius);
	gScreenSpaceLensFlareUniformDefinition.gHaloThickness.Set(uniforms, settings.HaloThickness);
	gScreenSpaceLensFlareUniformDefinition.gHaloThreshold.Set(uniforms, settings.HaloThreshold);
	gScreenSpaceLensFlareUniformDefinition.gHaloAspectRatio.Set(uniforms, settings.HaloAspectRatio);
	gScreenSpaceLensFlareUniformDefinition.gChromaticAberration.Set(uniforms, settings.ChromaticAberrationOffset);

	mUniformBufferParameter.Set(set, uniforms);

	mInputTextureParameter.Set(set, input);
	mGradientTextureParameter.Set(set, RendererTextures::lensFlareGradient);

	return set;
}

void ScreenSpaceLensFlareMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> ChromaticAberrationMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, const ChromaticAberrationSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& texProps = input->GetProperties();

	GpuBufferMappedScope uniforms = gChromaticAberrationUniformDefinition.AllocateTransient().Map();

	gChromaticAberrationUniformDefinition.gInputSize.Set(uniforms, Vector2((float)texProps.Width, (float)texProps.Height));
	gChromaticAberrationUniformDefinition.gShiftAmount.Set(uniforms, settings.ShiftAmount);

	mUniformBufferParameter.Set(set, uniforms);

	TShared<Texture> fringeTex;
	if(settings.FringeTexture)
		fringeTex = settings.FringeTexture;
	else
		fringeTex = RendererTextures::chromaticAberrationFringe;

	mInputTextureParameter.Set(set, input);
	mFringeTextureParameter.Set(set, fringeTex);

	return set;
}

void ChromaticAberrationMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> FilmGrainMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, float time, const FilmGrainSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gFilmGrainUniformDefinition.AllocateTransient().Map();

	gFilmGrainUniformDefinition.gIntensity.Set(uniforms, settings.Intensity);
	gFilmGrainUniformDefinition.gTime.Set(uniforms, settings.Speed * time);

	mUniformBufferParameter.Set(set, uniforms);

	mInputTextureParameter.Set(set, input);

	return set;
}

void FilmGrainMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

TShared<GpuParameterSet> GaussianBlurMaterial::Prepare(FrameGraphPass& pass, Direction direction, const TShared<Texture>& source, float filterSize, const Color& tint,
	const TShared<Texture>& additive)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gGaussianBlurUniformDefinition.AllocateTransient().Map();
	PopulateUniformBuffer(uniforms, direction, source, filterSize, tint);
	mUniformBufferParameter.Set(set, uniforms);

	mInputTextureParameter.Set(set, source);

	if(mIsAdditive)
	{
		if(additive)
			mAdditiveTextureParameter.Set(set, additive);
		else
			mAdditiveTextureParameter.Set(set, Texture::kBlack);
	}

	return set;
}

void GaussianBlurMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void AddGaussianBlurPasses(FrameGraph& graph, const TShared<Texture>& source, float filterSize, const TShared<Texture>& destination, const Color& tint, const TShared<Texture>& additive)
{
	const TextureProperties& srcProps = source->GetProperties();
	const TextureProperties& dstProps = destination->GetProperties();

	const TShared<Texture> horizontalOutput = graph.CreateTexture("GaussianBlurHorizontal", TextureCreateInformation::Create2D(srcProps.Format, dstProps.Width, dstProps.Height, TextureUsageFlag::RenderTarget));

	GaussianBlurMaterial* const horizontalMaterial = GaussianBlurMaterial::GetVariation(false);

	FrameGraphRasterPass horizontalPass;
	horizontalPass.SetColorAttachment(0, horizontalOutput, FrameGraphAttachmentLoad::DontCare);

	const TShared<GpuParameterSet> horizontalSet = horizontalMaterial->Prepare(horizontalPass, GaussianBlurMaterial::DirHorizontal, source, filterSize);
	graph.AddRasterPass("GaussianBlurH", std::move(horizontalPass), [horizontalMaterial, horizontalSet](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		horizontalMaterial->Execute(context, horizontalSet);
	});

	// Adding nothing gives the same result as the variation without the additive input
	GaussianBlurMaterial* const verticalMaterial = GaussianBlurMaterial::GetVariation(additive != nullptr);

	FrameGraphRasterPass verticalPass;
	verticalPass.SetColorAttachment(0, destination, FrameGraphAttachmentLoad::DontCare);

	const TShared<GpuParameterSet> verticalSet = verticalMaterial->Prepare(verticalPass, GaussianBlurMaterial::DirVertical, horizontalOutput, filterSize, tint, additive);
	graph.AddRasterPass("GaussianBlurV", std::move(verticalPass), [verticalMaterial, verticalSet](FrameGraphPassContext& context)
	{
		context.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
		verticalMaterial->Execute(context, verticalSet);
	});
}

GaussianDOFUniformDefinition gGaussianDOFUniformDefinition;

void GaussianDOFSeparateMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
	mGpuParameterSet->GetSampledTextureParameter("gColorTex", mColorTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	if(mGpuParameterSet->HasSamplerState("gColorSamp"))
		mGpuParameterSet->GetSamplerStateParameter("gColorSamp", mColorSamplerParameter);
	else
		mGpuParameterSet->GetSamplerStateParameter("gColorTex", mColorSamplerParameter);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;
	samplerStateCreateInformation.AddressMode.U = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.V = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.W = TAM_CLAMP;

	mPointSamplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
}

TShared<GpuParameterSet> GaussianDOFSeparateMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& color, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& srcProps = color->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gGaussianDOFUniformDefinition.AllocateTransient().Map();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniforms, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniforms, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniforms, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniforms, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniforms, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(set, uniforms);

	mColorTextureParameter.Set(set, color);
	mDepthTextureParameter.Set(set, depth);
	mColorSamplerParameter.Set(set, mPointSamplerState);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void GaussianDOFSeparateMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TextureCreateInformation GaussianDOFSeparateMaterial::GetOutputCreateInformation(const TShared<Texture>& color)
{
	const TextureProperties& srcProps = color->GetProperties();

	u32 outputWidth = std::max(1U, srcProps.Width / 2);
	u32 outputHeight = std::max(1U, srcProps.Height / 2);

	return TextureCreateInformation::Create2D(srcProps.Format, outputWidth, outputHeight, TextureUsageFlag::RenderTarget);
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
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);

	mGpuParameterSet->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	if(mGpuParameterSet->HasSampledTexture("gNearTex"))
		mGpuParameterSet->GetSampledTextureParameter("gNearTex", mNearTextureParameter);

	if(mGpuParameterSet->HasSampledTexture("gFarTex"))
		mGpuParameterSet->GetSampledTextureParameter("gFarTex", mFarTextureParameter);
}

TShared<GpuParameterSet> GaussianDOFCombineMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& focused, const TShared<Texture>& near, const TShared<Texture>& far,
	const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& srcProps = focused->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gGaussianDOFUniformDefinition.AllocateTransient().Map();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniforms, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniforms, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniforms, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniforms, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniforms, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(set, uniforms);

	mFocusedTextureParameter.Set(set, focused);
	mNearTextureParameter.Set(set, near);
	mFarTextureParameter.Set(set, far);
	mDepthTextureParameter.Set(set, depth);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void GaussianDOFCombineMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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
	mMsaa = mVariationParameters.GetI32("MSAA_COUNT") > 1;

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

TShared<GpuParameterSet> BokehDOFPrepareMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, const TShared<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& srcProps = input->GetProperties();

	GpuBufferMappedScope prepareUniforms = gBokehDOFPrepareUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope commonUniforms = gDepthOfFieldCommonUniformDefinition.AllocateTransient().Map();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gBokehDOFPrepareUniformDefinition.gInvInputSize.Set(prepareUniforms, invTexSize);

	BokehDOFMaterial::PopulateDofCommonParams(commonUniforms, settings, view);

	mUniformBufferParameter.Set(set, prepareUniforms);
	mCommonUniformBufferParameter.Set(set, commonUniforms);
	mInputTextureParameter.Set(set, input);
	mDepthTextureParameter.Set(set, depth);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void BokehDOFPrepareMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& input) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

	if(mMsaa)
	{
		const TextureProperties& srcProps = input->GetProperties();
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)srcProps.Width, (float)srcProps.Height));
	}
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TextureCreateInformation BokehDOFPrepareMaterial::GetOutputCreateInformation(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.Width, 2U));
	u32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.Height, 2U));

	return TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget);
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
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
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

TShared<GpuParameterSet> BokehDOFMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const TShared<Texture>& output)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& srcProps = input->GetProperties();
	const TextureProperties& dstProps = output->GetProperties();

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

	mUniformBufferParameter.Set(set, bokehUniforms);
	mCommonUniformBufferParameter.Set(set, depthOfFieldCommonUniforms);
	mInputTextureVSParameter.Set(set, input);
	mInputTextureFSParameter.Set(set, input);

	TShared<Texture> bokehTexture = settings.BokehShape;
	if(bokehTexture == nullptr)
		bokehTexture = RendererTextures::bokehFlare;

	mBokehTextureParameter.Set(set, bokehTexture);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void BokehDOFMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& input) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = input->GetProperties();
	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	const Vector2I tileCount = imageSize / 1;

	commandBuffer.SetVertexDescription(mTileVertexDescription);

	TShared<GpuBuffer> buffers[] = { mTileVertexBuffer };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(mTileIndexBuffer);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	Bind(commandBuffer, set);
	const u32 numInstances = Math::DivideAndRoundUp((u32)(tileCount.X * tileCount.Y), kQuadsPerTile);
	commandBuffer.DrawIndexed(0, kQuadsPerTile * 6, 0, kQuadsPerTile * 4, numInstances);
}

TextureCreateInformation BokehDOFMaterial::GetOutputCreateInformation(const TShared<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = rtProps.Width;
	u32 height = rtProps.Height * 2 + kNearFarPadding;

	return TextureCreateInformation::Create2D(PF_RGBA16F, width, height, TextureUsageFlag::RenderTarget);
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
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
	mGpuParameterSet->GetSampledTextureParameter("gUnfocusedTex", mUnfocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

TShared<GpuParameterSet> BokehDOFCombineMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& unfocused, const TShared<Texture>& focused, const TShared<Texture>& depth,
	const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

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

	mUniformBufferParameter.Set(set, bokehCombineUniforms);
	mCommonUniformBufferParameter.Set(set, depthOfFieldCommonUniforms);
	mUnfocusedTextureParameter.Set(set, unfocused);
	mFocusedTextureParameter.Set(set, focused);
	mDepthTextureParameter.Set(set, depth);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void BokehDOFCombineMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void BuildHiZMaterial::Initialize()
{
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mInputTexture);
}

TShared<GpuParameterSet> BuildHiZMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 sourceMip)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
	mInputTexture.Set(set, source, TextureSurface(sourceMip));

	return set;
}

void BuildHiZMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Area2& sourceArea) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer, sourceArea);
}

FXAAUniformDefinition gFXAAUniformDefinition;

void FXAAMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTexture);
}

TShared<GpuParameterSet> FXAAMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& source)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferMappedScope uniforms = gFXAAUniformDefinition.AllocateTransient().Map();
	gFXAAUniformDefinition.gInvTexSize.Set(uniforms, invTexSize);

	mUniformBufferParameter.Set(set, uniforms);
	mInputTexture.Set(set, source);

	return set;
}

void FXAAMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	mInputSamplerState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		AddSamplerHandle(mGpuParameterSet, "gInputSamp", mInputSamplerParameters);
	else
	{
		if(isFinal)
		{
			AddSamplerHandle(mGpuParameterSet, "gDepthTex", mInputSamplerParameters);
			AddSamplerHandle(mGpuParameterSet, "gNormalsTex", mInputSamplerParameters);
		}

		if(!isFinal || mixWithUpsampled)
			AddSamplerHandle(mGpuParameterSet, "gSetupAO", mInputSamplerParameters);

		if(mixWithUpsampled)
			AddSamplerHandle(mGpuParameterSet, "gDownsampledAO", mInputSamplerParameters);
	}

	SamplerStateInformation randomSampDesc;
	randomSampDesc.MinFilter = FO_POINT;
	randomSampDesc.MagFilter = FO_POINT;
	randomSampDesc.MipFilter = FO_POINT;
	randomSampDesc.AddressMode.U = TAM_WRAP;
	randomSampDesc.AddressMode.V = TAM_WRAP;
	randomSampDesc.AddressMode.W = TAM_WRAP;

	mRandomSamplerState = mGpuDevice->FindOrCreateSamplerState(randomSampDesc);
	if(mGpuParameterSet->HasSamplerState("gRandomSamp"))
		mGpuParameterSet->GetSamplerStateParameter("gRandomSamp", mRandomSamplerParameter);
	else if(mGpuParameterSet->HasSamplerState("gRandomTex"))
		mGpuParameterSet->GetSamplerStateParameter("gRandomTex", mRandomSamplerParameter);
}

TShared<GpuParameterSet> SSAOMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const SSAOTextureInputs& textures, const TShared<Texture>& destination, const AmbientOcclusionSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	// Scale that can be used to adjust how quickly does AO radius increase with downsampled AO. This yields a very
	// small AO radius at highest level, and very large radius at lowest level
	static const float kDownsampleScale = 4.0f;

	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& rtProps = destination->GetProperties();

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

	mUniformBufferParameter.Set(set, uniforms);

	mSetupAOTexture.Set(set, textures.AoSetup);

	bool finalPass = mVariationParameters.GetBool("FINAL_AO");
	if(finalPass)
	{
		mDepthTexture.Set(set, textures.SceneDepth);
		mNormalsTexture.Set(set, textures.SceneNormals);
	}

	if(upsample)
		mDownsampledAOTexture.Set(set, textures.AoDownsampled);

	mRandomTexture.Set(set, textures.RandomRotations);

	for(const GpuParameterSampler& samplerParameter : mInputSamplerParameters)
		samplerParameter.Set(set, mInputSamplerState);

	mRandomSamplerParameter.Set(set, mRandomSamplerState);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void SSAOMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_LINEAR;
	inputSampDesc.MagFilter = FO_LINEAR;
	inputSampDesc.MipFilter = FO_LINEAR;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	mInputSamplerState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);

	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		AddSamplerHandle(mGpuParameterSet, "gInputSamp", mInputSamplerParameters);
	else
	{
		AddSamplerHandle(mGpuParameterSet, "gDepthTex", mInputSamplerParameters);
		AddSamplerHandle(mGpuParameterSet, "gNormalsTex", mInputSamplerParameters);
	}
}

TShared<GpuParameterSet> SSAODownsampleMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const TShared<Texture>& depth, const TShared<Texture>& normals, const TShared<Texture>& destination, float depthRange)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& rtProps = destination->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / rtProps.Width;
	pixelSize.Y = 1.0f / rtProps.Height;

	float scale = viewProps.Target.ViewRect.Width / (float)rtProps.Width;

	GpuBufferMappedScope uniforms = gSSAODownsampleUniformDefinition.AllocateTransient().Map();
	gSSAODownsampleUniformDefinition.gPixelSize.Set(uniforms, pixelSize);
	gSSAODownsampleUniformDefinition.gInvDepthThreshold.Set(uniforms, (1.0f / depthRange) / scale);

	mUniformBufferParameter.Set(set, uniforms);
	mDepthTexture.Set(set, depth);
	mNormalsTexture.Set(set, normals);

	for(const GpuParameterSampler& samplerParameter : mInputSamplerParameters)
		samplerParameter.Set(set, mInputSamplerState);

	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void SSAODownsampleMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

SSAOBlurUniformDefinition gSSAOBlurUniformDefinition;

void SSAOBlurMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gInputTex", mAOTexture);
	mGpuParameterSet->GetSampledTextureParameter("gDepthTex", mDepthTexture);
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	mInputSamplerState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGpuParameterSet->HasSamplerState("gInputSamp"))
		AddSamplerHandle(mGpuParameterSet, "gInputSamp", mInputSamplerParameters);
	else
	{
		AddSamplerHandle(mGpuParameterSet, "gInputTex", mInputSamplerParameters);
		AddSamplerHandle(mGpuParameterSet, "gDepthTex", mInputSamplerParameters);
	}
}

TShared<GpuParameterSet> SSAOBlurMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const TShared<Texture>& ao, const TShared<Texture>& sceneDepth, float depthRange)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

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

	mUniformBufferParameter.Set(set, uniforms);
	mAOTexture.Set(set, ao);
	mDepthTexture.Set(set, sceneDepth);

	for(const GpuParameterSampler& samplerParameter : mInputSamplerParameters)
		samplerParameter.Set(set, mInputSamplerState);

	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void SSAOBlurMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);
}

TShared<GpuParameterSet> SSRStencilMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const GBufferTextures& gbuffer, const ScreenSpaceReflectionsSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	mGBufferParams.Bind(set, gbuffer);

	Vector2 roughnessScaleBias = SSRTraceMaterial::CalcRoughnessFadeScaleBias(settings.MaxRoughness);

	GpuBufferMappedScope uniforms = gSSRStencilUniformDefinition.AllocateTransient().Map();
	gSSRStencilUniformDefinition.gRoughnessScaleBias.Set(uniforms, roughnessScaleBias);
	mUniformBufferParameter.Set(set, uniforms);

	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void SSRStencilMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const RendererView& view) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

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
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	SamplerStateInformation desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	mHiZSamplerState = mGpuDevice->FindOrCreateSamplerState(desc);
	if(mGpuParameterSet->HasSamplerState("gHiZSamp"))
		mGpuParameterSet->GetSamplerStateParameter("gHiZSamp", mHiZSamplerParameter);
	else if(mGpuParameterSet->HasSamplerState("gHiZ"))
		mGpuParameterSet->GetSamplerStateParameter("gHiZ", mHiZSamplerParameter);
}

TShared<GpuParameterSet> SSRTraceMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const GBufferTextures& gbuffer, const TShared<Texture>& sceneColor, const TShared<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& hiZProps = hiZ->GetProperties();

	mGBufferParams.Bind(set, gbuffer);
	mSceneColorTextureParameter.Set(set, sceneColor);
	mHiZTextureParameter.Set(set, hiZ);
	mHiZSamplerParameter.Set(set, mHiZSamplerState);

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

	mUniformBufferParameter.Set(set, uniforms);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void SSRTraceMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const RendererView& view) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

	const RendererViewProperties& viewProps = view.GetProperties();
	if(viewProps.Target.NumSamples > 1)
	{
		const Area2I& viewRect = viewProps.Target.ViewRect;
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	}
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
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

	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);

	SamplerStateInformation pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	mPointSamplerState = mGpuDevice->FindOrCreateSamplerState(pointSampDesc);

	// The compiler strips samplers a variation does not use (the MSAA depth read uses Load)
	if(mGpuParameterSet->HasSamplerState("gPointSampler"))
		AddSamplerHandle(mGpuParameterSet, "gPointSampler", mPointSamplerParameters);
	else
		AddSamplerHandle(mGpuParameterSet, "gSceneDepth", mPointSamplerParameters);

	SamplerStateInformation linearSampDesc;
	linearSampDesc.MinFilter = FO_LINEAR;
	linearSampDesc.MagFilter = FO_LINEAR;
	linearSampDesc.MipFilter = FO_LINEAR;
	linearSampDesc.AddressMode.U = TAM_CLAMP;
	linearSampDesc.AddressMode.V = TAM_CLAMP;
	linearSampDesc.AddressMode.W = TAM_CLAMP;

	mLinearSamplerState = mGpuDevice->FindOrCreateSamplerState(linearSampDesc);
	if(mGpuParameterSet->HasSamplerState("gLinearSampler"))
		AddSamplerHandle(mGpuParameterSet, "gLinearSampler", mLinearSamplerParameters);
	else
	{
		AddSamplerHandle(mGpuParameterSet, "gSceneColor", mLinearSamplerParameters);
		AddSamplerHandle(mGpuParameterSet, "gPrevColor", mLinearSamplerParameters);
	}
}

TShared<GpuParameterSet> TemporalFilteringMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const TShared<Texture>& previousFrame, const TShared<Texture>& currentFrame, const TShared<Texture>& velocity, const TShared<Texture>& sceneDepth, const Vector2& jitter, float exposure)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope filteringUniforms = gTemporalFilteringUniformDefinition.AllocateTransient().Map();
	GpuBufferMappedScope resolveUniforms = gTemporalResolveUniformDefinition.AllocateTransient().Map();

	TShared<Texture> velocityTex = velocity;
	if(!velocityTex)
		velocityTex = Texture::kBlack;

	mPreviousColorTextureParameter.Set(set, previousFrame);
	mSceneColorTextureParameter.Set(set, currentFrame);
	mSceneDepthTextureParameter.Set(set, sceneDepth);

	if(mHasVelocityTexture)
		mVelocityTextureParameter.Set(set, velocityTex);

	for(const GpuParameterSampler& samplerParameter : mPointSamplerParameters)
		samplerParameter.Set(set, mPointSamplerState);

	for(const GpuParameterSampler& samplerParameter : mLinearSamplerParameters)
		samplerParameter.Set(set, mLinearSamplerState);

	auto& colorProps = currentFrame->GetProperties(); // Assuming prev and current frame are the same size
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

	mUniformBufferParameter.Set(set, filteringUniforms);
	mTemporalUniformBufferParameter.Set(set, resolveUniforms);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void TemporalFilteringMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const RendererView& view) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);

	const RendererViewProperties& viewProps = view.GetProperties();
	Area2I viewRect = viewProps.Target.ViewRect;

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
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
	mGpuParameterSet->GetUniformBufferParameter("PerCamera", mPerCameraParameter);
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

TShared<GpuParameterSet> EncodeDepthMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const TShared<Texture>& depth, float near, float far)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	GpuBufferMappedScope uniforms = gEncodeDepthUniformDefinition.AllocateTransient().Map();
	gEncodeDepthUniformDefinition.gNear.Set(uniforms, near);
	gEncodeDepthUniformDefinition.gFar.Set(uniforms, far);

	mUniformBufferParameter.Set(set, uniforms);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());
	mInputTextureParameter.Set(set, depth);

	return set;
}

void EncodeDepthMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

void MSAACoverageMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mGpuParameterSet->TryGetUniformBufferParameter("PerCamera", mPerCameraParameter);
}

TShared<GpuParameterSet> MSAACoverageMaterial::Prepare(FrameGraphPass& pass, const RendererView& view, const GBufferTextures& gbuffer)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);

	mGBufferParams.Bind(set, gbuffer);
	mPerCameraParameter.Set(set, view.GetPerViewBuffer());

	return set;
}

void MSAACoverageMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const RendererView& view) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer, set);
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

TShared<GpuParameterSet> MSAACoverageStencilMaterial::Prepare(FrameGraphPass& pass, const TShared<Texture>& coverage)
{
	const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
	mCoverageTextureParameter.Set(set, coverage);

	return set;
}

void MSAACoverageStencilMaterial::Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const RendererView& view) const
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer, set);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}
}}
