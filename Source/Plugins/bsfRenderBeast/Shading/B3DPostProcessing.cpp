//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPostProcessing.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DCamera.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Image/B3DPixelUtility.h"
#include "Utility/B3DBitwise.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "B3DRendererView.h"
#include "B3DRenderBeast.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "Utility/B3DRendererTextures.h"
#include "RenderAPI/B3DVertexDescription.h"

namespace b3d { namespace render {

void SetSamplerState(const SPtr<GpuParameters>& params, const String& name, const String& secondaryName, const SPtr<SamplerState>& samplerState, bool optional = false)
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
	mGPUParameters->TryGetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void DownsampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Populate parameter buffer
	GpuBufferSuballocation uniformBuffer = gDownsampleUniformDefinition.AllocateTransient();
	const TextureProperties& rtProps = input->GetProperties();

	bool MSAA = mVariationParameters.GetI32("MSAA") > 0;
	if(MSAA)
	{
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, Vector2(-1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, Vector2(1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, Vector2(-1.0f, 1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, Vector2(1.0f, 1.0f));
	}
	else
	{
		Vector2 invTextureSize(1.0f / rtProps.Width, 1.0f / rtProps.Height);

		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, invTextureSize * Vector2(-1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, invTextureSize * Vector2(1.0f, -1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, invTextureSize * Vector2(-1.0f, 1.0f));
		gDownsampleUniformDefinition.gOffsets.Set(uniformBuffer, invTextureSize * Vector2(1.0f, 1.0f));
	}

	// Set parameters
	mUniformBufferParameter.Set(uniformBuffer);
	mInputTextureParameter.Set(input);

	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGPUParameters, RT_DEPTH_STENCIL));

	Bind(commandBuffer);

	if(MSAA)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)rtProps.Width, (float)rtProps.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation DownsampleMaterial::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1, Math::CeilToInt(rtProps.Width * 0.5f));
	u32 height = std::max(1, Math::CeilToInt(rtProps.Height * 0.5f));

	return PooledRenderTextureCreateInformation::Create2D(rtProps.Format, width, height, TU_RENDERTARGET);
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
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gSceneColorTex", mSceneColorParameter);
	mGPUParameters->GetStorageTextureParameter("gOutputTex", mOutputTextureParameter);
}

void EyeAdaptHistogramMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", kThreadGroupSizeY);
	defines.Set("LOOP_COUNT_X", kLoopCountX);
	defines.Set("LOOP_COUNT_Y", kLoopCountY);
}

void EyeAdaptHistogramMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Populate parameter buffer
	GpuBufferSuballocation uniformBuffer = gEyeAdaptHistogramUniformDefinition.AllocateTransient();

	const TextureProperties& props = input->GetProperties();
	Vector4I offsetAndSize(0, 0, (i32)props.Width, (i32)props.Height);

	gEyeAdaptHistogramUniformDefinition.gHistogramParams.Set(uniformBuffer, GetHistogramScaleOffset(settings));
	gEyeAdaptHistogramUniformDefinition.gPixelOffsetAndSize.Set(uniformBuffer, offsetAndSize);

	Vector2I threadGroupCount = GetThreadGroupCount(input);
	gEyeAdaptHistogramUniformDefinition.gThreadGroupCount.Set(uniformBuffer, threadGroupCount);

	// Set parameters
	mUniformBufferParameter.Set(uniformBuffer);
	mSceneColorParameter.Set(input);

	// Dispatch
	mOutputTextureParameter.Set(output);

	Bind(commandBuffer);

	commandBuffer.DispatchCompute(threadGroupCount.X, threadGroupCount.Y);
}

PooledRenderTextureCreateInformation EyeAdaptHistogramMaterial::GetOutputDesc(const SPtr<Texture>& target)
{
	Vector2I threadGroupCount = GetThreadGroupCount(target);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, kHistogramNumTexels, numHistograms, TU_LOADSTORE);
}

Vector2I EyeAdaptHistogramMaterial::GetThreadGroupCount(const SPtr<Texture>& target)
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
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gHistogramTex", mHistogramTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTextureParameter);
}

void EyeAdaptHistogramReduceMaterial::Prepare(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame)
{
	GpuBufferSuballocation uniformBuffer = gEyeAdaptHistogramReduceUniformDefinition.AllocateTransient();

	mHistogramTextureParameter.Set(histogram);

	SPtr<Texture> eyeAdaptationTex;
	if(prevFrame == nullptr) // Could be that this is the first run
		eyeAdaptationTex = Texture::kWhite;
	else
		eyeAdaptationTex = prevFrame;

	mEyeAdaptationTextureParameter.Set(eyeAdaptationTex);

	Vector2I threadGroupCount = EyeAdaptHistogramMaterial::GetThreadGroupCount(sceneColor);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	gEyeAdaptHistogramReduceUniformDefinition.gThreadGroupCount.Set(uniformBuffer, numHistograms);

	mUniformBufferParameter.Set(uniformBuffer);
}

void EyeAdaptHistogramReduceMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters, RT_DEPTH_STENCIL);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);

	Area2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMaterial::kHistogramNumTexels, 2.0f);
	GetRendererUtility().DrawScreenQuad(commandBuffer, drawUV);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptHistogramReduceMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, EyeAdaptHistogramMaterial::kHistogramNumTexels, 2, TU_RENDERTARGET);
}

EyeAdaptationUniformDefinition gEyeAdaptationUniformDefinition;

void EyeAdaptationMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("EyeAdaptationParams", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gHistogramTex", mReducedHistogramTex);
}

void EyeAdaptationMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", EyeAdaptHistogramMaterial::kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMaterial::kThreadGroupSizeY);
}

void EyeAdaptationMaterial::Prepare(const SPtr<Texture>& reducedHistogram, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	mReducedHistogramTex.Set(reducedHistogram);

	GpuBufferSuballocation uniformBuffer = gEyeAdaptationUniformDefinition.AllocateTransient();
	PopulateUniformBuffer(uniformBuffer, frameDelta, settings, exposureScale);

	mUniformBufferParameter.Set(uniformBuffer);
}

void EyeAdaptationMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters, RT_DEPTH_STENCIL);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

