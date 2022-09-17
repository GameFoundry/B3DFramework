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

namespace bs { namespace ct
{
	void setSamplerState(const SPtr<GpuParams>& params, GpuProgramType gpType, const String& name,
		const String& secondaryName, const SPtr<SamplerState>& samplerState, bool optional = false)
	{
		if (params->HasSamplerState(gpType, name))
			params->SetSamplerState(gpType, name, samplerState);
		else
		{
			if(optional)
			{
				if (params->HasSamplerState(gpType, secondaryName))
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

		if (MSAA)
			gRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)rtProps.GetWidth(), (float)rtProps.GetHeight()));
		else
			gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC DownsampleMat::GetOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->GetProperties();
		
		UINT32 width = std::max(1, Math::CeilToInt(rtProps.GetWidth() * 0.5f));
		UINT32 height = std::max(1, Math::CeilToInt(rtProps.GetHeight() * 0.5f));

		return POOLED_RENDER_TEXTURE_DESC::Create2D(rtProps.GetFormat(), width, height, TU_RENDERTARGET);
	}

	DownsampleMat* DownsampleMat::GetVariation(UINT32 quality, bool msaa)
	{
		if(quality == 0)
		{
			if (msaa)
				return Get(GetVariation<0, true>());
			else
				return Get(GetVariation<0, false>());
		}
		else
		{
			if (msaa)
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
		defines.Set("THREADGROUP_SIZE_X", THREAD_GROUP_SIZE_X);
		defines.Set("THREADGROUP_SIZE_Y", THREAD_GROUP_SIZE_Y);
		defines.Set("LOOP_COUNT_X", LOOP_COUNT_X);
		defines.Set("LOOP_COUNT_Y", LOOP_COUNT_Y);
	}

	void EyeAdaptHistogramMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& output,
		const AutoExposureSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mSceneColor.Set(input);

		const TextureProperties& props = input->GetProperties();
		Vector4I offsetAndSize(0, 0, (INT32)props.GetWidth(), (INT32)props.GetHeight());

		gEyeAdaptHistogramParamDef.gHistogramParams.Set(mParamBuffer, GetHistogramScaleOffset(settings));
		gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.Set(mParamBuffer, offsetAndSize);

		Vector2I threadGroupCount = GetThreadGroupCount(input);
		gEyeAdaptHistogramParamDef.gThreadGroupCount.Set(mParamBuffer, threadGroupCount);

		// Dispatch
		mOutputTex.Set(output);

		Bind();

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.DispatchCompute(threadGroupCount.x, threadGroupCount.y);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramMat::GetOutputDesc(const SPtr<Texture>& target)
	{
		Vector2I threadGroupCount = GetThreadGroupCount(target);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, HISTOGRAM_NUM_TEXELS, numHistograms,
			TU_LOADSTORE);
	}

	Vector2I EyeAdaptHistogramMat::GetThreadGroupCount(const SPtr<Texture>& target)
	{
		const UINT32 texelsPerThreadGroupX = THREAD_GROUP_SIZE_X * LOOP_COUNT_X;
		const UINT32 texelsPerThreadGroupY = THREAD_GROUP_SIZE_Y * LOOP_COUNT_Y;

		const TextureProperties& props = target->GetProperties();
	
		Vector2I threadGroupCount;
		threadGroupCount.x = ((INT32)props.GetWidth() + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
		threadGroupCount.y = ((INT32)props.GetHeight() + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

		return threadGroupCount;
	}

	Vector2 EyeAdaptHistogramMat::GetHistogramScaleOffset(const AutoExposureSettings& settings)
	{
		float diff = settings.histogramLog2Max - settings.histogramLog2Min;
		float scale = 1.0f / diff;
		float offset = -settings.histogramLog2Min * scale;

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

	void EyeAdaptHistogramReduceMat::Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram,
		const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mHistogramTex.Set(histogram);

		SPtr<Texture> eyeAdaptationTex;
		if (prevFrame == nullptr) // Could be that this is the first run
			eyeAdaptationTex = Texture::WHITE;
		else
			eyeAdaptationTex = prevFrame;

		mEyeAdaptationTex.Set(eyeAdaptationTex);

		Vector2I threadGroupCount = EyeAdaptHistogramMat::GetThreadGroupCount(sceneColor);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		gEyeAdaptHistogramReduceParamDef.gThreadGroupCount.Set(mParamBuffer, numHistograms);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		Bind();

		Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2.0f);
		gRendererUtility().DrawScreenQuad(drawUV);

		rapi.SetRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramReduceMat::GetOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2,
			TU_RENDERTARGET);
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
		defines.Set("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_X);
		defines.Set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_Y);
	}

	void EyeAdaptationMat::Execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output,
		float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mReducedHistogramTex.Set(reducedHistogram);

		PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		Bind();
		gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptationMat::GetOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
	}

	void EyeAdaptationMat::PopulateParams(const SPtr<GpuParamBlockBuffer>& paramBuffer, float frameDelta,
		const AutoExposureSettings& settings, float exposureScale)
	{
		Vector2 histogramScaleAndOffset = EyeAdaptHistogramMat::GetHistogramScaleOffset(settings);

		Vector4 eyeAdaptationParams[3];
		eyeAdaptationParams[0].x = histogramScaleAndOffset.x;
		eyeAdaptationParams[0].y = histogramScaleAndOffset.y;

		float histogramPctHigh = Math::Clamp01(settings.histogramPctHigh);

		eyeAdaptationParams[0].z = std::min(Math::Clamp01(settings.histogramPctLow), histogramPctHigh);
		eyeAdaptationParams[0].w = histogramPctHigh;

		eyeAdaptationParams[1].x = std::min(settings.minEyeAdaptation, settings.maxEyeAdaptation);
		eyeAdaptationParams[1].y = settings.maxEyeAdaptation;

		eyeAdaptationParams[1].z = settings.eyeAdaptationSpeedUp;
		eyeAdaptationParams[1].w = settings.eyeAdaptationSpeedDown;

		eyeAdaptationParams[2].x = Math::Pow(2.0f, exposureScale);
		eyeAdaptationParams[2].y = frameDelta;

		eyeAdaptationParams[2].z = Math::Pow(2.0f, settings.histogramLog2Min);
		eyeAdaptationParams[2].w = 0.0f; // Unused

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
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;

		SPtr<SamplerState> samplerState = SamplerState::Create(desc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gInputSamp", "gInputTex", samplerState);
	}

	void EyeAdaptationBasicSetupMat::Execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output,
		float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mInputTex.Set(input);

		EyeAdaptationMat::PopulateParams(mParamBuffer, frameDelta, settings, exposureScale);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();

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

	void EyeAdaptationBasicMat::Execute(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame,
		const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mCurFrameTexParam.Set(curFrame);

		if (prevFrame == nullptr) // Could be that this is the first run
			mPrevFrameTexParam.Set(Texture::WHITE);
		else
			mPrevFrameTexParam.Set(prevFrame);

		EyeAdaptationMat::PopulateParams(mEyeAdaptationParamsBuffer, frameDelta, settings, exposureScale);

		auto& texProps = curFrame->GetProperties();
		Vector2I texSize = { (INT32)texProps.GetWidth(), (INT32)texProps.GetHeight() };

		gEyeAdaptationBasicParamsMatDef.gInputTexSize.Set(mParamsBuffer, texSize);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();

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
		defines.Set("LUT_SIZE", LUT_SIZE);
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
		rapi.DispatchCompute(LUT_SIZE / 8, LUT_SIZE / 8, LUT_SIZE);
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
		gRendererUtility().DrawScreenQuad();

		rapi.SetRenderTarget(nullptr);
	}

	void CreateTonemapLUTMat::PopulateParamBuffers(const RenderSettings& settings)
	{
		// Set parameters
		gCreateTonemapLUTParamDef.gGammaAdjustment.Set(mParamBuffer, 2.2f / settings.gamma);

		// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
		// gamma when running on Mac (value 2)
		gCreateTonemapLUTParamDef.gGammaCorrectionType.Set(mParamBuffer, 0);

		Vector4 tonemapParams[2];
		tonemapParams[0].x = settings.tonemapping.filmicCurveShoulderStrength;
		tonemapParams[0].y = settings.tonemapping.filmicCurveLinearStrength;
		tonemapParams[0].z = settings.tonemapping.filmicCurveLinearAngle;
		tonemapParams[0].w = settings.tonemapping.filmicCurveToeStrength;

		tonemapParams[1].x = settings.tonemapping.filmicCurveToeNumerator;
		tonemapParams[1].y = settings.tonemapping.filmicCurveToeDenominator;
		tonemapParams[1].z = settings.tonemapping.filmicCurveLinearWhitePoint;
		tonemapParams[1].w = 0.0f; // Unused

		gCreateTonemapLUTParamDef.gTonemapParams.Set(mParamBuffer, tonemapParams[0], 0);
		gCreateTonemapLUTParamDef.gTonemapParams.Set(mParamBuffer, tonemapParams[1], 1);

		// Set color grading params
		gCreateTonemapLUTParamDef.gSaturation.Set(mParamBuffer, settings.colorGrading.saturation);
		gCreateTonemapLUTParamDef.gContrast.Set(mParamBuffer, settings.colorGrading.contrast);
		gCreateTonemapLUTParamDef.gGain.Set(mParamBuffer, settings.colorGrading.gain);
		gCreateTonemapLUTParamDef.gOffset.Set(mParamBuffer, settings.colorGrading.offset);

		// Set white balance params
		gWhiteBalanceParamDef.gWhiteTemp.Set(mWhiteBalanceParamBuffer, settings.whiteBalance.temperature);
		gWhiteBalanceParamDef.gWhiteOffset.Set(mWhiteBalanceParamBuffer, settings.whiteBalance.tint);
	}

	POOLED_RENDER_TEXTURE_DESC CreateTonemapLUTMat::GetOutputDesc() const
	{
		if(mIs3D)
			return POOLED_RENDER_TEXTURE_DESC::Create3D(PF_RGBA8, LUT_SIZE, LUT_SIZE, LUT_SIZE, TU_LOADSTORE);
		
		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA8, LUT_SIZE * LUT_SIZE, LUT_SIZE, TU_RENDERTARGET);
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
		defines.Set("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE);
	}

	void TonemappingMat::Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation,
		const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output,
		const RenderSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& texProps = sceneColor->GetProperties();

		gTonemappingParamDef.gRawGamma.Set(mParamBuffer, 1.0f / settings.gamma);
		gTonemappingParamDef.gManualExposureScale.Set(mParamBuffer, Math::Pow(2.0f, settings.exposureScale));
		gTonemappingParamDef.gTexSize.Set(mParamBuffer, Vector2((float)texProps.GetWidth(), (float)texProps.GetHeight()));
		gTonemappingParamDef.gBloomTint.Set(mParamBuffer, settings.bloom.tint);
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
		gRendererUtility().DrawScreenQuad();
	}

	TonemappingMat* TonemappingMat::GetVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA)
	{
		if(volumeLUT)
		{
			if (gammaOnly)
			{
				if (autoExposure)
				{
					if (MSAA)
						return Get(GetVariation<true, true, true, true>());
					else
						return Get(GetVariation<true, true, true, false>());
				}
				else
				{
					if (MSAA)
						return Get(GetVariation<true, true, false, true>());
					else
						return Get(GetVariation<true, true, false, false>());
				}
			}
			else
			{
				if (autoExposure)
				{
					if (MSAA)
						return Get(GetVariation<true, false, true, true>());
					else
						return Get(GetVariation<true, false, true, false>());
				}
				else
				{
					if (MSAA)
						return Get(GetVariation<true, false, false, true>());
					else
						return Get(GetVariation<true, false, false, false>());
				}
			}
		}
		else
		{
			if (gammaOnly)
			{
				if (autoExposure)
				{
					if (MSAA)
						return Get(GetVariation<false, true, true, true>());
					else
						return Get(GetVariation<false, true, true, false>());
				}
				else
				{
					if (MSAA)
						return Get(GetVariation<false, true, false, true>());
					else
						return Get(GetVariation<false, true, false, false>());
				}
			}
			else
			{
				if (autoExposure)
				{
					if (MSAA)
						return Get(GetVariation<false, false, true, true>());
					else
						return Get(GetVariation<false, false, true, false>());
				}
				else
				{
					if (MSAA)
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

	void BloomClipMat::Execute(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation,
		const RenderSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		gBloomClipParamDef.gThreshold.Set(mParamBuffer, threshold);
		gBloomClipParamDef.gManualExposureScale.Set(mParamBuffer, Math::Pow(2.0f, settings.exposureScale));

		// Set parameters
		mInputTex.Set(input);
		mEyeAdaptationTex.Set(eyeAdaptation);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();

		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	BloomClipMat* BloomClipMat::GetVariation(bool autoExposure)
	{
		if (autoExposure)
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

	void ScreenSpaceLensFlareMat::Execute(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		gScreenSpaceLensFlareParamDef.gThreshold.Set(mParamBuffer, settings.threshold);
		gScreenSpaceLensFlareParamDef.gGhostCount.Set(mParamBuffer, settings.ghostCount);
		gScreenSpaceLensFlareParamDef.gGhostSpacing.Set(mParamBuffer, settings.ghostSpacing);
		gScreenSpaceLensFlareParamDef.gHaloRadius.Set(mParamBuffer, settings.haloRadius);
		gScreenSpaceLensFlareParamDef.gHaloThickness.Set(mParamBuffer, settings.haloThickness);
		gScreenSpaceLensFlareParamDef.gHaloThreshold.Set(mParamBuffer, settings.haloThreshold);
		gScreenSpaceLensFlareParamDef.gHaloAspectRatio.Set(mParamBuffer, settings.haloAspectRatio);
		gScreenSpaceLensFlareParamDef.gChromaticAberration.Set(mParamBuffer, settings.chromaticAberrationOffset);

		mInputTex.Set(input);
		mGradientTex.Set(RendererTextures::lensFlareGradient);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
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
			if (chromaticAberration)
				return Get(GetVariation<0, true>());

			return Get(GetVariation<0, false>());
		}
	}

	ChromaticAberrationParamDef gChromaticAberrationParamDef;

	constexpr int ChromaticAberrationMat::MAX_SAMPLES;

	ChromaticAberrationMat::ChromaticAberrationMat()
	{
		mParamBuffer = gChromaticAberrationParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gFringeTex", mFringeTex);
	}

	void ChromaticAberrationMat::Execute(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& texProps = input->GetProperties();
		
		// Set parameters
		gChromaticAberrationParamDef.gInputSize.Set(mParamBuffer,
			Vector2((float)texProps.GetWidth(), (float)texProps.GetHeight()));

		gChromaticAberrationParamDef.gShiftAmount.Set(mParamBuffer, settings.shiftAmount);
		
		SPtr<Texture> fringeTex;
		if (settings.fringeTexture)
			fringeTex = settings.fringeTexture;
		else
			fringeTex = RendererTextures::chromaticAberrationFringe;
		
		mInputTex.Set(input);
		mFringeTex.Set(fringeTex);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	ChromaticAberrationMat* ChromaticAberrationMat::GetVariation(ChromaticAberrationType type)
	{
		if (type == ChromaticAberrationType::Complex)
			return Get(GetVariation<false>());

		return Get(GetVariation<true>());
	}

	void ChromaticAberrationMat::InitDefinesInternal(ShaderDefines& defines)
	{
		defines.Set("MAX_SAMPLES", MAX_SAMPLES);
	}

	FilmGrainParamDef gFilmGrainParamDef;

	FilmGrainMat::FilmGrainMat()
	{
		mParamBuffer = gFilmGrainParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Params", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	}

	void FilmGrainMat::Execute(const SPtr<Texture>& input, float time,
		const FilmGrainSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		gFilmGrainParamDef.gIntensity.Set(mParamBuffer, settings.intensity);
		gFilmGrainParamDef.gTime.Set(mParamBuffer, settings.speed * time);

		mInputTex.Set(input);

		// Render
		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
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
		defines.Set("MAX_NUM_SAMPLES", MAX_BLUR_SAMPLES);
	}

	void GaussianBlurMat::Execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination,
		const Color& tint, const SPtr<Texture>& additive)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = source->GetProperties();
		const RenderTextureProperties& dstProps = destination->GetProperties();

		POOLED_RENDER_TEXTURE_DESC tempTextureDesc = POOLED_RENDER_TEXTURE_DESC::Create2D(srcProps.GetFormat(),
			dstProps.width, dstProps.height, TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = gGpuResourcePool().Get(tempTextureDesc);

		// Horizontal pass
		{
			PopulateBuffer(mParamBuffer, DirHorizontal, source, filterSize, Color::White);
			mInputTexture.Set(source);

			if(mIsAdditive)
				mAdditiveTexture.Set(Texture::BLACK);

			RenderAPI& rapi = RenderAPI::Instance();
			rapi.SetRenderTarget(tempTexture->renderTexture);

			Bind();
			gRendererUtility().DrawScreenQuad();
		}

		// Vertical pass
		{
			PopulateBuffer(mParamBuffer, DirVertical, source, filterSize, tint);
			mInputTexture.Set(tempTexture->texture);

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
			gRendererUtility().DrawScreenQuad();
		}
	}

	UINT32 GaussianBlurMat::CalcStdDistribution(float filterRadius, std::array<float, MAX_BLUR_SAMPLES>& weights,
		std::array<float, MAX_BLUR_SAMPLES>& offsets)
	{
		filterRadius = Math::Clamp(filterRadius, 0.00001f, (float)(MAX_BLUR_SAMPLES - 1));
		INT32 intFilterRadius = std::min(Math::CeilToInt(filterRadius), MAX_BLUR_SAMPLES - 1);

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
		UINT32 numSamples = 0;
		float totalWeight = 0.0f;
		for(int i = -intFilterRadius; i < intFilterRadius; i += 2)
		{
			float w1 = normalDistribution(i, scale);
			float w2 = normalDistribution(i + 1, scale);

			float w3 = w1 + w2;
			float o = (float)i + w2/w3; // Relative to first sample

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
		for(UINT32 i = 0; i < numSamples; i++)
			weights[i] *= invTotalWeight;

		return numSamples;
	}

	float GaussianBlurMat::CalcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir)
	{
		scale = Math::Clamp01(scale);

		UINT32 length;
		if (filterDir == DirHorizontal)
			length = source->GetProperties().GetWidth();
		else
			length = source->GetProperties().GetHeight();

		// Divide by two because we need the radius
		return std::min(length * scale / 2, (float)MAX_BLUR_SAMPLES - 1);
	}

	void GaussianBlurMat::PopulateBuffer(const SPtr<GpuParamBlockBuffer>& buffer, Direction direction,
		const SPtr<Texture>& source, float filterSize, const Color& tint)
	{
		const TextureProperties& srcProps = source->GetProperties();

		Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

		std::array<float, MAX_BLUR_SAMPLES> sampleOffsets;
		std::array<float, MAX_BLUR_SAMPLES> sampleWeights;

		const float kernelRadius = CalcKernelRadius(source, filterSize, direction);
		const UINT32 numSamples = CalcStdDistribution(kernelRadius, sampleWeights, sampleOffsets);

		for (UINT32 i = 0; i < numSamples; ++i)
		{
			Vector4 weight(tint.r, tint.g, tint.b, tint.a);
			weight *= sampleWeights[i];

			gGaussianBlurParamDef.gSampleWeights.Set(buffer, weight, i);
		}

		UINT32 axis0 = direction == DirHorizontal ? 0 : 1;
		UINT32 axis1 = (axis0 + 1) % 2;

		for (UINT32 i = 0; i < (numSamples + 1) / 2; ++i)
		{
			UINT32 remainder = std::min(2U, numSamples - i * 2);

			Vector4 offset;
			offset[axis0] = sampleOffsets[i * 2 + 0] * invTexSize[axis0];
			offset[axis1] = 0.0f;

			if (remainder == 2)
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
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::Create(desc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gColorSamp", "gColorTex", samplerState);
	}

	void GaussianDOFSeparateMat::Execute(const SPtr<Texture>& color, const SPtr<Texture>& depth,
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = color->GetProperties();

		UINT32 outputWidth = std::max(1U, srcProps.GetWidth() / 2);
		UINT32 outputHeight = std::max(1U, srcProps.GetHeight() / 2);

		POOLED_RENDER_TEXTURE_DESC outputTexDesc = POOLED_RENDER_TEXTURE_DESC::Create2D(srcProps.GetFormat(),
			outputWidth, outputHeight, TU_RENDERTARGET);
		mOutput0 = gGpuResourcePool().Get(outputTexDesc);

		bool near = mVariation.GetBool("NEAR");
		bool far = mVariation.GetBool("FAR");

		SPtr<RenderTexture> rt;
		if (near && far)
		{
			mOutput1 = gGpuResourcePool().Get(outputTexDesc);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = mOutput0->texture;
			rtDesc.colorSurfaces[1].texture = mOutput1->texture;

			rt = RenderTexture::Create(rtDesc);
		}
		else
			rt = mOutput0->renderTexture;

		Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

		gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
		gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.focalDistance - settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.focalDistance + settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.nearTransitionRange);
		gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.farTransitionRange);

		mColorTexture.Set(color);
		mDepthTexture.Set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(rt);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	SPtr<PooledRenderTexture> GaussianDOFSeparateMat::GetOutput(UINT32 idx)
	{
		if (idx == 0)
			return mOutput0;
		else if (idx == 1)
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
		if (near)
		{
			if (far)
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

	void GaussianDOFCombineMat::Execute(const SPtr<Texture>& focused, const SPtr<Texture>& near,
		const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output,
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = focused->GetProperties();

		Vector2 invTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());

		gGaussianDOFParamDef.gHalfPixelOffset.Set(mParamBuffer, invTexSize * 0.5f);
		gGaussianDOFParamDef.gNearBlurPlane.Set(mParamBuffer, settings.focalDistance - settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gFarBlurPlane.Set(mParamBuffer, settings.focalDistance + settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gInvNearBlurRange.Set(mParamBuffer, 1.0f / settings.nearTransitionRange);
		gGaussianDOFParamDef.gInvFarBlurRange.Set(mParamBuffer, 1.0f / settings.farTransitionRange);

		mFocusedTexture.Set(focused);
		mNearTexture.Set(near);
		mFarTexture.Set(far);
		mDepthTexture.Set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	GaussianDOFCombineMat* GaussianDOFCombineMat::GetVariation(bool near, bool far)
	{
		if (near)
		{
			if (far)
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

	void BokehDOFPrepareMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
		const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
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
		if (MSAA)
			gRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)srcProps.GetWidth(), (float)srcProps.GetHeight()));
		else
			gRendererUtility().DrawScreenQuad();
	}

	POOLED_RENDER_TEXTURE_DESC BokehDOFPrepareMat::GetOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->GetProperties();

		UINT32 width = std::max(1U, Math::DivideAndRoundUp(rtProps.GetWidth(), 2U));
		UINT32 height = std::max(1U, Math::DivideAndRoundUp(rtProps.GetHeight(), 2U));

		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
	}

	BokehDOFPrepareMat* BokehDOFPrepareMat::GetVariation(bool msaa)
	{
		if (msaa)
			return Get(GetVariation<true>());
		else
			return Get(GetVariation<false>());
	}

	BokehDOFParamDef gBokehDOFParamDef;

	constexpr UINT32 BokehDOFMat::NEAR_FAR_PADDING;
	constexpr UINT32 BokehDOFMat::QUADS_PER_TILE;

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
		SPtr<VertexDataDesc> tileVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		tileVertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);

		mTileVertexDecl = VertexDeclaration::Create(tileVertexDesc);

		// Prepare vertex buffer for rendering tiles
		VERTEX_BUFFER_DESC tileVertexBufferDesc;
		tileVertexBufferDesc.numVerts = QUADS_PER_TILE * 4;
		tileVertexBufferDesc.vertexSize = tileVertexDesc->GetVertexStride();

		mTileVertexBuffer = VertexBuffer::Create(tileVertexBufferDesc);

		auto* const vertexData = (Vector2*)mTileVertexBuffer->Lock(GBL_WRITE_ONLY_DISCARD);
		for (UINT32 i = 0; i < QUADS_PER_TILE; i++)
		{
			vertexData[i * 4 + 0] = Vector2(0.0f, 0.0f);
			vertexData[i * 4 + 1] = Vector2(1.0f, 0.0f);
			vertexData[i * 4 + 2] = Vector2(0.0f, 1.0f);
			vertexData[i * 4 + 3] = Vector2(1.0f, 1.0f);
		}

		mTileVertexBuffer->Unlock();

		// Prepare indices for rendering tiles
		INDEX_BUFFER_DESC tileIndexBufferDesc;
		tileIndexBufferDesc.indexType = IT_16BIT;
		tileIndexBufferDesc.numIndices = QUADS_PER_TILE * 6;

		mTileIndexBuffer = IndexBuffer::Create(tileIndexBufferDesc);

		auto* const indices = (UINT16*)mTileIndexBuffer->Lock(GBL_WRITE_ONLY_DISCARD);

		const Conventions& rapiConventions = gCaps().conventions;
		for (UINT32 i = 0; i < QUADS_PER_TILE; i++)
		{
			// If UV is flipped, then our tile will be upside down so we need to change index order so it doesn't
			// get culled.
			if (rapiConventions.uvYAxis == Conventions::Axis::Up)
			{
				indices[i * 6 + 0] = i * 4 + 2; indices[i * 6 + 1] = i * 4 + 1; indices[i * 6 + 2] = i * 4 + 0;
				indices[i * 6 + 3] = i * 4 + 2; indices[i * 6 + 4] = i * 4 + 3; indices[i * 6 + 5] = i * 4 + 1;
			}
			else
			{
				indices[i * 6 + 0] = i * 4 + 0; indices[i * 6 + 1] = i * 4 + 1; indices[i * 6 + 2] = i * 4 + 2;
				indices[i * 6 + 3] = i * 4 + 1; indices[i * 6 + 4] = i * 4 + 3; indices[i * 6 + 5] = i * 4 + 2;
			}
		}

		mTileIndexBuffer->Unlock();
	}

	void BokehDOFMat::InitDefinesInternal(ShaderDefines& defines)
	{
		defines.Set("QUADS_PER_TILE", QUADS_PER_TILE);
	}

	void BokehDOFMat::Execute(const SPtr<Texture>& input, const RendererView& view,
		const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = input->GetProperties();
		const RenderTargetProperties& dstProps = output->GetProperties();

		Vector2 inputInvTexSize(1.0f / srcProps.GetWidth(), 1.0f / srcProps.GetHeight());
		Vector2 outputInvTexSize(1.0f / dstProps.width, 1.0f / dstProps.height);
		gBokehDOFParamDef.gInvInputSize.Set(mParamBuffer, inputInvTexSize);
		gBokehDOFParamDef.gInvOutputSize.Set(mParamBuffer, outputInvTexSize);
		gBokehDOFParamDef.gAdaptiveThresholdCOC.Set(mParamBuffer, settings.adaptiveRadiusThreshold);
		gBokehDOFParamDef.gAdaptiveThresholdColor.Set(mParamBuffer, settings.adaptiveColorThreshold);
		gBokehDOFParamDef.gLayerPixelOffset.Set(mParamBuffer, (INT32)srcProps.GetHeight() + (INT32)NEAR_FAR_PADDING);
		gBokehDOFParamDef.gInvDepthRange.Set(mParamBuffer, 1.0f / settings.occlusionDepthRange);

		float bokehSize = settings.maxBokehSize * srcProps.GetWidth();
		gBokehDOFParamDef.gBokehSize.Set(mParamBuffer, Vector2(bokehSize, bokehSize));

		Vector2I imageSize(srcProps.GetWidth(), srcProps.GetHeight());

		// TODO - Allow tile count to halve (i.e. half sampling rate)
		Vector2I tileCount = imageSize / 1;
		gBokehDOFParamDef.gTileCount.Set(mParamBuffer, tileCount);

		PopulateDofCommonParams(mCommonParamBuffer, settings, view);
		mInputTextureVS.Set(input);
		mInputTextureFS.Set(input);

		SPtr<Texture> bokehTexture = settings.bokehShape;
		if(bokehTexture == nullptr)
			bokehTexture = RendererTextures::bokehFlare;

		mBokehTexture.Set(bokehTexture);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
		rapi.ClearRenderTarget(FBT_COLOR, Color::ZERO);
		rapi.SetVertexDeclaration(mTileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { mTileVertexBuffer };
		rapi.SetVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.SetIndexBuffer(mTileIndexBuffer);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);

		Bind();
		const UINT32 numInstances = Math::DivideAndRoundUp((UINT32)(tileCount.x * tileCount.y), QUADS_PER_TILE);
		rapi.DrawIndexed(0, QUADS_PER_TILE * 6, 0, QUADS_PER_TILE * 4, numInstances);
	}

	POOLED_RENDER_TEXTURE_DESC BokehDOFMat::GetOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->GetProperties();

		UINT32 width = rtProps.GetWidth();
		UINT32 height = rtProps.GetHeight() * 2 + NEAR_FAR_PADDING;

		return POOLED_RENDER_TEXTURE_DESC::Create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
	}

	void BokehDOFMat::PopulateDofCommonParams(const SPtr<GpuParamBlockBuffer>& buffer, const DepthOfFieldSettings& settings,
		const RendererView& view)
	{
		gDepthOfFieldCommonParamDef.gFocalPlaneDistance.Set(buffer, settings.focalDistance);
		gDepthOfFieldCommonParamDef.gApertureSize.Set(buffer, settings.apertureSize * 0.001f); // mm to m
		gDepthOfFieldCommonParamDef.gFocalLength.Set(buffer, settings.focalLength * 0.001f); // mm to m
		gDepthOfFieldCommonParamDef.gInFocusRange.Set(buffer, settings.focalRange);
		gDepthOfFieldCommonParamDef.gNearTransitionRegion.Set(buffer, settings.nearTransitionRange);
		gDepthOfFieldCommonParamDef.gFarTransitionRegion.Set(buffer, settings.farTransitionRange);

		float sensorSize, imageSize;
		if(settings.sensorSize.x < settings.sensorSize.y)
		{
			sensorSize = settings.sensorSize.x;
			imageSize = (float)view.GetProperties().target.targetWidth;
		}
		else
		{
			sensorSize = settings.sensorSize.y;
			imageSize = (float)view.GetProperties().target.targetHeight;
		}

		gDepthOfFieldCommonParamDef.gSensorSize.Set(buffer, sensorSize);
		gDepthOfFieldCommonParamDef.gImageSize.Set(buffer, imageSize);
		gDepthOfFieldCommonParamDef.gMaxBokehSize.Set(buffer, Math::Clamp01(settings.maxBokehSize) * imageSize);
	}

	BokehDOFMat* BokehDOFMat::GetVariation(bool depthOcclusion)
	{
		if (depthOcclusion)
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

	void BokehDOFCombineMat::Execute(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused,
		const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& focusedProps = focused->GetProperties();
		const TextureProperties& unfocusedProps = unfocused->GetProperties();
		UINT32 halfHeight = std::max(1U, Math::DivideAndRoundUp(focusedProps.GetHeight(), 2U));

		float uvScale = halfHeight / (float)unfocusedProps.GetHeight();
		float uvOffset = (halfHeight + BokehDOFMat::NEAR_FAR_PADDING) / (float)unfocusedProps.GetHeight();

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
		gRendererUtility().DrawScreenQuad();
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
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::Create(pointSampDesc);

		if (mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp", pointSampState);
	}

	void MotionBlurMat::Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
		const MotionBlurSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		UINT32 numSamples;
		switch(settings.quality)
		{
		default:
		case MotionBlurQuality::VeryLow: numSamples = 4; break;
		case MotionBlurQuality::Low: numSamples = 6; break;
		case MotionBlurQuality::Medium: numSamples = 8; break;
		case MotionBlurQuality::High: numSamples = 12;  break;
		case MotionBlurQuality::Ultra: numSamples = 16;  break;
		}
		
		gMotionBlurParamDef.gHalfNumSamples.Set(mParamBuffer, numSamples / 2);

		mInputTexture.Set(input);
		mDepthTexture.Set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(output);

		Bind();
		gRendererUtility().DrawScreenQuad();
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
			inputSampDesc.minFilter = FO_POINT;
			inputSampDesc.magFilter = FO_POINT;
			inputSampDesc.mipFilter = FO_POINT;

			SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
			setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gDepthSamp", "gDepthTex", inputSampState);
		}
	}

	void BuildHiZMat::Execute(const SPtr<Texture>& source, UINT32 srcMip, const Rect2& srcRect, const Rect2& dstRect,
		const SPtr<RenderTexture>& output)
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
		gRendererUtility().DrawScreenQuad(srcRect);

		rapi.SetViewport(Rect2(0, 0, 1, 1));
	}

	BuildHiZMat* BuildHiZMat::GetVariation(bool noTextureViews)
	{
		if (noTextureViews)
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
		gRendererUtility().DrawScreenQuad();
	}

	SSAOParamDef gSSAOParamDef;

	SSAOMat::SSAOMat()
	{
		bool isFinal = mVariation.GetBool("FINAL_AO");
		bool mixWithUpsampled = mVariation.GetBool("MIX_WITH_UPSAMPLED");

		mParamBuffer = gSSAOParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Input", mParamBuffer);

		if (isFinal)
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
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
		if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			if (isFinal)
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
		randomSampDesc.minFilter = FO_POINT;
		randomSampDesc.magFilter = FO_POINT;
		randomSampDesc.mipFilter = FO_POINT;
		randomSampDesc.addressMode.u = TAM_WRAP;
		randomSampDesc.addressMode.v = TAM_WRAP;
		randomSampDesc.addressMode.w = TAM_WRAP;

		SPtr<SamplerState> randomSampState = SamplerState::Create(randomSampDesc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gRandomSamp", "gRandomTex", randomSampState);
	}

	void SSAOMat::Execute(const RendererView& view, const SSAOTextureInputs& textures,
		const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Scale that can be used to adjust how quickly does AO radius increase with downsampled AO. This yields a very
		// small AO radius at highest level, and very large radius at lowest level
		static const float DOWNSAMPLE_SCALE = 4.0f;

		const RendererViewProperties& viewProps = view.GetProperties();
		const RenderTargetProperties& rtProps = destination->GetProperties();

		Vector2 tanHalfFOV;
		tanHalfFOV.x = 1.0f / viewProps.projTransform[0][0];
		tanHalfFOV.y = 1.0f / viewProps.projTransform[1][1];

		float cotHalfFOV = viewProps.projTransform[0][0];

		// Downsampled AO uses a larger AO radius (in higher resolutions this would cause too much cache trashing). This
		// means if only full res AO is used, then only AO from nearby geometry will be calculated.
		float viewScale = viewProps.target.viewRect.width / (float)rtProps.width;

		// Ramp up the radius exponentially. c^log2(x) function chosen arbitrarily, as it ramps up the radius in a nice way
		float scale = pow(DOWNSAMPLE_SCALE, Math::Log2(viewScale));

		// Determine maximum radius scale (division by 4 because we don't downsample more than quarter-size)
		float maxScale = pow(DOWNSAMPLE_SCALE, Math::Log2(4.0f));

		// Normalize the scale in [0, 1] range
		scale /= maxScale;

		float radius = settings.radius * scale;

		// Factors used for scaling the AO contribution with range
		Vector2 fadeMultiplyAdd;
		fadeMultiplyAdd.x = 1.0f / settings.fadeRange;
		fadeMultiplyAdd.y = -settings.fadeDistance / settings.fadeRange;

		gSSAOParamDef.gSampleRadius.Set(mParamBuffer, radius);
		gSSAOParamDef.gCotHalfFOV.Set(mParamBuffer, cotHalfFOV);
		gSSAOParamDef.gTanHalfFOV.Set(mParamBuffer, tanHalfFOV);
		gSSAOParamDef.gWorldSpaceRadiusMask.Set(mParamBuffer, 1.0f);
		gSSAOParamDef.gBias.Set(mParamBuffer, (settings.bias * viewScale) / 1000.0f);
		gSSAOParamDef.gFadeMultiplyAdd.Set(mParamBuffer, fadeMultiplyAdd);
		gSSAOParamDef.gPower.Set(mParamBuffer, settings.power);
		gSSAOParamDef.gIntensity.Set(mParamBuffer, settings.intensity);
		
		bool upsample = mVariation.GetBool("MIX_WITH_UPSAMPLED");
		if(upsample)
		{
			const TextureProperties& props = textures.aoDownsampled->GetProperties();

			Vector2 downsampledPixelSize;
			downsampledPixelSize.x = 1.0f / props.GetWidth();
			downsampledPixelSize.y = 1.0f / props.GetHeight();

			gSSAOParamDef.gDownsampledPixelSize.Set(mParamBuffer, downsampledPixelSize);
		}

		// Generate a scale which we need to use in order to achieve tiling
		const TextureProperties& rndProps = textures.randomRotations->GetProperties();
		UINT32 rndWidth = rndProps.GetWidth();
		UINT32 rndHeight = rndProps.GetHeight();

		//// Multiple of random texture size, rounded up
		UINT32 scaleWidth = (rtProps.width + rndWidth - 1) / rndWidth;
		UINT32 scaleHeight = (rtProps.height + rndHeight - 1) / rndHeight;

		Vector2 randomTileScale((float)scaleWidth, (float)scaleHeight);
		gSSAOParamDef.gRandomTileScale.Set(mParamBuffer, randomTileScale);

		mSetupAOTexture.Set(textures.aoSetup);

		bool finalPass = mVariation.GetBool("FINAL_AO");
		if (finalPass)
		{
			mDepthTexture.Set(textures.sceneDepth);
			mNormalsTexture.Set(textures.sceneNormals);
		}

		if (upsample)
			mDownsampledAOTexture.Set(textures.aoDownsampled);
		
		mRandomTexture.Set(textures.randomRotations);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(destination);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	SSAOMat* SSAOMat::GetVariation(bool upsample, bool finalPass, int quality)
	{
#define PICK_MATERIAL(QUALITY)															\
		if(upsample)																	\
			if(finalPass)																\
				return Get(GetVariation<true, true, QUALITY>());						\
			else																		\
				return Get(GetVariation<true, false, QUALITY>());						\
		else																			\
			if(finalPass)																\
				return Get(GetVariation<false, true, QUALITY>());						\
			else																		\
				return Get(GetVariation<false, false, QUALITY>());						\

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
		inputSampDesc.minFilter = FO_LINEAR;
		inputSampDesc.magFilter = FO_LINEAR;
		inputSampDesc.mipFilter = FO_LINEAR;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);

		if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gNormalsTex", inputSampState);
		}
	}

	void SSAODownsampleMat::Execute(const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals,
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.GetProperties();
		const RenderTargetProperties& rtProps = destination->GetProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / rtProps.width;
		pixelSize.y = 1.0f / rtProps.height;

		float scale = viewProps.target.viewRect.width / (float)rtProps.width;

		gSSAODownsampleParamDef.gPixelSize.Set(mParamBuffer, pixelSize);
		gSSAODownsampleParamDef.gInvDepthThreshold.Set(mParamBuffer, (1.0f / depthRange) / scale);

		mDepthTexture.Set(depth);
		mNormalsTexture.Set(normals);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(destination);

		Bind();
		gRendererUtility().DrawScreenQuad();
	}

	SSAOBlurParamDef gSSAOBlurParamDef;

	SSAOBlurMat::SSAOBlurMat()
	{
		mParamBuffer = gSSAOBlurParamDef.CreateBuffer();

		mParams->SetParamBlockBuffer("Input", mParamBuffer);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mAOTexture);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::Create(inputSampDesc);
		if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gInputTex", inputSampState);
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
		}
	}

	void SSAOBlurMat::Execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth,
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.GetProperties();
		const TextureProperties& texProps = ao->GetProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / texProps.GetWidth();
		pixelSize.y = 1.0f / texProps.GetHeight();

		Vector2 pixelOffset(BsZero);
		if (mVariation.GetBool("DIR_HORZ"))
			pixelOffset.x = pixelSize.x;
		else
			pixelOffset.y = pixelSize.y;

		float scale = viewProps.target.viewRect.width / (float)texProps.GetWidth();

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
		gRendererUtility().DrawScreenQuad();
	}

	SSAOBlurMat* SSAOBlurMat::GetVariation(bool horizontal)
	{
		if (horizontal)
			return Get(GetVariation<true>());
		
		return Get(GetVariation<false>());
	}

	SSRStencilParamDef gSSRStencilParamDef;

	SSRStencilMat::SSRStencilMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParamBuffer = gSSRStencilParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Input", mParamBuffer);
	}

	void SSRStencilMat::Execute(const RendererView& view, GBufferTextures gbuffer,
		const ScreenSpaceReflectionsSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		mGBufferParams.Bind(gbuffer);

		Vector2 roughnessScaleBias = SSRTraceMat::CalcRoughnessFadeScaleBias(settings.maxRoughness);
		gSSRStencilParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		const RendererViewProperties& viewProps = view.GetProperties();
		const Rect2I& viewRect = viewProps.target.viewRect;
		Bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
			gRendererUtility().DrawScreenQuad();
	}

	SSRStencilMat* SSRStencilMat::GetVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return Get(GetVariation<true, true>());

			return Get(GetVariation<true, false>());
		}
		else
			return Get(GetVariation<false, false>());
	}

	SSRTraceParamDef gSSRTraceParamDef;

	SSRTraceMat::SSRTraceMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParamBuffer = gSSRTraceParamDef.CreateBuffer();

		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gHiZ", mHiZTexture);

		if(mParams->HasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			mParams->SetParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> hiZSamplerState = SamplerState::Create(desc);
		if (mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp", hiZSamplerState);
		else if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ", hiZSamplerState);
	}

	void SSRTraceMat::Execute(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor,
			const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings,
			const SPtr<RenderTarget>& destination)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.GetProperties();

		const TextureProperties& hiZProps = hiZ->GetProperties();

		mGBufferParams.Bind(gbuffer);
		mSceneColorTexture.Set(sceneColor);
		mHiZTexture.Set(hiZ);
		
		Rect2I viewRect = viewProps.target.viewRect;

		// Maps from NDC to UV [0, 1]
		Vector4 ndcToHiZUV;
		ndcToHiZUV.x = 0.5f;
		ndcToHiZUV.y = -0.5f;
		ndcToHiZUV.z = 0.5f;
		ndcToHiZUV.w = 0.5f;

		// Either of these flips the Y axis, but if they're both true they cancel out
		const Conventions& rapiConventions = gCaps().conventions;

		if ((rapiConventions.uvYAxis == Conventions::Axis::Up) ^ (rapiConventions.ndcYAxis == Conventions::Axis::Down))
			ndcToHiZUV.y = -ndcToHiZUV.y;
		
		// Maps from [0, 1] to area of HiZ where depth is stored in
		ndcToHiZUV.x *= (float)viewRect.width / hiZProps.GetWidth();
		ndcToHiZUV.y *= (float)viewRect.height / hiZProps.GetHeight();
		ndcToHiZUV.z *= (float)viewRect.width / hiZProps.GetWidth();
		ndcToHiZUV.w *= (float)viewRect.height / hiZProps.GetHeight();
		
		// Maps from HiZ UV to [0, 1] UV
		Vector2 HiZUVToScreenUV;
		HiZUVToScreenUV.x = hiZProps.GetWidth() / (float)viewRect.width;
		HiZUVToScreenUV.y = hiZProps.GetHeight() / (float)viewRect.height;

		// Used for roughness fading
		Vector2 roughnessScaleBias = CalcRoughnessFadeScaleBias(settings.maxRoughness);

		UINT32 temporalJitter = (viewProps.frameIdx % 8) * 1503;

		Vector2I bufferSize(viewRect.width, viewRect.height);
		gSSRTraceParamDef.gHiZSize.Set(mParamBuffer, bufferSize);
		gSSRTraceParamDef.gHiZNumMips.Set(mParamBuffer, hiZProps.GetNumMipmaps());
		gSSRTraceParamDef.gNDCToHiZUV.Set(mParamBuffer, ndcToHiZUV);
		gSSRTraceParamDef.gHiZUVToScreenUV.Set(mParamBuffer, HiZUVToScreenUV);
		gSSRTraceParamDef.gIntensity.Set(mParamBuffer, settings.intensity);
		gSSRTraceParamDef.gRoughnessScaleBias.Set(mParamBuffer, roughnessScaleBias);
		gSSRTraceParamDef.gTemporalJitter.Set(mParamBuffer, temporalJitter);

		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(destination, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

		Bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
			gRendererUtility().DrawScreenQuad();
	}

	Vector2 SSRTraceMat::CalcRoughnessFadeScaleBias(float maxRoughness)
	{
		const static float RANGE_SCALE = 2.0f;

		Vector2 scaleBias;
		scaleBias.x = -RANGE_SCALE / (-1.0f + maxRoughness);
		scaleBias.y = (RANGE_SCALE * maxRoughness) / (-1.0f + maxRoughness);

		return scaleBias;
	}

	SSRTraceMat* SSRTraceMat::GetVariation(UINT32 quality, bool msaa, bool singleSampleMSAA)
	{
#define PICK_MATERIAL(QUALITY)											\
		if(msaa)														\
			if(singleSampleMSAA)										\
				return Get(GetVariation<QUALITY, true, true>());		\
			else														\
				return Get(GetVariation<QUALITY, true, false>());		\
		else															\
				return Get(GetVariation<QUALITY, false, false>());		\

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
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::Create(pointSampDesc);

		if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler", pointSampState);
		else
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneDepth", pointSampState);

		SAMPLER_STATE_DESC linearSampDesc;
		linearSampDesc.minFilter = FO_POINT;
		linearSampDesc.magFilter = FO_POINT;
		linearSampDesc.mipFilter = FO_POINT;
		linearSampDesc.addressMode.u = TAM_CLAMP;
		linearSampDesc.addressMode.v = TAM_CLAMP;
		linearSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> linearSampState = SamplerState::Create(linearSampDesc);
		if(mParams->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler"))
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler", linearSampState);
		else
		{
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneColor", linearSampState);
			mParams->SetSamplerState(GPT_FRAGMENT_PROGRAM, "gPrevColor", linearSampState);
		}
	}

	void TemporalFilteringMat::Execute(const RendererView& view, const SPtr<Texture>& prevFrame,
		const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth,
		const Vector2& jitter, float exposure, const SPtr<RenderTarget>& destination)
	{
		BS_RENMAT_PROFILE_BLOCK

		SPtr<Texture> velocityTex = velocity;
		if (!velocityTex)
			velocityTex = Texture::BLACK;

		mPrevColorTexture.Set(prevFrame);
		mSceneColorTexture.Set(curFrame);
		mSceneDepthTexture.Set(sceneDepth);

		if(mHasVelocityTexture)
			mVelocityTexture.Set(velocityTex);

		auto& colorProps = curFrame->GetProperties(); // Assuming prev and current frame are the same size
		auto& depthProps = sceneDepth->GetProperties();

		Vector4 colorPixelSize(1.0f / colorProps.GetWidth(), 1.0f / colorProps.GetHeight(),
			(float)colorProps.GetWidth(), (float)colorProps.GetHeight());
		Vector4 depthPixelSize(1.0f / depthProps.GetWidth(), 1.0f / depthProps.GetHeight(),
			(float)depthProps.GetWidth(), (float)depthProps.GetHeight());

		Vector4 velocityPixelSize(1.0f, 1.0f, 1.0f, 1.0f);
		if(mHasVelocityTexture)
		{
			auto& velocityProps = velocityTex->GetProperties();
			velocityPixelSize = Vector4(1.0f / velocityProps.GetWidth(), 1.0f / velocityProps.GetHeight(),
				(float)velocityProps.GetWidth(), (float)velocityProps.GetHeight());
		}

		gTemporalFilteringParamDef.gSceneColorTexelSize.Set(mParamBuffer, colorPixelSize);
		gTemporalFilteringParamDef.gSceneDepthTexelSize.Set(mParamBuffer, depthPixelSize);
		gTemporalFilteringParamDef.gVelocityTexelSize.Set(mParamBuffer, velocityPixelSize);
		gTemporalFilteringParamDef.gManualExposure.Set(mParamBuffer, 1.0f / exposure);

		Vector2 jitterUV;
		jitterUV.x = jitter.x * 0.5f;

		if ((gCaps().conventions.uvYAxis == Conventions::Axis::Up) ^ (gCaps().conventions.ndcYAxis == Conventions::Axis::Down))
			jitterUV.y = jitter.y * 0.5f;
		else
			jitterUV.y = jitter.y * -0.5f;
		
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
			static const Vector2 sampleOffsets[] =
			{
				{  0.0f, -1.0f },
				{ -1.0f,  0.0f },
				{  0.0f,  0.0f },
				{  1.0f,  0.0f },
				{  0.0f,  1.0f },
			};

			for (UINT32 i = 0; i < 5; ++i)
			{
				// Get rid of jitter introduced by the projection matrix
				Vector2 offset = sampleOffsets[i] - jitterUV * Vector2(0.5f, -0.5f);

				offset *= 1.0f + sharpness * 0.5f;
				sampleWeights[i] = exp(-2.29f * offset.Dot(offset));
				totalWeights += sampleWeights[i];
			}

			for (UINT32 i = 5; i < 9; ++i)
				sampleWeights[i] = 0.0f;
			
			memset(sampleWeightsLowPass, 0, sizeof(sampleWeightsLowPass));
			totalWeightsLowPass = 1.0f;
		}
		else
		{
			static const Vector2 sampleOffsets[] =
			{
				{ -1.0f, -1.0f },
				{  0.0f, -1.0f },
				{  1.0f, -1.0f },
				{ -1.0f,  0.0f },
				{  0.0f,  0.0f },
				{  1.0f,  0.0f },
				{ -1.0f,  1.0f },
				{  0.0f,  1.0f },
				{  1.0f,  1.0f },
			};

			for (UINT32 i = 0; i < 9; ++i)
			{
				// Get rid of jitter introduced by the projection matrix
				Vector2 offset = sampleOffsets[i] - jitterUV;

				offset *= 1.0f + sharpness * 0.5f;
				sampleWeights[i] = exp(-2.29f * offset.Dot(offset));
				totalWeights += sampleWeights[i];

				// Low pass
				offset *= 0.25f;
				sampleWeightsLowPass[i] = exp(-2.29f * offset.Dot(offset));
				totalWeightsLowPass += sampleWeightsLowPass[i];
			}
		}

		for (UINT32 i = 0; i < 9; ++i)
		{
			gTemporalResolveParamDef.gSampleWeights.Set(mTemporalParamBuffer, sampleWeights[i] / totalWeights, i);
			gTemporalResolveParamDef.gSampleWeightsLowpass.Set(mTemporalParamBuffer, sampleWeightsLowPass[i] / totalWeightsLowPass, i);
		}
		
		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::Instance();
		rapi.SetRenderTarget(destination);

		const RendererViewProperties& viewProps = view.GetProperties();
		const Rect2I& viewRect = viewProps.target.viewRect;

		Bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().DrawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
			gRendererUtility().DrawScreenQuad();
	}

	TemporalFilteringMat* TemporalFilteringMat::GetVariation(TemporalFilteringType type, bool velocity, bool msaa)
	{
		switch(type)
		{
		default:
		case TemporalFilteringType::FullScreenAA:
			if(velocity)
			{
				if (msaa)
					return Get(GetVariation<TemporalFilteringType::FullScreenAA, true, true>());

				return Get(GetVariation<TemporalFilteringType::FullScreenAA, true, false>());
			}

			if (msaa)
				return Get(GetVariation<TemporalFilteringType::FullScreenAA, false, true>());

			return Get(GetVariation<TemporalFilteringType::FullScreenAA, false, false>());
		case TemporalFilteringType::SSR:
			if(velocity)
			{
				if (msaa)
					return Get(GetVariation<TemporalFilteringType::SSR, true, true>());

				return Get(GetVariation<TemporalFilteringType::SSR, true, false>());
			}

			if (msaa)
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
		sampDesc.minFilter = FO_POINT;
		sampDesc.magFilter = FO_POINT;
		sampDesc.mipFilter = FO_POINT;
		sampDesc.addressMode.u = TAM_CLAMP;
		sampDesc.addressMode.v = TAM_CLAMP;
		sampDesc.addressMode.w = TAM_CLAMP;

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
		gRendererUtility().DrawScreenQuad();
	}

	MSAACoverageMat::MSAACoverageMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{ }

	void MSAACoverageMat::Execute(const RendererView& view, GBufferTextures gbuffer)
	{
		BS_RENMAT_PROFILE_BLOCK

		mGBufferParams.Bind(gbuffer);

		const Rect2I& viewRect = view.GetProperties().target.viewRect;
		SPtr<GpuParamBlockBuffer> perView = view.GetPerViewBuffer();
		mParams->SetParamBlockBuffer("PerCamera", perView);

		Bind();
		gRendererUtility().DrawScreenQuad(Rect2(0, 0, (float)viewRect.width, (float)viewRect.height));
	}

	MSAACoverageMat* MSAACoverageMat::GetVariation(UINT32 msaaCount)
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

		const Rect2I& viewRect = view.GetProperties().target.viewRect;
		mCoverageTexParam.Set(coverage);

		Bind();
		gRendererUtility().DrawScreenQuad(Rect2(0, 0, (float)viewRect.width, (float)viewRect.height));
	}
}}
