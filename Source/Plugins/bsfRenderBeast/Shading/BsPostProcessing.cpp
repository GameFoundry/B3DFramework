//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPostProcessing.h"
#include "RenderAPI/BsRenderTexture.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsCamera.h"
#include "Material/BsGpuParamsSet.h"
#include "Image/BsPixelUtil.h"
#include "Utility/BsBitwise.h"
#include "Renderer/BsGpuResourcePool.h"
#include "BsRendererView.h"
#include "BsRenderBeast.h"
#include "Utility/BsRendererTextures.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs { namespace ct {

void setSamplerState(const SPtr<GpuParams>& params, GpuProgramType gpType, const String& name, const String& secondaryName, const SPtr<SamplerState>& samplerState, bool optional = false)
{
	if(params->HasSamplerState(gpType, name))
		params->SetSamplerState(gpType, name, samplerState);
	else
	{
		if(optional)
		{
			if(params->HasSamplerState(gpType, secondaryName))
				params->SetSamplerState(gpType, secondaryName, samplerState);
		}
		else
			params->SetSamplerState(gpType, secondaryName, samplerState);
	}
}

DownsampleParamDef gDownsampleParamDef;

DownsampleMat::DownsampleMat()
{
	mParamBuffer = gDownsampleParamDef.CreateBuffer();

	if(mParams->HasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
		mParams->SetParamBlockBuffer("Input", mParamBuffer);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void DownsampleMat::Execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTexture.Set(input);

	const TextureProperties& rtProps = input->GetProperties();

	bool MSAA = mVariation.GetInt("MSAA") > 0;
	if(MSAA)
	{
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(-1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(-1.0f, 1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, Vector2(1.0f, 1.0f));
	}
	else
	{
		Vector2 invTextureSize(1.0f / rtProps.GetWidth(), 1.0f / rtProps.GetHeight());

		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(-1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(1.0f, -1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(-1.0f, 1.0f));
		gDownsampleParamDef.gOffsets.Set(mParamBuffer, invTextureSize * Vector2(1.0f, 1.0f));
	}

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind();

	if(MSAA)
		GetRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)rtProps.GetWidth(), (float)rtProps.GetHeight()));
	else
		GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC DownsampleMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1, Math::CeilToInt(rtProps.GetWidth() * 0.5f));
	u32 height = std::max(1, Math::CeilToInt(rtProps.GetHeight() * 0.5f));

	return POOLED_RENDER_TEXTURE_DESC::Create2D(rtProps.GetFormat(), width, height, TU_RENDERTARGET);
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

EyeAdaptHistogramMat::EyeAdaptHistogramMat()
{
	mParamBuffer = gEyeAdaptHistogramParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_COMPUTE_PROGRAM, "gSceneColorTex", mSceneColor);
	mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
}

void EyeAdaptHistogramMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", kThreadGroupSizeY);
	defines.Set("LOOP_COUNT_X", kLoopCountX);
	defines.Set("LOOP_COUNT_Y", kLoopCountY);
}

void EyeAdaptHistogramMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mSceneColor.Set(input);

	const TextureProperties& props = input->GetProperties();
	Vector4I offsetAndSize(0, 0, (i32)props.GetWidth(), (i32)props.GetHeight());

	gEyeAdaptHistogramParamDef.gHistogramParams.Set(mParamBuffer, GetHistogramScaleOffset(settings));
	gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.Set(mParamBuffer, offsetAndSize);

	Vector2I threadGroupCount = GetThreadGroupCount(input);
	gEyeAdaptHistogramParamDef.gThreadGroupCount.Set(mParamBuffer, threadGroupCount);

	// Dispatch
	mOutputTex.Set(output);

	Bind();

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.DispatchCompute(threadGroupCount.X, threadGroupCount.Y);
}

POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramMat::GetOutputDesc(const SPtr<Texture>& target)
{
	Vector2I threadGroupCount = GetThreadGroupCount(target);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, kHistogramNumTexels, numHistograms, TU_LOADSTORE);
}

Vector2I EyeAdaptHistogramMat::GetThreadGroupCount(const SPtr<Texture>& target)
{
	const u32 texelsPerThreadGroupX = kThreadGroupSizeX * kLoopCountX;
	const u32 texelsPerThreadGroupY = kThreadGroupSizeY * kLoopCountY;

	const TextureProperties& props = target->GetProperties();

	Vector2I threadGroupCount;
	threadGroupCount.X = ((i32)props.GetWidth() + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
	threadGroupCount.Y = ((i32)props.GetHeight() + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

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

EyeAdaptHistogramReduceMat::EyeAdaptHistogramReduceMat()
{
	mParamBuffer = gEyeAdaptHistogramReduceParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mHistogramTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
}

void EyeAdaptHistogramReduceMat::Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mHistogramTex.Set(histogram);

	SPtr<Texture> eyeAdaptationTex;
	if(prevFrame == nullptr) // Could be that this is the first run
		eyeAdaptationTex = Texture::WHITE;
	else
		eyeAdaptationTex = prevFrame;

	mEyeAdaptationTex.Set(eyeAdaptationTex);

	Vector2I threadGroupCount = EyeAdaptHistogramMat::GetThreadGroupCount(sceneColor);
	u32 numHistograms = threadGroupCount.X * threadGroupCount.Y;

	gEyeAdaptHistogramReduceParamDef.gThreadGroupCount.Set(mParamBuffer, numHistograms);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind();

	Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::kHistogramNumTexels, 2.0f);
	GetRendererUtility().DrawScreenQuad(drawUV);

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramReduceMat::GetOutputDesc()
{
	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, EyeAdaptHistogramMat::kHistogramNumTexels, 2, TU_RENDERTARGET);
}

EyeAdaptationParamDef gEyeAdaptationParamDef;

EyeAdaptationMat::EyeAdaptationMat()
{
	mParamBuffer = gEyeAdaptationParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("EyeAdaptationParams", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mReducedHistogramTex);
}

void EyeAdaptationMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::kThreadGroupSizeX);
	defines.Set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::kThreadGroupSizeY);
}