void EyeAdaptationMaterial::PopulateUniformBuffer(const GpuBufferSuballocation& uniformBuffer, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
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

	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniformBuffer, eyeAdaptationParams[0], 0);
	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniformBuffer, eyeAdaptationParams[1], 1);
	gEyeAdaptationUniformDefinition.gEyeAdaptationParams.Set(uniformBuffer, eyeAdaptationParams[2], 2);
}

void EyeAdaptationBasicSetupMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("EyeAdaptationParams", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;

	SPtr<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
	SetSamplerState(mGPUParameters, "gInputSamp", "gInputTex", samplerState);
}

void EyeAdaptationBasicSetupMaterial::Prepare(const SPtr<Texture>& input, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	mInputTextureParameter.Set(input);

	GpuBufferSuballocation uniformBuffer = gEyeAdaptationUniformDefinition.AllocateTransient();
	EyeAdaptationMaterial::PopulateUniformBuffer(uniformBuffer, frameDelta, settings, exposureScale);

	mUniformBufferParameter.Set(uniformBuffer);
}

void EyeAdaptationBasicSetupMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationBasicSetupMaterial::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, props.Width, props.Height, TU_RENDERTARGET);
}

EyeAdaptationBasicUniformDefinition gEyeAdaptationBasicUniformDefinition;

void EyeAdaptationBasicMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("EyeAdaptationParams", mEyeAdaptationUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gCurFrameTex", mCurrentFrameTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gPrevFrameTex", mPreviousFrameTextureParameter);
}

void EyeAdaptationBasicMaterial::Prepare(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	GpuBufferSuballocation eyeAdaptationUniformBuffer = gEyeAdaptationUniformDefinition.AllocateTransient();
	GpuBufferSuballocation uniformBuffer = gEyeAdaptationBasicUniformDefinition.AllocateTransient();

	EyeAdaptationMaterial::PopulateUniformBuffer(eyeAdaptationUniformBuffer, frameDelta, settings, exposureScale);

	auto& texProps = curFrame->GetProperties();
	Vector2I texSize = { (i32)texProps.Width, (i32)texProps.Height };

	gEyeAdaptationBasicUniformDefinition.gInputTexSize.Set(uniformBuffer, texSize);

	mEyeAdaptationUniformBufferParameter.Set(eyeAdaptationUniformBuffer);
	mUniformBufferParameter.Set(uniformBuffer);
	mCurrentFrameTextureParameter.Set(curFrame);

	if(prevFrame == nullptr) // Could be that this is the first run
		mPreviousFrameTextureParameter.Set(Texture::kWhite);
	else
		mPreviousFrameTextureParameter.Set(prevFrame);
}

void EyeAdaptationBasicMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation EyeAdaptationBasicMaterial::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

CreateTonemapLUTUniformDefinition gCreateTonemapLUTUniformDefinition;
WhiteBalanceUniformDefinition gWhiteBalanceUniformDefinition;

void CreateTonemap2DLUTMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("WhiteBalanceInput", mWhiteBalanceUniformBufferParameter);
}

void CreateTonemap2DLUTMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", kLutSize);
}

void CreateTonemap2DLUTMaterial::Prepare(const RenderSettings& settings)
{
	GpuBufferSuballocation uniformBuffer = gCreateTonemapLUTUniformDefinition.AllocateTransient();
	GpuBufferSuballocation whiteBalanceUniformBuffer = gWhiteBalanceUniformDefinition.AllocateTransient();

	PopulateTonemappingUniformBuffer(settings, uniformBuffer);
	PopulateWhiteBalanceUniformBuffer(settings, whiteBalanceUniformBuffer);

	mUniformBufferParameter.Set(uniformBuffer);
	mWhiteBalanceUniformBufferParameter.Set(whiteBalanceUniformBuffer);
}

void CreateTonemap2DLUTMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void CreateTonemap2DLUTMaterial::PopulateTonemappingUniformBuffer(const RenderSettings& settings, const GpuBufferSuballocation& uniformBuffer)
{
	// Set parameters
	gCreateTonemapLUTUniformDefinition.gGammaAdjustment.Set(uniformBuffer, 2.2f / settings.Gamma);

	// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
	// gamma when running on Mac (value 2)
	gCreateTonemapLUTUniformDefinition.gGammaCorrectionType.Set(uniformBuffer, 0);

	Vector4 tonemapParams[2];
	tonemapParams[0].X = settings.Tonemapping.FilmicCurveShoulderStrength;
	tonemapParams[0].Y = settings.Tonemapping.FilmicCurveLinearStrength;
	tonemapParams[0].Z = settings.Tonemapping.FilmicCurveLinearAngle;
	tonemapParams[0].W = settings.Tonemapping.FilmicCurveToeStrength;

	tonemapParams[1].X = settings.Tonemapping.FilmicCurveToeNumerator;
	tonemapParams[1].Y = settings.Tonemapping.FilmicCurveToeDenominator;
	tonemapParams[1].Z = settings.Tonemapping.FilmicCurveLinearWhitePoint;
	tonemapParams[1].W = 0.0f; // Unused

	gCreateTonemapLUTUniformDefinition.gTonemapParams.Set(uniformBuffer, tonemapParams[0], 0);
	gCreateTonemapLUTUniformDefinition.gTonemapParams.Set(uniformBuffer, tonemapParams[1], 1);

	// Set color grading params
	gCreateTonemapLUTUniformDefinition.gSaturation.Set(uniformBuffer, settings.ColorGrading.Saturation);
	gCreateTonemapLUTUniformDefinition.gContrast.Set(uniformBuffer, settings.ColorGrading.Contrast);
	gCreateTonemapLUTUniformDefinition.gGain.Set(uniformBuffer, settings.ColorGrading.Gain);
	gCreateTonemapLUTUniformDefinition.gOffset.Set(uniformBuffer, settings.ColorGrading.Offset);
}

void CreateTonemap2DLUTMaterial::PopulateWhiteBalanceUniformBuffer(const RenderSettings& settings, const GpuBufferSuballocation& uniformBuffer)
{
	gWhiteBalanceUniformDefinition.gWhiteTemp.Set(uniformBuffer, settings.WhiteBalance.Temperature);
	gWhiteBalanceUniformDefinition.gWhiteOffset.Set(uniformBuffer, settings.WhiteBalance.Tint);
}

PooledRenderTextureCreateInformation CreateTonemap2DLUTMaterial::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, kLutSize * kLutSize, kLutSize, TU_RENDERTARGET);
}

