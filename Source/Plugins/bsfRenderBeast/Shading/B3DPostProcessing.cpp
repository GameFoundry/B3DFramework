//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPostProcessing.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Renderer/B3DRendererUtility.h"
#include "Components/B3DCamera.h"
#include "Material/B3DGpuParamsSet.h"
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

DownsampleParamDef gDownsampleParamDef;

void DownsampleMat::Initialize()
{
	mParamBuffer = gDownsampleParamDef.CreateBuffer();

	if(mGPUParameters->HasUniformBuffer("Input"))
		mGPUParameters->SetUniformBuffer("Input", mParamBuffer);

	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
}

void DownsampleMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mInputTexture.Set(input);

	const TextureProperties& rtProps = input->GetProperties();

	bool MSAA = mVariationParameters.GetI32("MSAA") > 0;
	if(MSAA)
	{
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(-1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(-1.0f, 1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(1.0f, 1.0f));
	}
	else
	{
		Vector2 invTextureSize(1.0f / rtProps.Width, 1.0f / rtProps.Height);

		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(-1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(-1.0f, 1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(1.0f, 1.0f));
	}

	commandBuffer.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind(commandBuffer);

	if(MSAA)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)rtProps.Width, (float)rtProps.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

PooledRenderTextureCreateInformation DownsampleMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1, Math::CeilToInt(rtProps.Width * 0.5f));
	u32 height = std::max(1, Math::CeilToInt(rtProps.Height * 0.5f));

	return PooledRenderTextureCreateInformation::Create2D(rtProps.Format, width, height, TU_RENDERTARGET);
}

DownsampleMat* DownsampleMat::GetVariation(u32 quality, bool msaa)
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

EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

void EyeAdaptHistogramMat::Initialize()
{
	mParamBuffer = gEyeAdaptHistogramParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gSceneColorTex", mSceneColor);
	mGPUParameters->GetStorageTextureParameter("gOutputTex", mOutputTex);
}

void EyeAdaptHistogramMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", kThreadGroupSizeY);
	defines.Set("LOOP_COUNT_X", kLoopCountX);
	defines.Set("LOOP_COUNT_Y", kLoopCountY);
}

void EyeAdaptHistogramMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mSceneColor.Set(input);

	const TextureProperties& props = input->GetProperties();
	Vector4I offsetAndSize(0, 0, (i32)props.Width, (i32)props.Height);

	gEyeAdaptHistogramParamDef.gHistogramParams.Set(mParamBuffer, GetHistogramScaleOffset(settings));
	gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.Set(mParamBuffer, offsetAndSize);

	Vector2I threadGroupCount = GetThreadGroupCount(input);
	gEyeAdaptHistogramParamDef.gThreadGroupCount.Set(mParamBuffer, threadGroupCount);

	// Dispatch
	mOutputTex.Set(output);

	Bind(commandBuffer);

	commandBuffer.DispatchCompute(threadGroupCount.X, threadGroupCount.Y);
}

PooledRenderTextureCreateInformation EyeAdaptHistogramMat::GetOutputDesc(const SPtr<Texture>& target)
{
	Vector2I threadGroupCount = GetThreadGroupCount(target);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, kHistogramNumTexels, numHistograms, TU_LOADSTORE);
}

Vector2I EyeAdaptHistogramMat::GetThreadGroupCount(const SPtr<Texture>& target)
{
	const u32 texelsPerThreadGroupX = kThreadGroupSizeX * kLoopCountX;
	const u32 texelsPerThreadGroupY = kThreadGroupSizeY * kLoopCountY;

	const TextureProperties& props = target->GetProperties();

	Vector2I threadGroupCount;
	threadGroupCount.X = ((i32)props.Width + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
	threadGroupCount.Y = ((i32)props.Height + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

	return threadGroupCount;
}

Vector2 EyeAdaptHistogramMat::GetHistogramScaleOffset(const AutoExposureSettings& settings)
{
	float diff = settings.HistogramLog2Max - settings.HistogramLog2Min;
	float scale = 1.0f / diff;
	float offset = -settings.HistogramLog2Min * scale;

	return Vector2(scale, offset);
}

EyeAdaptHistogramReduceParamDef gEyeAdaptHistogramReduceParamDef;

void EyeAdaptHistogramReduceMat::Initialize()
{
	mParamBuffer = gEyeAdaptHistogramReduceParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gHistogramTex", mHistogramTex);
	mGPUParameters->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTex);
}

void EyeAdaptHistogramReduceMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mHistogramTex.Set(histogram);

	SPtr<Texture> eyeAdaptationTex;
	if(prevFrame == nullptr) // Could be that this is the first run
		eyeAdaptationTex = Texture::kWhite;
	else
		eyeAdaptationTex = prevFrame;

	mEyeAdaptationTex.Set(eyeAdaptationTex);

	Vector2I threadGroupCount = EyeAdaptHistogramMat::GetThreadGroupCount(sceneColor);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	gEyeAdaptHistogramReduceParamDef.gThreadGroupCount.Set(mParamBuffer, numHistograms);

	commandBuffer.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind(commandBuffer);

	Area2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::kHistogramNumTexels, 2.0f);
	GetRendererUtility().DrawScreenQuad(commandBuffer, drawUV);

	commandBuffer.SetRenderTarget(nullptr);
}