void EyeAdaptationMat::Execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mReducedHistogramTex.Set(reducedHistogram);

	PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC EyeAdaptationMat::GetOutputDesc()
{
	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

void EyeAdaptationMat::PopulateParams(const SPtr<GpuParamBlockBuffer>& paramBuffer, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
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

	eyeAdaptationParams[2].X = Math::Pow(2.0f, exposureScale);
	eyeAdaptationParams[2].Y = frameDelta;

	eyeAdaptationParams[2].Z = Math::Pow(2.0f, settings.HistogramLog2Min);
	eyeAdaptationParams[2].W = 0.0f; // Unused

	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[0], 0);
	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[1], 1);
	gEyeAdaptationParamDef.gEyeAdaptationParams.Set(paramBuffer, eyeAdaptationParams[2], 2);
}

EyeAdaptationBasicSetupMat::EyeAdaptationBasicSetupMat()
{
	mParamBuffer = gEyeAdaptationParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("EyeAdaptationParams", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);

	SAMPLER_STATE_DESC desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;

	SPtr<SamplerState> samplerState = SamplerState::Create(desc);
	setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gInputSamp", "gInputTex", samplerState);
}

void EyeAdaptationBasicSetupMat::Execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mInputTex.Set(input);

	EyeAdaptationMat::PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC EyeAdaptationBasicSetupMat::GetOutputDesc(const SPtr<Texture>& input)
{
	auto& props = input->GetProperties();
	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, props.GetWidth(), props.GetHeight(), TU_RENDERTARGET);
}

EyeAdaptationBasicParamsMatDef gEyeAdaptationBasicParamsMatDef;

EyeAdaptationBasicMat::EyeAdaptationBasicMat()
{
	mEyeAdaptationParamsBuffer = gEyeAdaptationParamDef.CreateBuffer();
	mParamsBuffer = gEyeAdaptationBasicParamsMatDef.CreateBuffer();

	mParams->SetParamBlockBuffer("EyeAdaptationParams", mEyeAdaptationParamsBuffer);
	mParams->SetParamBlockBuffer("Input", mParamsBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gCurFrameTex", mCurFrameTexParam);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gPrevFrameTex", mPrevFrameTexParam);
}

void EyeAdaptationBasicMat::Execute(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mCurFrameTexParam.Set(curFrame);

	if(prevFrame == nullptr) // Could be that this is the first run
		mPrevFrameTexParam.Set(Texture::WHITE);
	else
		mPrevFrameTexParam.Set(prevFrame);

	EyeAdaptationMat::PopulateParams(mEyeAdaptationParamsBuffer, frameDelta, settings, exposureScale);

	auto& texProps = curFrame->GetProperties();
	Vector2I texSize = { (i32)texProps.GetWidth(), (i32)texProps.GetHeight() };

	gEyeAdaptationBasicParamsMatDef.gInputTexSize.Set(mParamsBuffer, texSize);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

POOLED_RENDER_TEXTURE_DESC EyeAdaptationBasicMat::GetOutputDesc()
{
	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
}

CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;
WhiteBalanceParamDef gWhiteBalanceParamDef;

CreateTonemapLUTMat::CreateTonemapLUTMat()
{
	mIs3D = mVariation.GetBool("VOLUME_LUT");

	mParamBuffer = gCreateTonemapLUTParamDef.CreateBuffer();
	mWhiteBalanceParamBuffer = gWhiteBalanceParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->SetParamBlockBuffer("WhiteBalanceInput", mWhiteBalanceParamBuffer);

	if(mIs3D)
		mParams->GetLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
}

void CreateTonemapLUTMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", kLutSize);
}

void CreateTonemapLUTMat::Execute3D(const SPtr<Texture>& output, const RenderSettings& settings)
{
	assert(mIs3D);
	BS_RENMAT_PROFILE_BLOCK

	PopulateParamBuffers(settings);

	// Dispatch
	mOutputTex.Set(output);

	Bind();

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.DispatchCompute(kLutSize / 8, kLutSize / 8, kLutSize);
}

void CreateTonemapLUTMat::Execute2D(const SPtr<RenderTexture>& output, const RenderSettings& settings)
{
	assert(!mIs3D);
	BS_RENMAT_PROFILE_BLOCK

	PopulateParamBuffers(settings);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();

	rapi.SetRenderTarget(nullptr);
}

void CreateTonemapLUTMat::PopulateParamBuffers(const RenderSettings& settings)
{
	// Set parameters
	gCreateTonemapLUTParamDef.gGammaAdjustment.Set(mParamBuffer, 2.2f / settings.Gamma);

	// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
	// gamma when running on Mac (value 2)
	gCreateTonemapLUTParamDef.gGammaCorrectionType.Set(mParamBuffer, 0);

	Vector4 tonemapParams[2];
	tonemapParams[0].X = settings.Tonemapping.FilmicCurveShoulderStrength;
	tonemapParams[0].Y = settings.Tonemapping.FilmicCurveLinearStrength;
	tonemapParams[0].Z = settings.Tonemapping.FilmicCurveLinearAngle;
	tonemapParams[0].W = settings.Tonemapping.FilmicCurveToeStrength;

	tonemapParams[1].X = settings.Tonemapping.FilmicCurveToeNumerator;
	tonemapParams[1].Y = settings.Tonemapping.FilmicCurveToeDenominator;
	tonemapParams[1].Z = settings.Tonemapping.FilmicCurveLinearWhitePoint;
	tonemapParams[1].W = 0.0f; // Unused

	gCreateTonemapLUTParamDef.gTonemapParams.Set(mParamBuffer, tonemapParams[0], 0);
	gCreateTonemapLUTParamDef.gTonemapParams.Set(mParamBuffer, tonemapParams[1], 1);

	// Set color grading params
	gCreateTonemapLUTParamDef.gSaturation.Set(mParamBuffer, settings.ColorGrading.Saturation);
	gCreateTonemapLUTParamDef.gContrast.Set(mParamBuffer, settings.ColorGrading.Contrast);
	gCreateTonemapLUTParamDef.gGain.Set(mParamBuffer, settings.ColorGrading.Gain);
	gCreateTonemapLUTParamDef.gOffset.Set(mParamBuffer, settings.ColorGrading.Offset);

	// Set white balance params
	gWhiteBalanceParamDef.gWhiteTemp.Set(mWhiteBalanceParamBuffer, settings.WhiteBalance.Temperature);
	gWhiteBalanceParamDef.gWhiteOffset.Set(mWhiteBalanceParamBuffer, settings.WhiteBalance.Tint);
}