void CreateTonemap3DLUTMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("WhiteBalanceInput", mWhiteBalanceUniformBufferParameter);
	mGPUParameters->GetStorageTextureParameter("gOutputTex", mOutputTextureParameter);
}

void CreateTonemap3DLUTMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMaterial::kLutSize);
}

void CreateTonemap3DLUTMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& output, const RenderSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferSuballocation uniformBuffer = gCreateTonemapLUTUniformDefinition.AllocateTransient();
	GpuBufferSuballocation whiteBalanceUniformBuffer = gWhiteBalanceUniformDefinition.AllocateTransient();

	CreateTonemap2DLUTMaterial::PopulateTonemappingUniformBuffer(settings, uniformBuffer);
	CreateTonemap2DLUTMaterial::PopulateWhiteBalanceUniformBuffer(settings, whiteBalanceUniformBuffer);

	mUniformBufferParameter.Set(uniformBuffer);
	mWhiteBalanceUniformBufferParameter.Set(whiteBalanceUniformBuffer);

	// Dispatch
	mOutputTextureParameter.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(CreateTonemap2DLUTMaterial::kLutSize / 8, CreateTonemap2DLUTMaterial::kLutSize / 8, CreateTonemap2DLUTMaterial::kLutSize);
}

PooledRenderTextureCreateInformation CreateTonemap3DLUTMaterial::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create3D(PF_RGBA8, CreateTonemap2DLUTMaterial::kLutSize, CreateTonemap2DLUTMaterial::kLutSize, CreateTonemap2DLUTMaterial::kLutSize, TU_LOADSTORE);
}

TonemappingUniformDefinition gTonemappingUniformDefinition;

void TonemappingMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gBloomTex", mBloomTextureParameter);

	if(!mVariationParameters.GetBool("GAMMA_ONLY"))
		mGPUParameters->GetSampledTextureParameter("gColorLUT", mColorLUTParameter);
}

void TonemappingMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMaterial::kLutSize);
}

void TonemappingMaterial::Prepare(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const RenderSettings& settings)
{
	const TextureProperties& texProps = sceneColor->GetProperties();

	GpuBufferSuballocation uniformBuffer = gTonemappingUniformDefinition.AllocateTransient();

	gTonemappingUniformDefinition.gRawGamma.Set(uniformBuffer, 1.0f / settings.Gamma);
	gTonemappingUniformDefinition.gManualExposureScale.Set(uniformBuffer, Math::RaiseToPower(2.0f, settings.ExposureScale));
	gTonemappingUniformDefinition.gTexSize.Set(uniformBuffer, Vector2((float)texProps.Width, (float)texProps.Height));
	gTonemappingUniformDefinition.gBloomTint.Set(uniformBuffer, settings.Bloom.Tint);
	gTonemappingUniformDefinition.gNumSamples.Set(uniformBuffer, texProps.SampleCount);

	mUniformBufferParameter.Set(uniformBuffer);

	mInputTextureParameter.Set(sceneColor);
	mColorLUTParameter.Set(colorLUT);
	mEyeAdaptationTextureParameter.Set(eyeAdaptation);
	mBloomTextureParameter.Set(bloom != nullptr ? bloom : Texture::kBlack);
}

void TonemappingMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
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

BloomClipParamDef gBloomClipParamDef;

void BloomClipMat::Initialize()
{
	mParamBuffer = gBloomClipParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTex);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);
}

void BloomClipMat::Prepare(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation, const RenderSettings& settings)
{
	gBloomClipParamDef.gThreshold.Set(mParamBuffer, threshold);
	gBloomClipParamDef.gManualExposureScale.Set(mParamBuffer, Math::RaiseToPower(2.0f, settings.ExposureScale));

	// Set parameters
	mInputTex.Set(input);
	mEyeAdaptationTex.Set(eyeAdaptation);
}

void BloomClipMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BloomClipMat* BloomClipMat::GetVariation(bool autoExposure)
{
	if(autoExposure)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

ScreenSpaceLensFlareUniformDefinition gScreenSpaceLensFlareUniformDefinition;

void ScreenSpaceLensFlareMaterial::Initialize()
{
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gGradientTex", mGradientTextureParameter);
}

void ScreenSpaceLensFlareMaterial::Prepare(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings)
{
	GpuBufferSuballocation uniformBuffer = gScreenSpaceLensFlareUniformDefinition.AllocateTransient();

	gScreenSpaceLensFlareUniformDefinition.gThreshold.Set(uniformBuffer, settings.Threshold);
	gScreenSpaceLensFlareUniformDefinition.gGhostCount.Set(uniformBuffer, settings.GhostCount);
	gScreenSpaceLensFlareUniformDefinition.gGhostSpacing.Set(uniformBuffer, settings.GhostSpacing);
	gScreenSpaceLensFlareUniformDefinition.gHaloRadius.Set(uniformBuffer, settings.HaloRadius);
	gScreenSpaceLensFlareUniformDefinition.gHaloThickness.Set(uniformBuffer, settings.HaloThickness);
	gScreenSpaceLensFlareUniformDefinition.gHaloThreshold.Set(uniformBuffer, settings.HaloThreshold);
	gScreenSpaceLensFlareUniformDefinition.gHaloAspectRatio.Set(uniformBuffer, settings.HaloAspectRatio);
	gScreenSpaceLensFlareUniformDefinition.gChromaticAberration.Set(uniformBuffer, settings.ChromaticAberrationOffset);

	mUniformBufferParameter.Set(uniformBuffer);

	mInputTextureParameter.Set(input);
	mGradientTextureParameter.Set(RendererTextures::lensFlareGradient);
}

void ScreenSpaceLensFlareMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGPUParameters);
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gFringeTex", mFringeTextureParameter);
}

void ChromaticAberrationMaterial::Prepare(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings)
{
	const TextureProperties& texProps = input->GetProperties();

	GpuBufferSuballocation uniformBuffer = gChromaticAberrationUniformDefinition.AllocateTransient();

	gChromaticAberrationUniformDefinition.gInputSize.Set(uniformBuffer, Vector2((float)texProps.Width, (float)texProps.Height));
	gChromaticAberrationUniformDefinition.gShiftAmount.Set(uniformBuffer, settings.ShiftAmount);

	mUniformBufferParameter.Set(uniformBuffer);

	SPtr<Texture> fringeTex;
	if(settings.FringeTexture)
		fringeTex = settings.FringeTexture;
	else
		fringeTex = RendererTextures::chromaticAberrationFringe;

	mInputTextureParameter.Set(input);
	mFringeTextureParameter.Set(fringeTex);
}

void ChromaticAberrationMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGPUParameters);
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
}

void FilmGrainMaterial::Prepare(const SPtr<Texture>& input, float time, const FilmGrainSettings& settings)
{
	GpuBufferSuballocation uniformBuffer = gFilmGrainUniformDefinition.AllocateTransient();

	gFilmGrainUniformDefinition.gIntensity.Set(uniformBuffer, settings.Intensity);
	gFilmGrainUniformDefinition.gTime.Set(uniformBuffer, settings.Speed * time);

	mUniformBufferParameter.Set(uniformBuffer);

	mInputTextureParameter.Set(input);
}

void FilmGrainMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Render
	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

GaussianBlurUniformDefinition gGaussianBlurUniformDefinition;

void GaussianBlurMaterial::Initialize()
{
	mIsAdditive = mVariationParameters.GetBool("ADDITIVE");

	mGPUParameters->GetUniformBufferParameter("GaussianBlurParams", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	if(mIsAdditive)
		mGPUParameters->GetSampledTextureParameter("gAdditiveTex", mAdditiveTextureParameter);
}

void GaussianBlurMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_NUM_SAMPLES", kMaxBlurSamples);
}

void GaussianBlurMaterial::PrepareDirection(Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint, const SPtr<Texture>& additive)
{
	GpuBufferSuballocation uniformBuffer = gGaussianBlurUniformDefinition.AllocateTransient();
	PopulateUniformBuffer(uniformBuffer, direction, source, filterSize, tint);
	mUniformBufferParameter.Set(uniformBuffer);

	mInputTextureParameter.Set(source);

	if(mIsAdditive)
	{
		if(additive)
			mAdditiveTextureParameter.Set(additive);
		else
			mAdditiveTextureParameter.Set(Texture::kBlack);
	}
}

void GaussianBlurMaterial::ExecutePass(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void GaussianBlurMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination, const Color& tint, const SPtr<Texture>& additive)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = source->GetProperties();
	const RenderTargetProperties& dstProps = destination->GetProperties();

	PooledRenderTextureCreateInformation tempTextureDesc = PooledRenderTextureCreateInformation::Create2D(srcProps.Format, dstProps.Width, dstProps.Height, TU_RENDERTARGET);
	SPtr<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTextureDesc);

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

float GaussianBlurMaterial::CalcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir)
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

void GaussianBlurMaterial::PopulateUniformBuffer(const GpuBufferSuballocation& uniformBuffer, Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint)
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

		gGaussianBlurUniformDefinition.gSampleWeights.Set(uniformBuffer, weight, i);
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

		gGaussianBlurUniformDefinition.gSampleOffsets.Set(uniformBuffer, offset, i);
	}

	gGaussianBlurUniformDefinition.gNumSamples.Set(uniformBuffer, numSamples);
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
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gColorTex", mColorTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;
	samplerStateCreateInformation.AddressMode.U = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.V = TAM_CLAMP;
	samplerStateCreateInformation.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
	SetSamplerState(mGPUParameters, "gColorSamp", "gColorTex", samplerState);
}

void GaussianDOFSeparateMaterial::Prepare(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = color->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferSuballocation uniformBuffer = gGaussianDOFUniformDefinition.AllocateTransient();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniformBuffer, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniformBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniformBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniformBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniformBuffer, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(uniformBuffer);

	mColorTextureParameter.Set(color);
	mDepthTextureParameter.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void GaussianDOFSeparateMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& color)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = color->GetProperties();

	u32 outputWidth = std::max(1U, srcProps.Width / 2);
	u32 outputHeight = std::max(1U, srcProps.Height / 2);

	PooledRenderTextureCreateInformation outputTexDesc = PooledRenderTextureCreateInformation::Create2D(srcProps.Format, outputWidth, outputHeight, TU_RENDERTARGET);
	mOutput0 = GetGpuResourcePool().Get(outputTexDesc);

	bool near = mVariationParameters.GetBool("NEAR");
	bool far = mVariationParameters.GetBool("FAR");

	SPtr<RenderTexture> rt;
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

	RenderPassCreateInformation info(rt, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SPtr<PooledRenderTexture> GaussianDOFSeparateMaterial::GetOutput(u32 idx)
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
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);

	mGPUParameters->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTextureParameter);

	if(mGPUParameters->HasSampledTexture("gNearTex"))
		mGPUParameters->GetSampledTextureParameter("gNearTex", mNearTextureParameter);

	if(mGPUParameters->HasSampledTexture("gFarTex"))
		mGPUParameters->GetSampledTextureParameter("gFarTex", mFarTextureParameter);
}

void GaussianDOFCombineMaterial::Prepare(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = focused->GetProperties();
	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	GpuBufferSuballocation uniformBuffer = gGaussianDOFUniformDefinition.AllocateTransient();

	gGaussianDOFUniformDefinition.gHalfPixelOffset.Set(uniformBuffer, invTexSize * 0.5f);
	gGaussianDOFUniformDefinition.gNearBlurPlane.Set(uniformBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gFarBlurPlane.Set(uniformBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFUniformDefinition.gInvNearBlurRange.Set(uniformBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFUniformDefinition.gInvFarBlurRange.Set(uniformBuffer, 1.0f / settings.FarTransitionRange);

	mUniformBufferParameter.Set(uniformBuffer);

	mFocusedTextureParameter.Set(focused);
	mNearTextureParameter.Set(near);
	mFarTextureParameter.Set(far);
	mDepthTextureParameter.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void GaussianDOFCombineMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

void BokehDOFPrepareMaterial::Prepare(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& srcProps = input->GetProperties();

	GpuBufferSuballocation uniformBuffer = gBokehDOFPrepareUniformDefinition.AllocateTransient();
	GpuBufferSuballocation commonUniformBuffer = gDepthOfFieldCommonUniformDefinition.AllocateTransient();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gBokehDOFPrepareUniformDefinition.gInvInputSize.Set(uniformBuffer, invTexSize);

	BokehDOFMaterial::PopulateDofCommonParams(commonUniformBuffer, settings, view);

	mUniformBufferParameter.Set(uniformBuffer);
	mCommonUniformBufferParameter.Set(commonUniformBuffer);
	mInputTextureParameter.Set(input);
	mDepthTextureParameter.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFPrepareMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
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

PooledRenderTextureCreateInformation BokehDOFPrepareMaterial::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.Width, 2U));
	u32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.Height, 2U));

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureVSParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureFSParameter);
	mGPUParameters->GetSampledTextureParameter("gBokehTex", mBokehTextureParameter);

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

	GpuBufferUtility::Write(mTileVertexBuffer, 0, mTileVertexBuffer->GetTotalSize(), vertexData);
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

	GpuBufferUtility::Write(mTileIndexBuffer, 0, mTileIndexBuffer->GetTotalSize(), indices);
	B3DStackFree(indices);
}