PooledRenderTextureCreateInformation EyeAdaptHistogramReduceMat::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, EyeAdaptHistogramMat::kHistogramNumTexels, 2, TU_RENDERTARGET);
}

EyeAdaptationParamDef gEyeAdaptationParamDef;

void EyeAdaptationMat::Initialize()
{
	mParamBuffer = gEyeAdaptationParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("EyeAdaptationParams", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gHistogramTex", mReducedHistogramTex);
}

void EyeAdaptationMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::kThreadGroupSizeY);
}

void EyeAdaptationMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mReducedHistogramTex.Set(reducedHistogram);

	PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

	// Render
	commandBuffer.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

PooledRenderTextureCreateInformation EyeAdaptationMat::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

void EyeAdaptationMat::PopulateParams(const SPtr<GpuBuffer>& paramBuffer, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	Vector2 histogramScaleAndOffset = EyeAdaptHistogramMat::GetHistogramScaleOffset(settings);

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

	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[0], 0);
	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[1], 1);
	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[2], 2);
}

void EyeAdaptationBasicSetupMat::Initialize()
{
	mParamBuffer = gEyeAdaptationParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("EyeAdaptationParams", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);

	SamplerStateCreateInformation samplerStateCreateInformation;
	samplerStateCreateInformation.MinFilter = FO_POINT;
	samplerStateCreateInformation.MagFilter = FO_POINT;
	samplerStateCreateInformation.MipFilter = FO_POINT;

	SPtr<SamplerState> samplerState = mGpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
	SetSamplerState(mGPUParameters, "gInputSamp", "gInputTex", samplerState);
}

void EyeAdaptationBasicSetupMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mInputTex.Set(input);

	EyeAdaptationMat::PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

PooledRenderTextureCreateInformation EyeAdaptationBasicSetupMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, props.Width, props.Height, TU_RENDERTARGET);
}

EyeAdaptationBasicParamsMatDef gEyeAdaptationBasicParamsMatDef;

void EyeAdaptationBasicMat::Initialize()
{
	mEyeAdaptationParamsBuffer = gEyeAdaptationParamDef.CreateBuffer();
	mParamsBuffer = gEyeAdaptationBasicParamsMatDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("EyeAdaptationParams", mEyeAdaptationParamsBuffer);
	mGPUParameters->SetUniformBuffer("Input", mParamsBuffer);
	mGPUParameters->GetSampledTextureParameter("gCurFrameTex", mCurFrameTexParam);
	mGPUParameters->GetSampledTextureParameter("gPrevFrameTex", mPrevFrameTexParam);
}

void EyeAdaptationBasicMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	mCurFrameTexParam.Set(curFrame);

	if(prevFrame == nullptr) // Could be that this is the first run
		mPrevFrameTexParam.Set(Texture::kWhite);
	else
		mPrevFrameTexParam.Set(prevFrame);

	EyeAdaptationMat::PopulateParams(mEyeAdaptationParamsBuffer, frameDelta, settings, exposureScale);

	auto& texProps = curFrame->GetProperties();
	Vector2I texSize = { (i32)texProps.Width, (i32)texProps.Height };

	gEyeAdaptationBasicParamsMatDef.gInputTexSize.Set(mParamsBuffer, texSize);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

PooledRenderTextureCreateInformation EyeAdaptationBasicMat::GetOutputDesc()
{
	return PooledRenderTextureCreateInformation::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;
WhiteBalanceParamDef gWhiteBalanceParamDef;

void CreateTonemap2DLUTMat::Initialize()
{
	mParamBuffer = gCreateTonemapLUTParamDef.CreateBuffer();
	mWhiteBalanceParamBuffer = gWhiteBalanceParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->SetUniformBuffer("WhiteBalanceInput", mWhiteBalanceParamBuffer);
}

void CreateTonemap2DLUTMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", kLutSize);
}

void CreateTonemap2DLUTMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& output, const RenderSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	PopulateTonemappingParameterBuffer(settings, mParamBuffer);
	PopulateWhiteBalanceParameterBuffer(settings, mWhiteBalanceParamBuffer);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.SetRenderTarget(nullptr);
}