POOLED_RENDER_TEXTURE_DESC CreateTonemapLUTMat::GetOutputDesc() const
{
	if(mIs3D)
		return POOLED_RENDER_TEXTURE_DESC::Create3D(PF_RGBA8, kLutSize, kLutSize, kLutSize, TU_LOADSTORE);

	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA8, kLutSize * kLutSize, kLutSize, TU_RENDERTARGET);
}

CreateTonemapLUTMat* CreateTonemapLUTMat::GetVariation(bool is3D)
{
	if(is3D)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

TonemappingParamDef gTonemappingParamDef;

TonemappingMat::TonemappingMat()
{
	mParamBuffer = gTonemappingParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gBloomTex", mBloomTex);

	if(!mVariation.GetBool("GAMMA_ONLY"))
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gColorLUT", mColorLUT);
}

void TonemappingMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("LUT_SIZE", CreateTonemapLUTMat::kLutSize);
}

void TonemappingMat::Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output, const RenderSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& texProps = sceneColor->GetProperties();

	gTonemappingParamDef.gRawGamma.Set(mParamBuffer, 1.0f / settings.Gamma);
	gTonemappingParamDef.gManualExposureScale.Set(mParamBuffer, Math::Pow(2.0f, settings.ExposureScale));
	gTonemappingParamDef.gTexSize.Set(mParamBuffer, Vector2((float)texProps.GetWidth(), (float)texProps.GetHeight()));
	gTonemappingParamDef.gBloomTint.Set(mParamBuffer, settings.Bloom.Tint);
	gTonemappingParamDef.gNumSamples.Set(mParamBuffer, texProps.GetNumSamples());

	// Set parameters
	mInputTex.Set(sceneColor);
	mColorLUT.Set(colorLUT);
	mEyeAdaptationTex.Set(eyeAdaptation);
	mBloomTex.Set(bloom != nullptr ? bloom : Texture::BLACK);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

BloomClipMat::BloomClipMat()
{
	mParamBuffer = gBloomClipParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
}