void BokehDOFMaterial::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("QUADS_PER_TILE", kQuadsPerTile);
}

void BokehDOFMaterial::Prepare(const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	const TextureProperties& srcProps = input->GetProperties();
	const RenderTargetProperties& dstProps = output->GetProperties();

	GpuBufferSuballocation uniformBuffer = gBokehDOFUniformDefinition.AllocateTransient();
	GpuBufferSuballocation commonUniformBuffer = gDepthOfFieldCommonUniformDefinition.AllocateTransient();

	Vector2 inputInvTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	Vector2 outputInvTexSize(1.0f / dstProps.Width, 1.0f / dstProps.Height);
	gBokehDOFUniformDefinition.gInvInputSize.Set(uniformBuffer, inputInvTexSize);
	gBokehDOFUniformDefinition.gInvOutputSize.Set(uniformBuffer, outputInvTexSize);
	gBokehDOFUniformDefinition.gAdaptiveThresholdCOC.Set(uniformBuffer, settings.AdaptiveRadiusThreshold);
	gBokehDOFUniformDefinition.gAdaptiveThresholdColor.Set(uniformBuffer, settings.AdaptiveColorThreshold);
	gBokehDOFUniformDefinition.gLayerPixelOffset.Set(uniformBuffer, (i32)srcProps.Height + (i32)kNearFarPadding);
	gBokehDOFUniformDefinition.gInvDepthRange.Set(uniformBuffer, 1.0f / settings.OcclusionDepthRange);

	float bokehSize = settings.MaxBokehSize * srcProps.Width;
	gBokehDOFUniformDefinition.gBokehSize.Set(uniformBuffer, Vector2(bokehSize, bokehSize));

	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	Vector2I tileCount = imageSize / 1;
	gBokehDOFUniformDefinition.gTileCount.Set(uniformBuffer, tileCount);

	PopulateDofCommonParams(commonUniformBuffer, settings, view);

	mUniformBufferParameter.Set(uniformBuffer);
	mCommonUniformBufferParameter.Set(commonUniformBuffer);
	mInputTextureVSParameter.Set(input);
	mInputTextureFSParameter.Set(input);

	SPtr<Texture> bokehTexture = settings.BokehShape;
	if(bokehTexture == nullptr)
		bokehTexture = RendererTextures::bokehFlare;

	mBokehTextureParameter.Set(bokehTexture);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = input->GetProperties();
	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	const Vector2I tileCount = imageSize / 1;

	RenderPassCreateInformation info(output, mGPUParameters, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
	info.ClearMask = RT_COLOR_ALL;
	info.ClearColor = Color::kZero;

	commandBuffer.BeginRenderPass(info);
	commandBuffer.SetVertexDescription(mTileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { mTileVertexBuffer };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(mTileIndexBuffer);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	Bind(commandBuffer);
	const u32 numInstances = Math::DivideAndRoundUp((u32)(tileCount.X * tileCount.Y), kQuadsPerTile);
	commandBuffer.DrawIndexed(0, kQuadsPerTile * 6, 0, kQuadsPerTile * 4, numInstances);

	commandBuffer.EndRenderPass();
}

PooledRenderTextureCreateInformation BokehDOFMaterial::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = rtProps.Width;
	u32 height = rtProps.Height * 2 + kNearFarPadding;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
}

void BokehDOFMaterial::PopulateDofCommonParams(const GpuBufferSuballocation& uniformBuffer, const DepthOfFieldSettings& settings, const RendererView& view)
{
	gDepthOfFieldCommonUniformDefinition.gFocalPlaneDistance.Set(uniformBuffer, settings.FocalDistance);
	gDepthOfFieldCommonUniformDefinition.gApertureSize.Set(uniformBuffer, settings.ApertureSize * 0.001f); // mm to m
	gDepthOfFieldCommonUniformDefinition.gFocalLength.Set(uniformBuffer, settings.FocalLength * 0.001f); // mm to m
	gDepthOfFieldCommonUniformDefinition.gInFocusRange.Set(uniformBuffer, settings.FocalRange);
	gDepthOfFieldCommonUniformDefinition.gNearTransitionRegion.Set(uniformBuffer, settings.NearTransitionRange);
	gDepthOfFieldCommonUniformDefinition.gFarTransitionRegion.Set(uniformBuffer, settings.FarTransitionRange);

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

	gDepthOfFieldCommonUniformDefinition.gSensorSize.Set(uniformBuffer, sensorSize);
	gDepthOfFieldCommonUniformDefinition.gImageSize.Set(uniformBuffer, imageSize);
	gDepthOfFieldCommonUniformDefinition.gMaxBokehSize.Set(uniformBuffer, Math::Clamp01(settings.MaxBokehSize) * imageSize);
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetUniformBufferParameter("DepthOfFieldParams", mCommonUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gUnfocusedTex", mUnfocusedTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gFocusedTex", mFocusedTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mDepthTextureParameter);
}

void BokehDOFCombineMaterial::Prepare(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	const TextureProperties& focusedProps = focused->GetProperties();
	const TextureProperties& unfocusedProps = unfocused->GetProperties();
	u32 halfHeight = std::max(1U, Math::DivideAndRoundUp(focusedProps.Height, 2U));

	GpuBufferSuballocation uniformBuffer = gBokehDOFCombineUniformDefinition.AllocateTransient();
	GpuBufferSuballocation commonUniformBuffer = gDepthOfFieldCommonUniformDefinition.AllocateTransient();

	float uvScale = halfHeight / (float)unfocusedProps.Height;
	float uvOffset = (halfHeight + BokehDOFMaterial::kNearFarPadding) / (float)unfocusedProps.Height;

	Vector2 layerScaleOffset(uvScale, uvOffset);
	Vector2 focusedImageSize((float)focusedProps.Width, (float)focusedProps.Height);
	gBokehDOFCombineUniformDefinition.gLayerAndScaleOffset.Set(uniformBuffer, layerScaleOffset);
	gBokehDOFCombineUniformDefinition.gFocusedImageSize.Set(uniformBuffer, focusedImageSize);

	BokehDOFMaterial::PopulateDofCommonParams(commonUniformBuffer, settings, view);

	mUniformBufferParameter.Set(uniformBuffer);
	mCommonUniformBufferParameter.Set(commonUniformBuffer);
	mUnfocusedTextureParameter.Set(unfocused);
	mFocusedTextureParameter.Set(focused);
	mDepthTextureParameter.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void BokehDOFCombineMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
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

MotionBlurParamDef gMotionBlurParamDef;

void MotionBlurMat::Initialize()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);

	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mDepthTexture);

	SamplerStateInformation pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> pointSampState = mGpuDevice->FindOrCreateSamplerState(pointSampDesc);

	if(mGPUParameters->HasSamplerState("gDepthBufferSamp"))
		mGPUParameters->SetSamplerState("gDepthBufferSamp", pointSampState);
}