void CreateTonemap2DLUTMat::PopulateTonemappingParameterBuffer(const RenderSettings& settings, const SPtr<GpuBuffer>& parameterBuffer)
{
	// Set parameters
	gCreateTonemapLUTParamDef.gGammaAdjustment.Set(parameterBuffer, 2.2f / settings.Gamma);

	// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
	// gamma when running on Mac (value 2)
	gCreateTonemapLUTParamDef.gGammaCorrectionType.Set(parameterBuffer, 0);

	Vector4 tonemapParams[2];
	tonemapParams[0].X = settings.Tonemapping.FilmicCurveShoulderStrength;
	tonemapParams[0].Y = settings.Tonemapping.FilmicCurveLinearStrength;
	tonemapParams[0].Z = settings.Tonemapping.FilmicCurveLinearAngle;
	tonemapParams[0].W = settings.Tonemapping.FilmicCurveToeStrength;

	tonemapParams[1].X = settings.Tonemapping.FilmicCurveToeNumerator;
	tonemapParams[1].Y = settings.Tonemapping.FilmicCurveToeDenominator;
	tonemapParams[1].Z = settings.Tonemapping.FilmicCurveLinearWhitePoint;
	tonemapParams[1].W = 0.0f; // Unused

	gCreateTonemapLUTParamDef.gTonemapParams.Set(parameterBuffer, tonemapParams[0], 0);
	gCreateTonemapLUTParamDef.gTonemapParams.Set(parameterBuffer, tonemapParams[1], 1);

	// Set color grading params
	gCreateTonemapLUTParamDef.gSaturation.Set(parameterBuffer, settings.ColorGrading.Saturation);
	gCreateTonemapLUTParamDef.gContrast.Set(parameterBuffer, settings.ColorGrading.Contrast);
	gCreateTonemapLUTParamDef.gGain.Set(parameterBuffer, settings.ColorGrading.Gain);
	gCreateTonemapLUTParamDef.gOffset.Set(parameterBuffer, settings.ColorGrading.Offset);
}

void CreateTonemap2DLUTMat::PopulateWhiteBalanceParameterBuffer(const RenderSettings& settings, const SPtr<GpuBuffer>& parameterBuffer)
{
	gWhiteBalanceParamDef.gWhiteTemp.Set(parameterBuffer, settings.WhiteBalance.Temperature);
	gWhiteBalanceParamDef.gWhiteOffset.Set(parameterBuffer, settings.WhiteBalance.Tint);
}

PooledRenderTextureCreateInformation CreateTonemap2DLUTMat::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA8, kLutSize * kLutSize, kLutSize, TU_RENDERTARGET);
}

void CreateTonemap3DLUTMat::Initialize()
{
	mParamBuffer = gCreateTonemapLUTParamDef.CreateBuffer();
	mWhiteBalanceParamBuffer = gWhiteBalanceParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->SetUniformBuffer("WhiteBalanceInput", mWhiteBalanceParamBuffer);

	mGPUParameters->GetStorageTextureParameter("gOutputTex", mOutputTex);
}

void CreateTonemap3DLUTMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMat::kLutSize);
}

void CreateTonemap3DLUTMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& output, const RenderSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	CreateTonemap2DLUTMat::PopulateTonemappingParameterBuffer(settings, mParamBuffer);
	CreateTonemap2DLUTMat::PopulateWhiteBalanceParameterBuffer(settings, mWhiteBalanceParamBuffer);

	// Dispatch
	mOutputTex.Set(output);

	Bind(commandBuffer);
	commandBuffer.DispatchCompute(CreateTonemap2DLUTMat::kLutSize / 8, CreateTonemap2DLUTMat::kLutSize / 8, CreateTonemap2DLUTMat::kLutSize);
}

PooledRenderTextureCreateInformation CreateTonemap3DLUTMat::GetOutputDesc() const
{
	return PooledRenderTextureCreateInformation::Create3D(PF_RGBA8, CreateTonemap2DLUTMat::kLutSize, CreateTonemap2DLUTMat::kLutSize, CreateTonemap2DLUTMat::kLutSize, TU_LOADSTORE);
}

TonemappingParamDef gTonemappingParamDef;

void TonemappingMat::Initialize()
{
	mParamBuffer = gTonemappingParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gEyeAdaptationTex", mEyeAdaptationTex);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);
	mGPUParameters->GetSampledTextureParameter("gBloomTex", mBloomTex);

	if(!mVariationParameters.GetBool("GAMMA_ONLY"))
		mGPUParameters->GetSampledTextureParameter("gColorLUT", mColorLUT);
}

void TonemappingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemap2DLUTMat::kLutSize);
}

void TonemappingMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output, const RenderSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& texProps = sceneColor->GetProperties();

	gTonemappingParamDef.gRawGamma.Set(mParamBuffer, 1.0f / settings.Gamma);
	gTonemappingParamDef.gManualExposureScale.Set(mParamBuffer, Math::RaiseToPower(2.0f, settings.ExposureScale));
	gTonemappingParamDef.gTexSize.Set(mParamBuffer, Vector2((float)texProps.Width, (float)texProps.Height));
	gTonemappingParamDef.gBloomTint.Set(mParamBuffer, settings.Bloom.Tint);
	gTonemappingParamDef.gNumSamples.Set(mParamBuffer, texProps.SampleCount);

	// Set parameters
	mInputTex.Set(sceneColor);
	mColorLUT.Set(colorLUT);
	mEyeAdaptationTex.Set(eyeAdaptation);
	mBloomTex.Set(bloom != nullptr ? bloom : Texture::kBlack);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

TonemappingMat* TonemappingMat::GetVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA)
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

void BloomClipMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation, const RenderSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	gBloomClipParamDef.gThreshold.Set(mParamBuffer, threshold);
	gBloomClipParamDef.gManualExposureScale.Set(mParamBuffer, Math::RaiseToPower(2.0f, settings.ExposureScale));

	// Set parameters
	mInputTex.Set(input);
	mEyeAdaptationTex.Set(eyeAdaptation);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

BloomClipMat* BloomClipMat::GetVariation(bool autoExposure)
{
	if(autoExposure)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

ScreenSpaceLensFlareParamDef gScreenSpaceLensFlareParamDef;

void ScreenSpaceLensFlareMat::Initialize()
{
	mParamBuffer = gScreenSpaceLensFlareParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);
	mGPUParameters->GetSampledTextureParameter("gGradientTex", mGradientTex);
}

void ScreenSpaceLensFlareMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	gScreenSpaceLensFlareParamDef.gThreshold.Set(mParamBuffer, settings.Threshold);
	gScreenSpaceLensFlareParamDef.gGhostCount.Set(mParamBuffer, settings.GhostCount);
	gScreenSpaceLensFlareParamDef.gGhostSpacing.Set(mParamBuffer, settings.GhostSpacing);
	gScreenSpaceLensFlareParamDef.gHaloRadius.Set(mParamBuffer, settings.HaloRadius);
	gScreenSpaceLensFlareParamDef.gHaloThickness.Set(mParamBuffer, settings.HaloThickness);
	gScreenSpaceLensFlareParamDef.gHaloThreshold.Set(mParamBuffer, settings.HaloThreshold);
	gScreenSpaceLensFlareParamDef.gHaloAspectRatio.Set(mParamBuffer, settings.HaloAspectRatio);
	gScreenSpaceLensFlareParamDef.gChromaticAberration.Set(mParamBuffer, settings.ChromaticAberrationOffset);

	mInputTex.Set(input);
	mGradientTex.Set(RendererTextures::lensFlareGradient);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

ScreenSpaceLensFlareMat* ScreenSpaceLensFlareMat::GetVariation(bool halo, bool haloAspect, bool chromaticAberration)
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

ChromaticAberrationParamDef gChromaticAberrationParamDef;

constexpr int ChromaticAberrationMat::kMaxSamples;

void ChromaticAberrationMat::Initialize()
{
	mParamBuffer = gChromaticAberrationParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);
	mGPUParameters->GetSampledTextureParameter("gFringeTex", mFringeTex);
}

void ChromaticAberrationMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const ChromaticAberrationSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& texProps = input->GetProperties();

	// Set parameters
	gChromaticAberrationParamDef.gInputSize.Set(mParamBuffer, Vector2((float)texProps.Width, (float)texProps.Height));

	gChromaticAberrationParamDef.gShiftAmount.Set(mParamBuffer, settings.ShiftAmount);

	SPtr<Texture> fringeTex;
	if(settings.FringeTexture)
		fringeTex = settings.FringeTexture;
	else
		fringeTex = RendererTextures::chromaticAberrationFringe;

	mInputTex.Set(input);
	mFringeTex.Set(fringeTex);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

ChromaticAberrationMat* ChromaticAberrationMat::GetVariation(ChromaticAberrationType type)
{
	if(type == ChromaticAberrationType::Complex)
		return Get(GetVariation<false>());

	return Get(GetVariation<true>());
}

void ChromaticAberrationMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_SAMPLES", kMaxSamples);
}

FilmGrainParamDef gFilmGrainParamDef;