void BloomClipMat::Execute(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation, const RenderSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	gBloomClipParamDef.gThreshold.Set(mParamBuffer, threshold);
	gBloomClipParamDef.gManualExposureScale.Set(mParamBuffer, Math::Pow(2.0f, settings.ExposureScale));

	// Set parameters
	mInputTex.Set(input);
	mEyeAdaptationTex.Set(eyeAdaptation);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();

	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

BloomClipMat* BloomClipMat::GetVariation(bool autoExposure)
{
	if(autoExposure)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

ScreenSpaceLensFlareParamDef gScreenSpaceLensFlareParamDef;

ScreenSpaceLensFlareMat::ScreenSpaceLensFlareMat()
{
	mParamBuffer = gScreenSpaceLensFlareParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gGradientTex", mGradientTex);
}

void ScreenSpaceLensFlareMat::Execute(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

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
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

ChromaticAberrationMat::ChromaticAberrationMat()
{
	mParamBuffer = gChromaticAberrationParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gFringeTex", mFringeTex);
}

void ChromaticAberrationMat::Execute(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& texProps = input->GetProperties();

	// Set parameters
	gChromaticAberrationParamDef.gInputSize.Set(mParamBuffer, Vector2((float)texProps.GetWidth(), (float)texProps.GetHeight()));

	gChromaticAberrationParamDef.gShiftAmount.Set(mParamBuffer, settings.ShiftAmount);

	SPtr<Texture> fringeTex;
	if(settings.FringeTexture)
		fringeTex = settings.FringeTexture;
	else
		fringeTex = RendererTextures::chromaticAberrationFringe;

	mInputTex.Set(input);
	mFringeTex.Set(fringeTex);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

FilmGrainMat::FilmGrainMat()
{
	mParamBuffer = gFilmGrainParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
}

void FilmGrainMat::Execute(const SPtr<Texture>& input, float time, const FilmGrainSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	gFilmGrainParamDef.gIntensity.Set(mParamBuffer, settings.Intensity);
	gFilmGrainParamDef.gTime.Set(mParamBuffer, settings.Speed * time);

	mInputTex.Set(input);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

GaussianBlurParamDef gGaussianBlurParamDef;

GaussianBlurMat::GaussianBlurMat()
{
	mParamBuffer = gGaussianBlurParamDef.CreateBuffer();
	mIsAdditive = mVariation.GetBool("ADDITIVE");

	mParams->SetParamBlockBuffer("GaussianBlurParams", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);

	if(mIsAdditive)
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gAdditiveTex", mAdditiveTexture);
}

void GaussianBlurMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("MAX_NUM_SAMPLES", kMaxBlurSamples);
}

void GaussianBlurMat::Execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination, const Color& tint, const SPtr<Texture>& additive)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = source->GetProperties();
	const RenderTextureProperties& dstProps = destination->GetProperties();

	POOLED_RENDER_TEXTURE_DESC tempTextureDesc = POOLED_RENDER_TEXTURE_DESC::Create2D(srcProps.GetFormat(), dstProps.Width, dstProps.Height, TU_RENDERTARGET);
	SPtr<PooledRenderTexture> tempTexture = GetGpuResourcePool().Get(tempTextureDesc);

	// Horizontal pass
	{
		PopulateBuffer(mParamBuffer, DirHorizontal, source, filterSize, Color::kWhite);
		mInputTexture.Set(source);

		if(mIsAdditive)
			mAdditiveTexture.Set(Texture::BLACK);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(tempTexture->RenderTexture);

		Bind();
		GetRendererUtility().DrawScreenQuad();
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
				mAdditiveTexture.Set(Texture::BLACK);
		}

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(destination);

		Bind();
		GetRendererUtility().DrawScreenQuad();
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
		length = source->GetProperties().GetWidth();
	else
		length = source->GetProperties().GetHeight();

	// Divide by two because we need the radius
	return std::min(length * scale / 2, (float)kMaxBlurSamples - 1);
}

void GaussianBlurMat::PopulateBuffer(const SPtr<GpuParamBlockBuffer>& buffer, Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint)
{
	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

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

GaussianDOFSeparateMat::GaussianDOFSeparateMat()
{
	mParamBuffer = gGaussianDOFParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gColorTex", mColorTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

	SAMPLER_STATE_DESC desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> samplerState = SamplerState::Create(desc);
	setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gColorSamp", "gColorTex", samplerState);
}

void GaussianDOFSeparateMat::Execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = color->GetProperties();

	u32 outputWidth = std::max(1U, srcProps.GetWidth() / 2);
	u32 outputHeight = std::max(1U, srcProps.GetHeight() / 2);

	POOLED_RENDER_TEXTURE_DESC outputTexDesc = POOLED_RENDER_TEXTURE_DESC::Create2D(srcProps.GetFormat(), outputWidth, outputHeight, TU_RENDERTARGET);
	mOutput0 = GetGpuResourcePool().Get(outputTexDesc);

	bool near = mVariation.GetBool("NEAR");
	bool far = mVariation.GetBool("FAR");

	SPtr<RenderTexture> rt;
	if(near && far)
	{
		mOutput1 = GetGpuResourcePool().Get(outputTexDesc);

		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.ColorSurfaces[0].Texture = mOutput0->Texture;
		rtDesc.ColorSurfaces[1].Texture = mOutput1->Texture;

		rt = RenderTexture::Create(rtDesc);
	}
	else
		rt = mOutput0->RenderTexture;

	Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

	gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
	gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.FarTransitionRange);

	mColorTexture.Set(color);
	mDepthTexture.Set(depth);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(rt);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

GaussianDOFCombineMat::GaussianDOFCombineMat()
{
	mParamBuffer = gGaussianDOFParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gFocusedTex", mFocusedTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

	if(mParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gNearTex"))
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gNearTex", mNearTexture);

	if(mParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gFarTex"))
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gFarTex", mFarTexture);
}

void GaussianDOFCombineMat::Execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, const DepthOfFieldSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = focused->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

	gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
	gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.FocalDistance - settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.FocalDistance + settings.FocalRange * 0.5f);
	gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.NearTransitionRange);
	gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.FarTransitionRange);

	mFocusedTexture.Set(focused);
	mNearTexture.Set(near);
	mFarTexture.Set(far);
	mDepthTexture.Set(depth);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

BokehDOFPrepareMat::BokehDOFPrepareMat()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->SetParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);
}

void BokehDOFPrepareMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = input->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());
	gBokehDOFPrepareParamDef.gInvInputSize.Set(mParamBuffer, invTexSize);

	BokehDOFMat::PopulateDofCommonParams(mCommonParamBuffer, settings, view);

	mInputTexture.Set(input);
	mDepthTexture.Set(depth);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();

	bool MSAA = mVariation.GetInt("MSAA_COUNT") > 1;
	if(MSAA)
		GetRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)srcProps.GetWidth(), (float)srcProps.GetHeight()));
	else
		GetRendererUtility().DrawScreenQuad();
}

POOLED_RENDER_TEXTURE_DESC BokehDOFPrepareMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.GetWidth(), 2U));
	u32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.GetHeight(), 2U));

	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
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

BokehDOFMat::BokehDOFMat()
{
	mParamBuffer = gBokehDOFParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->SetParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);
	mParams->GetTextureParam(GPT_VERTEX_PROGRAM, "gInputTex", mInputTextureVS);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTextureFS);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gBokehTex", mBokehTexture);

	// Prepare vertex declaration for rendering tiles
	SPtr<VertexDataDesc> tileVertexDesc = B3DMakeShared<VertexDataDesc>();
	tileVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);

	mTileVertexDecl = VertexDeclaration::Create(tileVertexDesc);

	// Prepare vertex buffer for rendering tiles
	VERTEX_BUFFER_DESC tileVertexBufferDesc;
	tileVertexBufferDesc.NumVerts = kQuadsPerTile * 4;
	tileVertexBufferDesc.VertexSize = tileVertexDesc->GetVertexStride();

	mTileVertexBuffer = VertexBuffer::Create(tileVertexBufferDesc);

	auto* const vertexData = (Vector2*)mTileVertexBuffer->Lock(GBL_WRITE_ONLY_DISCARD);
	for(u32 i = 0; i < kQuadsPerTile; i++)
	{
		vertexData[i * 4 + 0] = Vector2(0.0f, 0.0f);
		vertexData[i * 4 + 1] = Vector2(1.0f, 0.0f);
		vertexData[i * 4 + 2] = Vector2(0.0f, 1.0f);
		vertexData[i * 4 + 3] = Vector2(1.0f, 1.0f);
	}

	mTileVertexBuffer->Unlock();

	// Prepare indices for rendering tiles
	INDEX_BUFFER_DESC tileIndexBufferDesc;
	tileIndexBufferDesc.IndexType = IT_16BIT;
	tileIndexBufferDesc.NumIndices = kQuadsPerTile * 6;

	mTileIndexBuffer = IndexBuffer::Create(tileIndexBufferDesc);

	auto* const indices = (u16*)mTileIndexBuffer->Lock(GBL_WRITE_ONLY_DISCARD);

	const Conventions& rapiConventions = GetRenderBackendCapabilities().Conventions;
	for(u32 i = 0; i < kQuadsPerTile; i++)
	{
		// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
		// get culled.
		if(rapiConventions.UvYAxis == Conventions::Axis::Up)
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

	mTileIndexBuffer->Unlock();
}

void BokehDOFMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("QUADS_PER_TILE", kQuadsPerTile);
}

void BokehDOFMat::Execute(const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = input->GetProperties();
	const RenderTargetProperties& dstProps = output->GetProperties();

	Vector2 inputInvTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());
	Vector2 outputInvTexSize(1.0f / dstProps.Width, 1.0f / dstProps.Height);
	gBokehDOFParamDef.gInvInputSize.Set(mParamBuffer, inputInvTexSize);
	gBokehDOFParamDef.gInvOutputSize.Set(mParamBuffer, outputInvTexSize);
	gBokehDOFParamDef.gAdaptiveThresholdCOC.Set(mParamBuffer, settings.AdaptiveRadiusThreshold);
	gBokehDOFParamDef.gAdaptiveThresholdColor.Set(mParamBuffer, settings.AdaptiveColorThreshold);
	gBokehDOFParamDef.gLayerPixelOffset.Set(mParamBuffer, (i32)srcProps.GetHeight() + (i32)kNearFarPadding);
	gBokehDOFParamDef.gInvDepthRange.Set(mParamBuffer, 1.0f / settings.OcclusionDepthRange);

	float bokehSize = settings.MaxBokehSize * srcProps.GetWidth();
	gBokehDOFParamDef.gBokehSize.Set(mParamBuffer, Vector2(bokehSize, bokehSize));

	Vector2I imageSize(srcProps.GetWidth(), srcProps.GetHeight());

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

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
	rapi.ClearRenderTarget(FBT_COLOR, Color::kZero);
	rapi.SetVertexDeclaration(mTileVertexDecl);

	SPtr<VertexBuffer> buffers[] = { mTileVertexBuffer };
	rapi.SetVertexBuffers(0, buffers, (u32)bs_size(buffers));
	rapi.SetIndexBuffer(mTileIndexBuffer);
	rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

	Bind();
	const u32 numInstances = Math::DivideAndRoundUp((u32)(tileCount.X * tileCount.Y), kQuadsPerTile);
	rapi.DrawIndexed(0, kQuadsPerTile * 6, 0, kQuadsPerTile * 4, numInstances);
}

POOLED_RENDER_TEXTURE_DESC BokehDOFMat::GetOutputDesc(const SPtr<Texture>& target)
{
	const TextureProperties& rtProps = target->GetProperties();

	u32 width = rtProps.GetWidth();
	u32 height = rtProps.GetHeight() * 2 + kNearFarPadding;

	return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
}

void BokehDOFMat::PopulateDofCommonParams(const SPtr<GpuParamBlockBuffer>& buffer, const DepthOfFieldSettings& settings, const RendererView& view)
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

BokehDOFCombineMat::BokehDOFCombineMat()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();
	mCommonParamBuffer = gDepthOfFieldCommonParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->SetParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gUnfocusedTex", mUnfocusedTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gFocusedTex", mFocusedTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);
}

void BokehDOFCombineMat::Execute(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& focusedProps = focused->GetProperties();
	const TextureProperties& unfocusedProps = unfocused->GetProperties();
	u32 halfHeight = std::max(1U, Math::DivideAndRoundUp(focusedProps.GetHeight(), 2U));

	float uvScale = halfHeight / (float)unfocusedProps.GetHeight();
	float uvOffset = (halfHeight + BokehDOFMat::kNearFarPadding) / (float)unfocusedProps.GetHeight();

	Vector2 layerScaleOffset(uvScale, uvOffset);
	Vector2 focusedImageSize((float)focusedProps.GetWidth(), (float)focusedProps.GetHeight());
	gBokehDOFCombineParamDef.gLayerAndScaleOffset.Set(mParamBuffer, layerScaleOffset);
	gBokehDOFCombineParamDef.gFocusedImageSize.Set(mParamBuffer, focusedImageSize);

	BokehDOFMat::PopulateDofCommonParams(mCommonParamBuffer, settings, view);

	mUnfocusedTexture.Set(unfocused);
	mFocusedTexture.Set(focused);
	mDepthTexture.Set(depth);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

MotionBlurMat::MotionBlurMat()
{
	mParamBuffer = gBokehDOFPrepareParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);

	SAMPLER_STATE_DESC pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> pointSampState = SamplerState::Create(pointSampDesc);

	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp", pointSampState);
}

void MotionBlurMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

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

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

BuildHiZFParamDef gBuildHiZParamDef;

BuildHiZMat::BuildHiZMat()
{
	mNoTextureViews = mVariation.GetBool("NO_TEXTURE_VIEWS");

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mInputTexture);

	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mParamBuffer = gBuildHiZParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.MinFilter = FO_POINT;
		inputSampDesc.MagFilter = FO_POINT;
		inputSampDesc.MipFilter = FO_POINT;

		SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gDepthSamp", "gDepthTex", inputSampState);
	}
}