void MotionBlurMat::Prepare(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings)
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

	gMotionBlurParamDef.gHalfNumSamples.Set(mParamBuffer, numSamples / 2);

	mInputTexture.Set(input);
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void MotionBlurMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BuildHiZFParamDef gBuildHiZParamDef;

void BuildHiZMat::Initialize()
{
	mNoTextureViews = mVariationParameters.GetBool("NO_TEXTURE_VIEWS");

	mGPUParameters->GetSampledTextureParameter("gDepthTex", mInputTexture);

	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mParamBuffer = gBuildHiZParamDef.CreateBuffer();
		mGPUParameters->SetUniformBuffer("Input", mParamBuffer);

		SamplerStateInformation inputSampDesc;
		inputSampDesc.MinFilter = FO_POINT;
		inputSampDesc.MagFilter = FO_POINT;
		inputSampDesc.MipFilter = FO_POINT;

		SPtr<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
		SetSamplerState(mGPUParameters, "gDepthSamp", "gDepthTex", inputSampState);
	}
}

void BuildHiZMat::Prepare(const SPtr<Texture>& source, u32 srcMip)
{
	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mInputTexture.Set(source);

		auto& props = source->GetProperties();
		float pixelWidth = (float)props.Width;
		float pixelHeight = (float)props.Height;

		Vector2 halfPixelOffset(0.5f / pixelWidth, 0.5f / pixelHeight);

		gBuildHiZParamDef.gHalfPixelOffset.Set(mParamBuffer, halfPixelOffset);
		gBuildHiZParamDef.gMipLevel.Set(mParamBuffer, srcMip);
	}
	else
		mInputTexture.Set(source, TextureSurface(srcMip));
}

void BuildHiZMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& output, const Area2& srcRect, const Area2& dstRect)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	commandBuffer.SetViewport(dstRect);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, srcRect);

	commandBuffer.SetViewport(Area2(0, 0, 1, 1));
	commandBuffer.EndRenderPass();
}

BuildHiZMat* BuildHiZMat::GetVariation(bool noTextureViews)
{
	if(noTextureViews)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

FXAAParamDef gFXAAParamDef;

void FXAAMat::Initialize()
{
	mParamBuffer = gFXAAParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void FXAAMat::Prepare(const SPtr<Texture>& source)
{
	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gFXAAParamDef.gInvTexSize.Set(mParamBuffer, invTexSize);

	mInputTexture.Set(source);
}

void FXAAMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOParamDef gSSAOParamDef;

void SSAOMat::Initialize()
{
	bool isFinal = mVariationParameters.GetBool("FINAL_AO");
	bool mixWithUpsampled = mVariationParameters.GetBool("MIX_WITH_UPSAMPLED");

	mParamBuffer = gSSAOParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);

	if(isFinal)
	{
		mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexture);
		mGPUParameters->GetSampledTextureParameter("gNormalsTex", mNormalsTexture);
	}

	if(!isFinal || mixWithUpsampled)
		mGPUParameters->GetSampledTextureParameter("gSetupAO", mSetupAOTexture);

	if(mixWithUpsampled)
		mGPUParameters->GetSampledTextureParameter("gDownsampledAO", mDownsampledAOTexture);

	mGPUParameters->GetSampledTextureParameter("gRandomTex", mRandomTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGPUParameters->HasSamplerState("gInputSamp"))
		mGPUParameters->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		if(isFinal)
		{
			mGPUParameters->SetSamplerState("gDepthTex", inputSampState);
			mGPUParameters->SetSamplerState("gNormalsTex", inputSampState);
		}

		if(!isFinal || mixWithUpsampled)
			mGPUParameters->SetSamplerState("gSetupAO", inputSampState);

		if(mixWithUpsampled)
			mGPUParameters->SetSamplerState("gDownsampledAO", inputSampState);
	}

	SamplerStateInformation randomSampDesc;
	randomSampDesc.MinFilter = FO_POINT;
	randomSampDesc.MagFilter = FO_POINT;
	randomSampDesc.MipFilter = FO_POINT;
	randomSampDesc.AddressMode.U = TAM_WRAP;
	randomSampDesc.AddressMode.V = TAM_WRAP;
	randomSampDesc.AddressMode.W = TAM_WRAP;

	SPtr<SamplerState> randomSampState = mGpuDevice->FindOrCreateSamplerState(randomSampDesc);
	SetSamplerState(mGPUParameters, "gRandomSamp", "gRandomTex", randomSampState);
}