void FilmGrainMat::Initialize()
{
	mParamBuffer = gFilmGrainParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTex);
}

void FilmGrainMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, float time, const FilmGrainSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	// Set parameters
	gFilmGrainParamDef.gIntensity.Set(mParamBuffer, settings.Intensity);
	gFilmGrainParamDef.gTime.Set(mParamBuffer, settings.Speed * time);

	mInputTex.Set(input);

	// Render
	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

GaussianBlurParamDef gGaussianBlurParamDef;

void GaussianBlurMat::Initialize()
{
	mParamBuffer = gGaussianBlurParamDef.CreateBuffer();
	mIsAdditive = mVariationParameters.GetBool("ADDITIVE");

	mGPUParameters->SetUniformBuffer("GaussianBlurParams", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);

	if(mIsAdditive)
		mGPUParameters->GetSampledTextureParameter("gAdditiveTex", mAdditiveTexture);
}

void GaussianBlurMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_NUM_SAMPLES", kMaxBlurSamples);
}

void GaussianBlurMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination, const Color& tint, const SPtr<Texture>& additive)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = source->GetProperties();
	const RenderTargetProperties& dstProps = destination->GetProperties();

	PooledRenderTextureCreateInformation tempTextureDesc = PooledRenderTextureCreateInformation::Create2D(srcProps.Format, dstProps.Width, dstProps.Height, TU_RENDERTARGET);
	SPtr<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTextureDesc);

	// Horizontal pass
	{
		PopulateBuffer(mParamBuffer, DirHorizontal, source, filterSize, Color::kWhite);
		mInputTexture.Set(source);

		if(mIsAdditive)
			mAdditiveTexture.Set(Texture::kBlack);

		commandBuffer.SetRenderTarget(tempTexture->RenderTexture);

		Bind(commandBuffer);
		GetRendererUtility().DrawScreenQuad(commandBuffer);
	}

	// Vertical pass
	{
		PopulateBuffer(mParamBuffer, DirVertical, source, filterSize, tint);
		mInputTexture.Set(tempTexture->Texture);

		if(mIsAdditive)
		{
			if(additive)
				mAdditiveTexture.Set(additive);
			else
				mAdditiveTexture.Set(Texture::kBlack);
		}

		commandBuffer.SetRenderTarget(destination);

		Bind(commandBuffer);
		GetRendererUtility().DrawScreenQuad(commandBuffer);
	}
}

u32 GaussianBlurMat::CalcStdDistribution(float filterRadius, std::array<float, kMaxBlurSamples>& weights, std::array<float, kMaxBlurSamples>& offsets)
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

float GaussianBlurMat::CalcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir)
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

void GaussianBlurMat::PopulateBuffer(const SPtr<GpuBuffer>& buffer, Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint)
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

		gGaussianBlurParamDef.gSampleWeights.Set(buffer, weight, i);
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

		gGaussianBlurParamDef.gSampleOffsets.Set(buffer, offset, i);
	}

	gGaussianBlurParamDef.gNumSamples.Set(buffer, numSamples);
}

GaussianBlurMat* GaussianBlurMat::GetVariation(bool additive)
{
	if(additive)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

GaussianDOFParamDef gGaussianDOFParamDef;

void GaussianDOFSeparateMat::Initialize()
{
	mParamBuffer = gGaussianDOFParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gColorTex", mColorTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexture);

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

void GaussianDOFSeparateMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
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

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
	gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.FarTransitionRange);

	mColorTexture.Set(color);
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(rt);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

SPtr<PooledRenderTexture> GaussianDOFSeparateMat::GetOutput(u32 idx)
{
	if(idx == 0)
		return mOutput0;
	else if(idx == 1)
		return mOutput1;

	return nullptr;
}

void GaussianDOFSeparateMat::Release()
{
	mOutput0 = nullptr;
	mOutput1 = nullptr;
}

GaussianDOFSeparateMat* GaussianDOFSeparateMat::GetVariation(bool near, bool far)
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

void GaussianDOFCombineMat::Initialize()
{
	mParamBuffer = gGaussianDOFParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);

	mGPUParameters->GetSampledTextureParameter("gFocusedTex", mFocusedTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthTex", mDepthTexture);

	if(mGPUParameters->HasSampledTexture("gNearTex"))
		mGPUParameters->GetSampledTextureParameter("gNearTex", mNearTexture);

	if(mGPUParameters->HasSampledTexture("gFarTex"))
		mGPUParameters->GetSampledTextureParameter("gFarTex", mFarTexture);
}

void GaussianDOFCombineMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, const DepthOfFieldSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = focused->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);

	gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
	gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.FarTransitionRange);

	mFocusedTexture.Set(focused);
	mNearTexture.Set(near);
	mFarTexture.Set(far);
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