void BuildHiZMat::Execute(const SPtr<Texture>& source, u32 srcMip, const Rect2& srcRect, const Rect2& dstRect, const SPtr<RenderTexture>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	RenderAPI& rapi = RenderAPI::Instance();

	// If no texture view support, we must manually pick a valid mip level in the shader
	if(mNoTextureViews)
	{
		mInputTexture.Set(source);

		auto& props = source->GetProperties();
		float pixelWidth = (float)props.GetWidth();
		float pixelHeight = (float)props.GetHeight();

		Vector2 halfPixelOffset(0.5f / pixelWidth, 0.5f / pixelHeight);

		gBuildHiZParamDef.gHalfPixelOffset.Set(mParamBuffer, halfPixelOffset);
		gBuildHiZParamDef.gMipLevel.Set(mParamBuffer, srcMip);
	}
	else
		mInputTexture.Set(source, TextureSurface(srcMip));

	rapi.SetRenderTarget(output);
	rapi.SetViewport(dstRect);

	Bind();
	GetRendererUtility().DrawScreenQuad(srcRect);

	rapi.SetViewport(Rect2(0, 0, 1, 1));
}

BuildHiZMat* BuildHiZMat::GetVariation(bool noTextureViews)
{
	if(noTextureViews)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

FXAAParamDef gFXAAParamDef;

FXAAMat::FXAAMat()
{
	mParamBuffer = gFXAAParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
}

void FXAAMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination)
{
	BS_RENMAT_PROFILE_BLOCK

	const TextureProperties& srcProps = source->GetProperties();

	Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());
	gFXAAParamDef.gInvTexSize.Set(mParamBuffer, invTexSize);

	mInputTexture.Set(source);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

SSAOParamDef gSSAOParamDef;

SSAOMat::SSAOMat()
{
	bool isFinal = mVariation.GetBool("FINAL_AO");
	bool mixWithUpsampled = mVariation.GetBool("MIX_WITH_UPSAMPLED");

	mParamBuffer = gSSAOParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);

	if(isFinal)
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);
	}

	if(!isFinal || mixWithUpsampled)
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSetupAO", mSetupAOTexture);

	if(mixWithUpsampled)
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDownsampledAO", mDownsampledAOTexture);

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gRandomTex", mRandomTexture);

	SAMPLER_STATE_DESC inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
	else
	{
		if(isFinal)
		{
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gNormalsTex", inputSampState);
		}

		if(!isFinal || mixWithUpsampled)
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gSetupAO", inputSampState);

		if(mixWithUpsampled)
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDownsampledAO", inputSampState);
	}

	SAMPLER_STATE_DESC randomSampDesc;
	randomSampDesc.MinFilter = FO_POINT;
	randomSampDesc.MagFilter = FO_POINT;
	randomSampDesc.MipFilter = FO_POINT;
	randomSampDesc.AddressMode.U = TAM_WRAP;
	randomSampDesc.AddressMode.V = TAM_WRAP;
	randomSampDesc.AddressMode.W = TAM_WRAP;

	SPtr<SamplerState> randomSampState = SamplerState::Create(randomSampDesc);
	setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gRandomSamp", "gRandomTex", randomSampState);
}

void SSAOMat::Execute(const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

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

	bool upsample = mVariation.GetBool("MIX_WITH_UPSAMPLED");
	if(upsample)
	{
		const TextureProperties& props = textures.AoDownsampled->GetProperties();

		Vector2 downsampledPixelSize;
		downsampledPixelSize.X = 1.0f / props.GetWidth();
		downsampledPixelSize.Y = 1.0f / props.GetHeight();

		gSSAOParamDef.gDownsampledPixelSize.Set(mParamBuffer, downsampledPixelSize);
	}

	// Generate a scale which we need to use in order to achieve tiling
	const TextureProperties& rndProps = textures.RandomRotations->GetProperties();
	u32 rndWidth = rndProps.GetWidth();
	u32 rndHeight = rndProps.GetHeight();

	//// Multiple of random texture size, rounded up
	u32 scaleWidth = (rtProps.Width + rndWidth - 1) / rndWidth;
	u32 scaleHeight = (rtProps.Height + rndHeight - 1) / rndHeight;

	Vector2 randomTileScale((float)scaleWidth, (float)scaleHeight);
	gSSAOParamDef.gRandomTileScale.Set(mParamBuffer, randomTileScale);

	mSetupAOTexture.Set(textures.AoSetup);

	bool finalPass = mVariation.GetBool("FINAL_AO");
	if(finalPass)
	{
		mDepthTexture.Set(textures.SceneDepth);
		mNormalsTexture.Set(textures.SceneNormals);
	}

	if(upsample)
		mDownsampledAOTexture.Set(textures.AoDownsampled);

	mRandomTexture.Set(textures.RandomRotations);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination);

	Bind();
	GetRendererUtility().DrawScreenQuad();
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

SSAODownsampleMat::SSAODownsampleMat()
{
	mParamBuffer = gSSAODownsampleParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);

	SAMPLER_STATE_DESC inputSampDesc;
	inputSampDesc.MinFilter = FO_LINEAR;
	inputSampDesc.MagFilter = FO_LINEAR;
	inputSampDesc.MipFilter = FO_LINEAR;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);

	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
	else
	{
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gNormalsTex", inputSampState);
	}
}