void SSAOMat::Prepare(const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
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

	gSSAOParamDef.gSampleRadius.Set(mParamBuffer, radius);
	gSSAOParamDef.gCotHalfFOV.Set(mParamBuffer, cotHalfFOV);
	gSSAOParamDef.gTanHalfFOV.Set(mParamBuffer, tanHalfFOV);
	gSSAOParamDef.gWorldSpaceRadiusMask.Set(mParamBuffer, 1.0f);
	gSSAOParamDef.gBias.Set(mParamBuffer, (settings.Bias * viewScale) / 1000.0f);
	gSSAOParamDef.gFadeMultiplyAdd.Set(mParamBuffer, fadeMultiplyAdd);
	gSSAOParamDef.gPower.Set(mParamBuffer, settings.Power);
	gSSAOParamDef.gIntensity.Set(mParamBuffer, settings.Intensity);

	bool upsample = mVariationParameters.GetBool("MIX_WITH_UPSAMPLED");
	if(upsample)
	{
		const TextureProperties& props = textures.AoDownsampled->GetProperties();

		Vector2 downsampledPixelSize;
		downsampledPixelSize.X = 1.0f / props.Width;
		downsampledPixelSize.Y = 1.0f / props.Height;

		gSSAOParamDef.gDownsampledPixelSize.Set(mParamBuffer, downsampledPixelSize);
	}

	// Generate a scale which we need to use in order to achieve tiling
	const TextureProperties& rndProps = textures.RandomRotations->GetProperties();
	u32 rndWidth = rndProps.Width;
	u32 rndHeight = rndProps.Height;

	//// Multiple of random texture size, rounded up
	u32 scaleWidth = (rtProps.Width + rndWidth - 1) / rndWidth;
	u32 scaleHeight = (rtProps.Height + rndHeight - 1) / rndHeight;

	Vector2 randomTileScale((float)scaleWidth, (float)scaleHeight);
	gSSAOParamDef.gRandomTileScale.Set(mParamBuffer, randomTileScale);

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

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void SSAOMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOMat* SSAOMat::GetVariation(bool upsample, bool finalPass, int quality)
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

SSAODownsampleParamDef gSSAODownsampleParamDef;

void SSAODownsampleMat::Initialize()
{
	mParamBuffer = gSSAODownsampleParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexture);
	mGPUParameters->GetSampledTextureParameter("gNormalsTex", mNormalsTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_LINEAR;
	inputSampDesc.MagFilter = FO_LINEAR;
	inputSampDesc.MipFilter = FO_LINEAR;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);

	if(mGPUParameters->HasSamplerState("gInputSamp"))
		mGPUParameters->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		mGPUParameters->SetSamplerState("gDepthTex", inputSampState);
		mGPUParameters->SetSamplerState("gNormalsTex", inputSampState);
	}
}

void SSAODownsampleMat::Prepare(const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<RenderTexture>& destination, float depthRange)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	const RenderTargetProperties& rtProps = destination->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / rtProps.Width;
	pixelSize.Y = 1.0f / rtProps.Height;

	float scale = viewProps.Target.ViewRect.Width / (float)rtProps.Width;

	gSSAODownsampleParamDef.gPixelSize.Set(mParamBuffer, pixelSize);
	gSSAODownsampleParamDef.gInvDepthThreshold.Set(mParamBuffer, (1.0f / depthRange) / scale);

	mDepthTexture.Set(depth);
	mNormalsTexture.Set(normals);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void SSAODownsampleMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOBlurParamDef gSSAOBlurParamDef;

void SSAOBlurMat::Initialize()
{
	mParamBuffer = gSSAOBlurParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mAOTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexture);

	SamplerStateInformation inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = mGpuDevice->FindOrCreateSamplerState(inputSampDesc);
	if(mGPUParameters->HasSamplerState("gInputSamp"))
		mGPUParameters->SetSamplerState("gInputSamp", inputSampState);
	else
	{
		mGPUParameters->SetSamplerState("gInputTex", inputSampState);
		mGPUParameters->SetSamplerState("gDepthTex", inputSampState);
	}
}

void SSAOBlurMat::Prepare(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& sceneDepth, float depthRange)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& texProps = ao->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / texProps.Width;
	pixelSize.Y = 1.0f / texProps.Height;

	Vector2 pixelOffset(BsZero);
	if(mVariationParameters.GetBool("DIR_HORZ"))
		pixelOffset.X = pixelSize.X;
	else
		pixelOffset.Y = pixelSize.Y;

	float scale = viewProps.Target.ViewRect.Width / (float)texProps.Width;

	gSSAOBlurParamDef.gPixelSize.Set(mParamBuffer, pixelSize);
	gSSAOBlurParamDef.gPixelOffset.Set(mParamBuffer, pixelOffset);
	gSSAOBlurParamDef.gInvDepthThreshold.Set(mParamBuffer, (1.0f / depthRange) / scale);

	mAOTexture.Set(ao);
	mDepthTexture.Set(sceneDepth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void SSAOBlurMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGPUParameters);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

SSAOBlurMat* SSAOBlurMat::GetVariation(bool horizontal)
{
	if(horizontal)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

SSRStencilUniformDefinition gSSRStencilUniformDefinition;

void SSRStencilMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetUniformBufferParameter("Input", mUniformBufferParameter);
}

void SSRStencilMaterial::Prepare(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings)
{
	mGBufferParams.Bind(gbuffer);

	Vector2 roughnessScaleBias = SSRTraceMaterial::CalcRoughnessFadeScaleBias(settings.MaxRoughness);

	GpuBufferSuballocation uniformBuffer = gSSRStencilUniformDefinition.AllocateTransient();
	gSSRStencilUniformDefinition.gRoughnessScaleBias.Set(uniformBuffer, roughnessScaleBias);
	mUniformBufferParameter.Set(uniformBuffer);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
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
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);

	mGPUParameters->TryGetUniformBufferParameter("Input", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gSceneColor", mSceneColorTextureParameter);
	mGPUParameters->GetSampledTextureParameter("gHiZ", mHiZTextureParameter);

	SamplerStateInformation desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> hiZSamplerState = mGpuDevice->FindOrCreateSamplerState(desc);
	if(mGPUParameters->HasSamplerState("gHiZSamp"))
		mGPUParameters->SetSamplerState("gHiZSamp", hiZSamplerState);
	else if(mGPUParameters->HasSamplerState("gHiZ"))
		mGPUParameters->SetSamplerState("gHiZ", hiZSamplerState);
}