GaussianDOFCombineMat* GaussianDOFCombineMat::GetVariation(bool near, bool far)
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

DepthOfFieldCommonParamDef gDepthOfFieldCommonParamDef;
BokehDOFPrepareParamDef gBokehDOFPrepareParamDef;

void BokehDOFPrepareMat::Initialize()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->SetUniformBuffer("DepthOfFieldParams", mCommonParamBuffer);

	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mDepthTexture);
}

void BokehDOFPrepareMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = input->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gBokehDOFPrepareParamDef.gInvInputSize.Set(mParamBuffer, invTexSize);

	BokehDOFMat::PopulateDofCommonParams(mCommonParamBuffer, settings, view);

	mInputTexture.Set(input);
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);

	bool MSAA = mVariationParameters.GetI32("MSAA_COUNT") > 1;
	if(MSAA)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)srcProps.Width, (float)srcProps.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

PooledRenderTextureCreateInformation BokehDOFPrepareMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.Width, 2U));
	u32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.Height, 2U));

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
}

BokehDOFPrepareMat* BokehDOFPrepareMat::GetVariation(bool msaa)
{
	if(msaa)
		return Get(GetVariation<true>());
	else
		return Get(GetVariation<false>());
}

BokehDOFParamDef gBokehDOFParamDef;

constexpr u32 BokehDOFMat::kNearFarPadding;
constexpr u32 BokehDOFMat::kQuadsPerTile;

void BokehDOFMat::Initialize()
{
	mParamBuffer = gBokehDOFParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->SetUniformBuffer("DepthOfFieldParams", mCommonParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureVS);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTextureFS);
	mGPUParameters->GetSampledTextureParameter("gBokehTex", mBokehTexture);

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

	mTileVertexBuffer->WriteData(0, mTileVertexBuffer->GetTotalSize(), vertexData);
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

	mTileIndexBuffer->WriteData(0, mTileVertexBuffer->GetTotalSize(), indices);
	B3DStackFree(indices);
}

void BokehDOFMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("QUADS_PER_TILE", kQuadsPerTile);
}

void BokehDOFMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = input->GetProperties();
	const RenderTargetProperties& dstProps = output->GetProperties();

	Vector2 inputInvTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	Vector2 outputInvTexSize(1.0f / dstProps.Width, 1.0f / dstProps.Height);
	gBokehDOFParamDef.gInvInputSize.Set(mParamBuffer, inputInvTexSize);
	gBokehDOFParamDef.gInvOutputSize.Set(mParamBuffer, outputInvTexSize);
	gBokehDOFParamDef.gAdaptiveThresholdCOC.Set(mParamBuffer, settings.AdaptiveRadiusThreshold);
	gBokehDOFParamDef.gAdaptiveThresholdColor.Set(mParamBuffer, settings.AdaptiveColorThreshold);
	gBokehDOFParamDef.gLayerPixelOffset.Set(mParamBuffer, (i32)srcProps.Height + (i32)kNearFarPadding);
	gBokehDOFParamDef.gInvDepthRange.Set(mParamBuffer, 1.0f / settings.OcclusionDepthRange);

	float bokehSize = settings.MaxBokehSize * srcProps.Width;
	gBokehDOFParamDef.gBokehSize.Set(mParamBuffer, Vector2(bokehSize, bokehSize));

	Vector2I imageSize(srcProps.Width, srcProps.Height);

	// TODO - Allow tile count to halve (i.e. half sampling rate)
	Vector2I tileCount = imageSize / 1;
	gBokehDOFParamDef.gTileCount.Set(mParamBuffer, tileCount);

	PopulateDofCommonParams(mCommonParamBuffer, settings, view);
	mInputTextureVS.Set(input);
	mInputTextureFS.Set(input);

	SPtr<Texture> bokehTexture = settings.BokehShape;
	if(bokehTexture == nullptr)
		bokehTexture = RendererTextures::bokehFlare;

	mBokehTexture.Set(bokehTexture);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
	commandBuffer.ClearRenderTarget(FBT_COLOR, Color::kZero);
	commandBuffer.SetVertexDescription(mTileVertexDescription);

	SPtr<GpuBuffer> buffers[] = { mTileVertexBuffer };
	commandBuffer.SetVertexBuffers(0, buffers, (u32)B3DSize(buffers));
	commandBuffer.SetIndexBuffer(mTileIndexBuffer);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	Bind(commandBuffer);
	const u32 numInstances = Math::DivideAndRoundUp((u32)(tileCount.X * tileCount.Y), kQuadsPerTile);
	commandBuffer.DrawIndexed(0, kQuadsPerTile * 6, 0, kQuadsPerTile * 4, numInstances);
}