void SSAODownsampleMat::Execute(const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals, const SPtr<RenderTexture>& destination, float depthRange)
{
	BS_RENMAT_PROFILE_BLOCK

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

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

SSAOBlurParamDef gSSAOBlurParamDef;

SSAOBlurMat::SSAOBlurMat()
{
	mParamBuffer = gSSAOBlurParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Input", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mAOTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

	SAMPLER_STATE_DESC inputSampDesc;
	inputSampDesc.MinFilter = FO_POINT;
	inputSampDesc.MagFilter = FO_POINT;
	inputSampDesc.MipFilter = FO_POINT;
	inputSampDesc.AddressMode.U = TAM_CLAMP;
	inputSampDesc.AddressMode.V = TAM_CLAMP;
	inputSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
	else
	{
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputTex", inputSampState);
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
	}
}

void SSAOBlurMat::Execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth, const SPtr<RenderTexture>& destination, float depthRange)
{
	BS_RENMAT_PROFILE_BLOCK

	const RendererViewProperties& viewProps = view.GetProperties();
	const TextureProperties& texProps = ao->GetProperties();

	Vector2 pixelSize;
	pixelSize.X = 1.0f / texProps.GetWidth();
	pixelSize.Y = 1.0f / texProps.GetHeight();

	Vector2 pixelOffset(BsZero);
	if(mVariation.GetBool("DIR_HORZ"))
		pixelOffset.X = pixelSize.X;
	else
		pixelOffset.Y = pixelSize.Y;

	float scale = viewProps.Target.ViewRect.Width / (float)texProps.GetWidth();

	gSSAOBlurParamDef.gPixelSize.Set(mParamBuffer, pixelSize);
	gSSAOBlurParamDef.gPixelOffset.Set(mParamBuffer, pixelOffset);
	gSSAOBlurParamDef.gInvDepthThreshold.Set(mParamBuffer, (1.0f / depthRange) / scale);

	mAOTexture.Set(ao);
	mDepthTexture.Set(depth);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

SSAOBlurMat* SSAOBlurMat::GetVariation(bool horizontal)
{
	if(horizontal)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

SSRStencilParamDef gSSRStencilParamDef;

SSRStencilMat::SSRStencilMat()
	: mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
{
	mParamBuffer = gSSRStencilParamDef.CreateBuffer();
	mParams->SetParamBlockBuffer("Input", mParamBuffer);
}

void SSRStencilMat::Execute(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings)
{
	BS_RENMAT_PROFILE_BLOCK

	mGBufferParams.Bind(gbuffer);

	Vector2 roughnessScaleBias = SSRTraceMat::CalcRoughnessFadeScaleBias(settings.MaxRoughness);
	gSSRStencilParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	const RendererViewProperties& viewProps = view.GetProperties();
	const Rect2I& viewRect = viewProps.Target.ViewRect;
	Bind();

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad();
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

SSRTraceMat::SSRTraceMat()
	: mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
{
	mParamBuffer = gSSRTraceParamDef.CreateBuffer();

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gHiZ", mHiZTexture);

	if(mParams->HasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
		mParams->SetParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);

	SAMPLER_STATE_DESC desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> hiZSamplerState = SamplerState::Create(desc);
	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp", hiZSamplerState);
	else if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ", hiZSamplerState);
}

void SSRTraceMat::Execute(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings, const SPtr<RenderTarget>& destination)
{
	BS_RENMAT_PROFILE_BLOCK

	const RendererViewProperties& viewProps = view.GetProperties();

	const TextureProperties& hiZProps = hiZ->GetProperties();

	mGBufferParams.Bind(gbuffer);
	mSceneColorTexture.Set(sceneColor);
	mHiZTexture.Set(hiZ);

	Rect2I viewRect = viewProps.Target.ViewRect;

	// Maps from NDC to UV [0, 1]
	Vector4 ndcToHiZUV;
	ndcToHiZUV.X = 0.5f;
	ndcToHiZUV.Y = -0.5f;
	ndcToHiZUV.Z = 0.5f;
	ndcToHiZUV.W = 0.5f;

	// Either of these flips the Y axis, but if they're both true they cancel out
	const Conventions& rapiConventions = GetRenderBackendCapabilities().Conventions;

	if((rapiConventions.UvYAxis == Conventions::Axis::Up) ^ (rapiConventions.NdcYAxis == Conventions::Axis::Down))
		ndcToHiZUV.Y = -ndcToHiZUV.Y;

	// Maps from [0, 1] to area of HiZ where depth is stored in
	ndcToHiZUV.X *= (float)viewRect.Width / hiZProps.GetWidth();
	ndcToHiZUV.Y *= (float)viewRect.Height / hiZProps.GetHeight();
	ndcToHiZUV.Z *= (float)viewRect.Width / hiZProps.GetWidth();
	ndcToHiZUV.W *= (float)viewRect.Height / hiZProps.GetHeight();

	// Maps from HiZ UV to [0, 1] UV
	Vector2 HiZUVToScreenUV;
	HiZUVToScreenUV.X = hiZProps.GetWidth() / (float)viewRect.Width;
	HiZUVToScreenUV.Y = hiZProps.GetHeight() / (float)viewRect.Height;

	// Used for roughness fading
	Vector2 roughnessScaleBias = CalcRoughnessFadeScaleBias(settings.MaxRoughness);

	u32 temporalJitter = (viewProps.FrameIdx % 8) * 1503;

	Vector2I bufferSize(viewRect.Width, viewRect.Height);
	gSSRTraceParamDef.gHiZSize.Set(mParamBuffer, bufferSize);
	gSSRTraceParamDef.gHiZNumMips.Set(mParamBuffer, hiZProps.GetNumMipmaps());
	gSSRTraceParamDef.gNDCToHiZUV.Set(mParamBuffer, ndcToHiZUV);
	gSSRTraceParamDef.gHiZUVToScreenUV.Set(mParamBuffer, HiZUVToScreenUV);
	gSSRTraceParamDef.gIntensity.Set(mParamBuffer, settings.Intensity);
	gSSRTraceParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);
	gSSRTraceParamDef.gTemporalJitter.Set(mParamBuffer, temporalJitter);

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

	Bind();

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad();
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

TemporalFilteringMat::TemporalFilteringMat()
{
	mParamBuffer = gTemporalFilteringParamDef.CreateBuffer();
	mTemporalParamBuffer = gTemporalResolveParamDef.CreateBuffer();

	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneDepth", mSceneDepthTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gPrevColor", mPrevColorTexture);

	mHasVelocityTexture = mVariation.GetBool("PER_PIXEL_VELOCITY");
	if(mHasVelocityTexture)
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gVelocity", mVelocityTexture);

	mParams->SetParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);
	mParams->SetParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "TemporalInput", mTemporalParamBuffer);

	SAMPLER_STATE_DESC pointSampDesc;
	pointSampDesc.MinFilter = FO_POINT;
	pointSampDesc.MagFilter = FO_POINT;
	pointSampDesc.MipFilter = FO_POINT;
	pointSampDesc.AddressMode.U = TAM_CLAMP;
	pointSampDesc.AddressMode.V = TAM_CLAMP;
	pointSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> pointSampState = SamplerState::Create(pointSampDesc);

	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler", pointSampState);
	else
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneDepth", pointSampState);

	SAMPLER_STATE_DESC linearSampDesc;
	linearSampDesc.MinFilter = FO_POINT;
	linearSampDesc.MagFilter = FO_POINT;
	linearSampDesc.MipFilter = FO_POINT;
	linearSampDesc.AddressMode.U = TAM_CLAMP;
	linearSampDesc.AddressMode.V = TAM_CLAMP;
	linearSampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> linearSampState = SamplerState::Create(linearSampDesc);
	if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler"))
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler", linearSampState);
	else
	{
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneColor", linearSampState);
		mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gPrevColor", linearSampState);
	}
}