void SSRTraceMaterial::Prepare(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings)
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

	GpuBufferSuballocation uniformBuffer = gSSRTraceUniformDefinition.AllocateTransient();

	Vector2I bufferSize(viewRect.Width, viewRect.Height);
	gSSRTraceUniformDefinition.gHiZSize.Set(uniformBuffer, bufferSize);
	gSSRTraceUniformDefinition.gHiZNumMips.Set(uniformBuffer, hiZProps.MipMapCount);
	gSSRTraceUniformDefinition.gNDCToHiZUV.Set(uniformBuffer, ndcToHiZUV);
	gSSRTraceUniformDefinition.gHiZUVToScreenUV.Set(uniformBuffer, HiZUVToScreenUV);
	gSSRTraceUniformDefinition.gIntensity.Set(uniformBuffer, settings.Intensity);
	gSSRTraceUniformDefinition.gRoughnessScaleBias.Set(uniformBuffer, roughnessScaleBias);
	gSSRTraceUniformDefinition.gTemporalJitter.Set(uniformBuffer, temporalJitter);

	mUniformBufferParameter.Set(uniformBuffer);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void SSRTraceMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& destination, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGPUParameters, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
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

TemporalResolveParamDef gTemporalResolveParamDef;
TemporalFilteringParamDef gTemporalFilteringParamDef;

void TemporalFilteringMat::Initialize()
{
	mParamBuffer = gTemporalFilteringParamDef.CreateBuffer();
	mTemporalParamBuffer = gTemporalResolveParamDef.CreateBuffer();

	mGPUParameters->GetSampledTextureParameter("gSceneDepth", mSceneDepthTexture);
	mGPUParameters->GetSampledTextureParameter("gSceneColor", mSceneColorTexture);
	mGPUParameters->GetSampledTextureParameter("gPrevColor", mPrevColorTexture);

	mHasVelocityTexture = mVariationParameters.GetBool("PER_PIXEL_VELOCITY");
	if(mHasVelocityTexture)
		mGPUParameters->GetSampledTextureParameter("gVelocity", mVelocityTexture);

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->SetUniformBuffer("TemporalInput", mTemporalParamBuffer);

	SamplerStateInformation pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> pointSampState = mGpuDevice->FindOrCreateSamplerState(pointSampDesc);

	if(mGPUParameters->HasSamplerState("gPointSampler"))
		mGPUParameters->SetSamplerState("gPointSampler", pointSampState);
	else
		mGPUParameters->SetSamplerState("gSceneDepth", pointSampState);

	SamplerStateInformation linearSampDesc;
	linearSampDesc.MinFilter = FO_POINT;
	linearSampDesc.MagFilter = FO_POINT;
	linearSampDesc.MipFilter = FO_POINT;
	linearSampDesc.AddressMode.U = TAM_CLAMP;
	linearSampDesc.AddressMode.V = TAM_CLAMP;
	linearSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> linearSampState = mGpuDevice->FindOrCreateSamplerState(linearSampDesc);
	if(mGPUParameters->HasSamplerState("gLinearSampler"))
		mGPUParameters->SetSamplerState("gLinearSampler", linearSampState);
	else
	{
		mGPUParameters->SetSamplerState("gSceneColor", linearSampState);
		mGPUParameters->SetSamplerState("gPrevColor", linearSampState);
	}
}

void TemporalFilteringMat::Prepare(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter, float exposure)
{
	SPtr<Texture> velocityTex = velocity;
	if(!velocityTex)
		velocityTex = Texture::kBlack;

	mPrevColorTexture.Set(prevFrame);
	mSceneColorTexture.Set(curFrame);
	mSceneDepthTexture.Set(sceneDepth);

	if(mHasVelocityTexture)
		mVelocityTexture.Set(velocityTex);

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

	gTemporalFilteringParamDef.gSceneColorTexelSize.Set(mParamBuffer, colorPixelSize);
	gTemporalFilteringParamDef.gSceneDepthTexelSize.Set(mParamBuffer, depthPixelSize);
	gTemporalFilteringParamDef.gVelocityTexelSize.Set(mParamBuffer, velocityPixelSize);
	gTemporalFilteringParamDef.gManualExposure.Set(mParamBuffer, 1.0f / exposure);

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
		gTemporalResolveParamDef.gSampleWeights.Set(mTemporalParamBuffer, sampleWeights[i] / totalWeights, i);
		gTemporalResolveParamDef.gSampleWeightsLowpass.Set(mTemporalParamBuffer, sampleWeightsLowPass[i] / totalWeightsLowPass, i);
	}

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void TemporalFilteringMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<RenderTarget>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(destination, mGPUParameters);
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

TemporalFilteringMat* TemporalFilteringMat::GetVariation(TemporalFilteringType type, bool velocity, bool msaa)
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
	mGPUParameters->GetUniformBufferParameter("Params", mUniformBufferParameter);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureParameter);

	SamplerStateInformation sampDesc;
	sampDesc.MinFilter = FO_POINT;
	sampDesc.MagFilter = FO_POINT;
	sampDesc.MipFilter = FO_POINT;
	sampDesc.AddressMode.U = TAM_CLAMP;
	sampDesc.AddressMode.V = TAM_CLAMP;
	sampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(sampDesc);
	SetSamplerState(mGPUParameters, "gInputSamp", "gInputTex", samplerState);
}

void EncodeDepthMaterial::Prepare(const SPtr<Texture>& depth, float near, float far)
{
	GpuBufferSuballocation uniformBuffer = gEncodeDepthUniformDefinition.AllocateTransient();
	gEncodeDepthUniformDefinition.gNear.Set(uniformBuffer, near);
	gEncodeDepthUniformDefinition.gFar.Set(uniformBuffer, far);

	mInputTextureParameter.Set(depth);
	mUniformBufferParameter.Set(uniformBuffer);
}

void EncodeDepthMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGPUParameters, RT_NONE, RT_COLOR0);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

void MSAACoverageMat::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
}

void MSAACoverageMat::Prepare(const RendererView& view, GBufferTextures gbuffer)
{
	mGBufferParams.Bind(gbuffer);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);
}

void MSAACoverageMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}

MSAACoverageMat* MSAACoverageMat::GetVariation(u32 msaaCount)
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

void MSAACoverageStencilMat::Initialize()
{
	mGPUParameters->GetSampledTextureParameter("gMSAACoverage", mCoverageTexParam);
}

void MSAACoverageStencilMat::Prepare(const SPtr<Texture>& coverage)
{
	mCoverageTexParam.Set(coverage);
}

void MSAACoverageStencilMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}
}}