PooledRenderTextureCreateInformation BokehDOFMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = rtProps.Width;
	u32 height = rtProps.Height * 2 + kNearFarPadding;

	return PooledRenderTextureCreateInformation::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
}

void BokehDOFMat::PopulateDofCommonParams(const SPtr<GpuBuffer>& buffer, const DepthOfFieldSettings& settings, const RendererView& view)
{
	gDepthOfFieldCommonParamDef.gFocalPlaneDistance.Set(buffer, settings.FocalDistance);
	gDepthOfFieldCommonParamDef.gApertureSize.Set(buffer, settings.ApertureSize * 0.001f); // mm to m
	gDepthOfFieldCommonParamDef.gFocalLength.Set(buffer, settings.FocalLength * 0.001f); // mm to m
	gDepthOfFieldCommonParamDef.gInFocusRange.Set(buffer, settings.FocalRange);
	gDepthOfFieldCommonParamDef.gNearTransitionRegion.Set(buffer, settings.NearTransitionRange);
	gDepthOfFieldCommonParamDef.gFarTransitionRegion.Set(buffer, settings.FarTransitionRange);

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

	gDepthOfFieldCommonParamDef.gSensorSize.Set(buffer, sensorSize);
	gDepthOfFieldCommonParamDef.gImageSize.Set(buffer, imageSize);
	gDepthOfFieldCommonParamDef.gMaxBokehSize.Set(buffer, Math::Clamp01(settings.MaxBokehSize) * imageSize);
}

BokehDOFMat* BokehDOFMat::GetVariation(bool depthOcclusion)
{
	if(depthOcclusion)
		return Get(GetVariation<true>());
	else
		return Get(GetVariation<false>());
}

BokehDOFCombineParamDef gBokehDOFCombineParamDef;

void BokehDOFCombineMat::Initialize()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->SetUniformBuffer("DepthOfFieldParams", mCommonParamBuffer);

	mGPUParameters->GetSampledTextureParameter("gUnfocusedTex", mUnfocusedTexture);
	mGPUParameters->GetSampledTextureParameter("gFocusedTex", mFocusedTexture);
	mGPUParameters->GetSampledTextureParameter("gDepthBufferTex", mDepthTexture);
}

void BokehDOFCombineMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& focusedProps = focused->GetProperties();
	const TextureProperties& unfocusedProps = unfocused->GetProperties();
	u32 halfHeight = std::max(1U, Math::DivideAndRoundUp(focusedProps.Height, 2U));

	float uvScale = halfHeight / (float)unfocusedProps.Height;
	float uvOffset = (halfHeight + BokehDOFMat::kNearFarPadding) / (float)unfocusedProps.Height;

	Vector2 layerScaleOffset(uvScale, uvOffset);
	Vector2 focusedImageSize((float)focusedProps.Width, (float)focusedProps.Height);
	gBokehDOFCombineParamDef.gLayerAndScaleOffset.Set(mParamBuffer, layerScaleOffset);
	gBokehDOFCombineParamDef.gFocusedImageSize.Set(mParamBuffer, focusedImageSize);

	BokehDOFMat::PopulateDofCommonParams(mCommonParamBuffer, settings, view);

	mUnfocusedTexture.Set(unfocused);
	mFocusedTexture.Set(focused);
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

BokehDOFCombineMat* BokehDOFCombineMat::GetVariation(MSAAMode msaaMode)
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

void MotionBlurMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	commandBuffer.SetRenderTarget(output);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void BuildHiZMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 srcMip, const Area2& srcRect, const Area2& dstRect, const SPtr<RenderTexture>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	commandBuffer.SetRenderTarget(output);
	commandBuffer.SetViewport(dstRect);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, srcRect);

	commandBuffer.SetViewport(Area2(0, 0, 1, 1));
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

void FXAAMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, const SPtr<RenderTarget>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.Width, 1.0f / srcProps.Height);
	gFXAAParamDef.gInvTexSize.Set(mParamBuffer, invTexSize);

	mInputTexture.Set(source);

	commandBuffer.SetRenderTarget(destination);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void SSAOMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	commandBuffer.SetRenderTarget(destination);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void SSAODownsampleMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<RenderTexture>& destination, float depthRange)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	commandBuffer.SetRenderTarget(destination);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
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

void SSAOBlurMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth, const SPtr<RenderTexture>& destination, float depthRange)
{
	B3D_PROFILE_RENDERER_MATERIAL

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
	mDepthTexture.Set(depth);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(destination);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

SSAOBlurMat* SSAOBlurMat::GetVariation(bool horizontal)
{
	if(horizontal)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

SSRStencilParamDef gSSRStencilParamDef;

void SSRStencilMat::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mParamBuffer = gSSRStencilParamDef.CreateBuffer();
	mGPUParameters->SetUniformBuffer("Input", mParamBuffer);
}

void SSRStencilMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGBufferParams.Bind(gbuffer);

	Vector2 roughnessScaleBias = SSRTraceMat::CalcRoughnessFadeScaleBias(settings.MaxRoughness);
	gSSRStencilParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	const RendererViewProperties& viewProps = view.GetProperties();
	const Area2I& viewRect = viewProps.Target.ViewRect;
	Bind(commandBuffer);

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

SSRStencilMat* SSRStencilMat::GetVariation(bool msaa, bool singleSampleMSAA)
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

SSRTraceParamDef gSSRTraceParamDef;

void SSRTraceMat::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mParamBuffer = gSSRTraceParamDef.CreateBuffer();

	mGPUParameters->GetSampledTextureParameter("gSceneColor", mSceneColorTexture);
	mGPUParameters->GetSampledTextureParameter("gHiZ", mHiZTexture);

	if(mGPUParameters->HasUniformBuffer("Input"))
		mGPUParameters->SetUniformBuffer("Input", mParamBuffer);

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

void SSRTraceMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings, const SPtr<RenderTarget>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const RendererViewProperties& viewProps = view.GetProperties();

	const TextureProperties& hiZProps = hiZ->GetProperties();

	mGBufferParams.Bind(gbuffer);
	mSceneColorTexture.Set(sceneColor);
	mHiZTexture.Set(hiZ);

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

	Vector2I bufferSize(viewRect.Width, viewRect.Height);
	gSSRTraceParamDef.gHiZSize.Set(mParamBuffer, bufferSize);
	gSSRTraceParamDef.gHiZNumMips.Set(mParamBuffer, hiZProps.MipMapCount);
	gSSRTraceParamDef.gNDCToHiZUV.Set(mParamBuffer, ndcToHiZUV);
	gSSRTraceParamDef.gHiZUVToScreenUV.Set(mParamBuffer, HiZUVToScreenUV);
	gSSRTraceParamDef.gIntensity.Set(mParamBuffer, settings.Intensity);
	gSSRTraceParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);
	gSSRTraceParamDef.gTemporalJitter.Set(mParamBuffer, temporalJitter);

	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

	commandBuffer.SetRenderTarget(destination, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

	Bind(commandBuffer);

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
}

Vector2 SSRTraceMat::CalcRoughnessFadeScaleBias(float maxRoughness)
{
	const static float kRangeScale = 2.0f;

	Vector2 scaleBias;
	scaleBias.X = -kRangeScale / (-1.0f + maxRoughness);
	scaleBias.Y = (kRangeScale * maxRoughness) / (-1.0f + maxRoughness);

	return scaleBias;
}

SSRTraceMat* SSRTraceMat::GetVariation(u32 quality, bool msaa, bool singleSampleMSAA)
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

void TemporalFilteringMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter, float exposure, const SPtr<RenderTarget>& destination)
{
	B3D_PROFILE_RENDERER_MATERIAL

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

	commandBuffer.SetRenderTarget(destination);

	const RendererViewProperties& viewProps = view.GetProperties();
	const Area2I& viewRect = viewProps.Target.ViewRect;

	Bind(commandBuffer);

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer);
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

EncodeDepthParamDef gEncodeDepthParamDef;

void EncodeDepthMat::Initialize()
{
	mParamBuffer = gEncodeDepthParamDef.CreateBuffer();

	mGPUParameters->SetUniformBuffer("Params", mParamBuffer);
	mGPUParameters->GetSampledTextureParameter("gInputTex", mInputTexture);

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

void EncodeDepthMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& depth, float near, float far, const SPtr<RenderTarget>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mInputTexture.Set(depth);

	gEncodeDepthParamDef.gNear.Set(mParamBuffer, near);
	gEncodeDepthParamDef.gFar.Set(mParamBuffer, far);

	commandBuffer.SetRenderTarget(output, 0, RT_COLOR0);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

void MSAACoverageMat::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGPUParameters);
}

void MSAACoverageMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, GBufferTextures gbuffer)
{
	B3D_PROFILE_RENDERER_MATERIAL

	mGBufferParams.Bind(gbuffer);

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;
	SPtr<GpuBuffer> perView = view.GetPerViewBuffer();
	mGPUParameters->SetUniformBuffer("PerCamera", perView);

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

void MSAACoverageStencilMat::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<Texture>& coverage)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const Area2I& viewRect = view.GetProperties().Target.ViewRect;
	mCoverageTexParam.Set(coverage);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}
}}