void TemporalFilteringMat::Execute(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter, float exposure, const SPtr<RenderTarget>& destination)
{
	BS_RENMAT_PROFILE_BLOCK

	SPtr<Texture> velocityTex = velocity;
	if(!velocityTex)
		velocityTex = Texture::BLACK;

	mPrevColorTexture.Set(prevFrame);
	mSceneColorTexture.Set(curFrame);
	mSceneDepthTexture.Set(sceneDepth);

	if(mHasVelocityTexture)
		mVelocityTexture.Set(velocityTex);

	auto& colorProps = curFrame->GetProperties(); // Assuming prev and current frame are the same size
	auto& depthProps = sceneDepth->GetProperties();

	Vector4 colorPixelSize(1.0f / colorProps.GetWidth(), 1.0f / colorProps.GetHeight(), (float)colorProps.GetWidth(), (float)colorProps.GetHeight());
	Vector4 depthPixelSize(1.0f / depthProps.GetWidth(), 1.0f / depthProps.GetHeight(), (float)depthProps.GetWidth(), (float)depthProps.GetHeight());

	Vector4 velocityPixelSize(1.0f, 1.0f, 1.0f, 1.0f);
	if(mHasVelocityTexture)
	{
		auto& velocityProps = velocityTex->GetProperties();
		velocityPixelSize = Vector4(1.0f / velocityProps.GetWidth(), 1.0f / velocityProps.GetHeight(), (float)velocityProps.GetWidth(), (float)velocityProps.GetHeight());
	}

	gTemporalFilteringParamDef.gSceneColorTexelSize.Set(mParamBuffer, colorPixelSize);
	gTemporalFilteringParamDef.gSceneDepthTexelSize.Set(mParamBuffer, depthPixelSize);
	gTemporalFilteringParamDef.gVelocityTexelSize.Set(mParamBuffer, velocityPixelSize);
	gTemporalFilteringParamDef.gManualExposure.Set(mParamBuffer, 1.0f / exposure);

	Vector2 jitterUV;
	jitterUV.X = jitter.X * 0.5f;

	if((GetRenderBackendCapabilities().Conventions.UvYAxis == Conventions::Axis::Up) ^ (GetRenderBackendCapabilities().Conventions.NdcYAxis == Conventions::Axis::Down))
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

	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(destination);

	const RendererViewProperties& viewProps = view.GetProperties();
	const Rect2I& viewRect = viewProps.Target.ViewRect;

	Bind();

	if(viewProps.Target.NumSamples > 1)
		GetRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.Width, (float)viewRect.Height));
	else
		GetRendererUtility().DrawScreenQuad();
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

EncodeDepthMat::EncodeDepthMat()
{
	mParamBuffer = gEncodeDepthParamDef.CreateBuffer();

	mParams->SetParamBlockBuffer("Params", mParamBuffer);
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);

	SAMPLER_STATE_DESC sampDesc;
	sampDesc.MinFilter = FO_POINT;
	sampDesc.MagFilter = FO_POINT;
	sampDesc.MipFilter = FO_POINT;
	sampDesc.AddressMode.U = TAM_CLAMP;
	sampDesc.AddressMode.V = TAM_CLAMP;
	sampDesc.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> samplerState = SamplerState::Create(sampDesc);
	setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gInputSamp", "gInputTex", samplerState);
}

void EncodeDepthMat::Execute(const SPtr<Texture>& depth, float near, float far, const SPtr<RenderTarget>& output)
{
	BS_RENMAT_PROFILE_BLOCK

	mInputTexture.Set(depth);

	gEncodeDepthParamDef.gNear.Set(mParamBuffer, near);
	gEncodeDepthParamDef.gFar.Set(mParamBuffer, far);

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(output, 0, RT_COLOR0);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

MSAACoverageMat::MSAACoverageMat()
	: mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
{}

void MSAACoverageMat::Execute(const RendererView& view, GBufferTextures gbuffer)
{
	BS_RENMAT_PROFILE_BLOCK

	mGBufferParams.Bind(gbuffer);

	const Rect2I& viewRect = view.GetProperties().Target.ViewRect;
	SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
	mParams->SetParamBlockBuffer("PerCamera", perView);

	Bind();
	GetRendererUtility().DrawScreenQuad(Rect2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
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

MSAACoverageStencilMat::MSAACoverageStencilMat()
{
	mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gMSAACoverage", mCoverageTexParam);
}

void MSAACoverageStencilMat::Execute(const RendererView& view, const SPtr<Texture>& coverage)
{
	BS_RENMAT_PROFILE_BLOCK

	const Rect2I& viewRect = view.GetProperties().Target.ViewRect;
	mCoverageTexParam.Set(coverage);

	Bind();
	GetRendererUtility().DrawScreenQuad(Rect2(0, 0, (float)viewRect.Width, (float)viewRect.Height));
}
}}
