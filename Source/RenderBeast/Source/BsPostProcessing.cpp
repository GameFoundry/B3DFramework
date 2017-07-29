//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPostProcessing.h"
#include "BsRenderTexture.h"
#include "BsGpuResourcePool.h"
#include "BsRendererUtility.h"
#include "BsTextureManager.h"
#include "BsCamera.h"
#include "BsGpuParamsSet.h"
#include "BsRendererView.h"
#include "BsPixelUtil.h"
#include "BsBitwise.h"
#include "BsBuiltinResourcesHelper.h"

namespace bs { namespace ct
{
	DownsampleParamDef gDownsampleParamDef;

	ShaderVariation DownsampleMat::VAR_LowQuality_NoMSAA = ShaderVariation({
		ShaderVariation::Param("QUALITY", 0),
		ShaderVariation::Param("MSAA", 0)
	});

	ShaderVariation DownsampleMat::VAR_LowQuality_MSAA = ShaderVariation({
		ShaderVariation::Param("QUALITY", 0),
		ShaderVariation::Param("MSAA", 1)
	});

	ShaderVariation DownsampleMat::VAR_HighQuality_NoMSAA = ShaderVariation({
		ShaderVariation::Param("QUALITY", 1),
		ShaderVariation::Param("MSAA", 0)
	});

	ShaderVariation DownsampleMat::VAR_HighQuality_MSAA = ShaderVariation({
		ShaderVariation::Param("QUALITY", 1),
		ShaderVariation::Param("MSAA", 1)
	});

	DownsampleMat::DownsampleMat()
	{
		mParamBuffer = gDownsampleParamDef.createBuffer();

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		if(gpuParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void DownsampleMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_LowQuality_NoMSAA);
		variations.add(VAR_LowQuality_MSAA);
		variations.add(VAR_HighQuality_NoMSAA);
		variations.add(VAR_HighQuality_MSAA);
	}

	void DownsampleMat::execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
	{
		// Set parameters
		mInputTexture.set(input);

		const TextureProperties& rtProps = input->getProperties();

		bool MSAA = mVariation.getInt("MSAA") > 0;
		if(MSAA)
		{
			gDownsampleParamDef.gOffsets.set(mParamBuffer, Vector2(-1.0f, -1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, Vector2(1.0f, -1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, Vector2(-1.0f, 1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, Vector2(1.0f, 1.0f));
		}
		else
		{
			Vector2 invTextureSize(1.0f / rtProps.getWidth(), 1.0f / rtProps.getHeight());

			gDownsampleParamDef.gOffsets.set(mParamBuffer, invTextureSize * Vector2(-1.0f, -1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, invTextureSize * Vector2(1.0f, -1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, invTextureSize * Vector2(-1.0f, 1.0f));
			gDownsampleParamDef.gOffsets.set(mParamBuffer, invTextureSize * Vector2(1.0f, 1.0f));
		}

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		if (MSAA)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)rtProps.getWidth(), (float)rtProps.getHeight()));
		else
			gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC DownsampleMat::getOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->getProperties();
		
		UINT32 width = std::max(1, Math::ceilToInt(rtProps.getWidth() * 0.5f));
		UINT32 height = std::max(1, Math::ceilToInt(rtProps.getHeight() * 0.5f));

		return POOLED_RENDER_TEXTURE_DESC::create2D(rtProps.getFormat(), width, height, TU_RENDERTARGET);
	}

	DownsampleMat* DownsampleMat::getVariation(UINT32 quality, bool msaa)
	{
		if(quality == 0)
		{
			if (msaa)
				return get(VAR_LowQuality_MSAA);
			else
				return get(VAR_LowQuality_NoMSAA);
		}
		else
		{
			if (msaa)
				return get(VAR_HighQuality_MSAA);
			else
				return get(VAR_HighQuality_NoMSAA);
		}
	}

	EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

	EyeAdaptHistogramMat::EyeAdaptHistogramMat()
	{
		mParamBuffer = gEyeAdaptHistogramParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gSceneColorTex", mSceneColor);
		params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
	}

	void EyeAdaptHistogramMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("THREADGROUP_SIZE_X", THREAD_GROUP_SIZE_X),
			ShaderVariation::Param("THREADGROUP_SIZE_Y", THREAD_GROUP_SIZE_Y),
			ShaderVariation::Param("LOOP_COUNT_X", LOOP_COUNT_X),
			ShaderVariation::Param("LOOP_COUNT_Y", LOOP_COUNT_Y),
		});

		variations.add(variation);
	}

	void EyeAdaptHistogramMat::execute(const SPtr<Texture>& input, const SPtr<Texture>& output, 
		const AutoExposureSettings& settings)
	{
		// Set parameters
		mSceneColor.set(input);

		const TextureProperties& props = input->getProperties();
		int offsetAndSize[4] = { 0, 0, (INT32)props.getWidth(), (INT32)props.getHeight() };

		gEyeAdaptHistogramParamDef.gHistogramParams.set(mParamBuffer, getHistogramScaleOffset(settings));
		gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.set(mParamBuffer, Vector4I(offsetAndSize));

		Vector2I threadGroupCount = getThreadGroupCount(input);
		gEyeAdaptHistogramParamDef.gThreadGroupCount.set(mParamBuffer, threadGroupCount);

		// Dispatch
		mOutputTex.set(output);

		RenderAPI& rapi = RenderAPI::instance();
		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(threadGroupCount.x, threadGroupCount.y);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramMat::getOutputDesc(const SPtr<Texture>& target)
	{
		Vector2I threadGroupCount = getThreadGroupCount(target);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, HISTOGRAM_NUM_TEXELS, numHistograms,
			TU_LOADSTORE);
	}

	Vector2I EyeAdaptHistogramMat::getThreadGroupCount(const SPtr<Texture>& target)
	{
		const UINT32 texelsPerThreadGroupX = THREAD_GROUP_SIZE_X * LOOP_COUNT_X;
		const UINT32 texelsPerThreadGroupY = THREAD_GROUP_SIZE_Y * LOOP_COUNT_Y;

		const TextureProperties& props = target->getProperties();
	
		Vector2I threadGroupCount;
		threadGroupCount.x = ((INT32)props.getWidth() + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
		threadGroupCount.y = ((INT32)props.getHeight() + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

		return threadGroupCount;
	}

	Vector2 EyeAdaptHistogramMat::getHistogramScaleOffset(const AutoExposureSettings& settings)
	{
		float diff = settings.histogramLog2Max - settings.histogramLog2Min;
		float scale = 1.0f / diff;
		float offset = -settings.histogramLog2Min * scale;

		return Vector2(scale, offset);
	}

	EyeAdaptHistogramReduceParamDef gEyeAdaptHistogramReduceParamDef;

	EyeAdaptHistogramReduceMat::EyeAdaptHistogramReduceMat()
	{
		mParamBuffer = gEyeAdaptHistogramReduceParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mHistogramTex);
		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
	}

	void EyeAdaptHistogramReduceMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void EyeAdaptHistogramReduceMat::execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram,
		const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output)
	{
		// Set parameters
		mHistogramTex.set(histogram);

		SPtr<Texture> eyeAdaptationTex;
		if (prevFrame == nullptr) // Could be that this is the first run
			eyeAdaptationTex = Texture::WHITE;
		else
			eyeAdaptationTex = prevFrame;

		mEyeAdaptationTex.set(eyeAdaptationTex);

		Vector2I threadGroupCount = EyeAdaptHistogramMat::getThreadGroupCount(sceneColor);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		gEyeAdaptHistogramReduceParamDef.gThreadGroupCount.set(mParamBuffer, numHistograms);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2.0f);
		gRendererUtility().drawScreenQuad(drawUV);

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramReduceMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2,
			TU_RENDERTARGET);
	}

	EyeAdaptationParamDef gEyeAdaptationParamDef;

	EyeAdaptationMat::EyeAdaptationMat()
	{
		mParamBuffer = gEyeAdaptationParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mReducedHistogramTex);
	}

	void EyeAdaptationMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_X),
			ShaderVariation::Param("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_Y)
		});

		variations.add(variation);
	}

	void EyeAdaptationMat::execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, 
		float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		// Set parameters
		mReducedHistogramTex.set(reducedHistogram);

		Vector2 histogramScaleAndOffset = EyeAdaptHistogramMat::getHistogramScaleOffset(settings);

		Vector4 eyeAdaptationParams[3];
		eyeAdaptationParams[0].x = histogramScaleAndOffset.x;
		eyeAdaptationParams[0].y = histogramScaleAndOffset.y;

		float histogramPctHigh = Math::clamp01(settings.histogramPctHigh);

		eyeAdaptationParams[0].z = std::min(Math::clamp01(settings.histogramPctLow), histogramPctHigh);
		eyeAdaptationParams[0].w = histogramPctHigh;

		eyeAdaptationParams[1].x = std::min(settings.minEyeAdaptation, settings.maxEyeAdaptation);
		eyeAdaptationParams[1].y = settings.maxEyeAdaptation;

		eyeAdaptationParams[1].z = settings.eyeAdaptationSpeedUp;
		eyeAdaptationParams[1].w = settings.eyeAdaptationSpeedDown;

		eyeAdaptationParams[2].x = Math::pow(2.0f, exposureScale);
		eyeAdaptationParams[2].y = frameDelta;

		eyeAdaptationParams[2].z = 0.0f; // Unused
		eyeAdaptationParams[2].w = 0.0f; // Unused

		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[0], 0);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[1], 1);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[2], 2);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptationMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT32_R, 1, 1, TU_RENDERTARGET);
	}

	CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;
	WhiteBalanceParamDef gWhiteBalanceParamDef;

	CreateTonemapLUTMat::CreateTonemapLUTMat()
	{
		mParamBuffer = gCreateTonemapLUTParamDef.createBuffer();
		mWhiteBalanceParamBuffer = gWhiteBalanceParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->setParamBlockBuffer("WhiteBalanceInput", mWhiteBalanceParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
	}

	void CreateTonemapLUTMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("LUT_SIZE", LUT_SIZE)
		});

		variations.add(variation);
	}

	void CreateTonemapLUTMat::execute(const SPtr<Texture>& output, const RenderSettings& settings)
	{
		// Set parameters
		gCreateTonemapLUTParamDef.gGammaAdjustment.set(mParamBuffer, 2.2f / settings.gamma);

		// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
		// gamma when running on Mac (value 2)
		gCreateTonemapLUTParamDef.gGammaCorrectionType.set(mParamBuffer, 0);

		Vector4 tonemapParams[2];
		tonemapParams[0].x = settings.tonemapping.filmicCurveShoulderStrength;
		tonemapParams[0].y = settings.tonemapping.filmicCurveLinearStrength;
		tonemapParams[0].z = settings.tonemapping.filmicCurveLinearAngle;
		tonemapParams[0].w = settings.tonemapping.filmicCurveToeStrength;

		tonemapParams[1].x = settings.tonemapping.filmicCurveToeNumerator;
		tonemapParams[1].y = settings.tonemapping.filmicCurveToeDenominator;
		tonemapParams[1].z = settings.tonemapping.filmicCurveLinearWhitePoint;
		tonemapParams[1].w = 0.0f; // Unused

		gCreateTonemapLUTParamDef.gTonemapParams.set(mParamBuffer, tonemapParams[0], 0);
		gCreateTonemapLUTParamDef.gTonemapParams.set(mParamBuffer, tonemapParams[1], 1);

		// Set color grading params
		gCreateTonemapLUTParamDef.gSaturation.set(mParamBuffer, settings.colorGrading.saturation);
		gCreateTonemapLUTParamDef.gContrast.set(mParamBuffer, settings.colorGrading.contrast);
		gCreateTonemapLUTParamDef.gGain.set(mParamBuffer, settings.colorGrading.gain);
		gCreateTonemapLUTParamDef.gOffset.set(mParamBuffer, settings.colorGrading.offset);

		// Set white balance params
		gWhiteBalanceParamDef.gWhiteTemp.set(mWhiteBalanceParamBuffer, settings.whiteBalance.temperature);
		gWhiteBalanceParamDef.gWhiteOffset.set(mWhiteBalanceParamBuffer, settings.whiteBalance.tint);

		// Dispatch
		mOutputTex.set(output);

		RenderAPI& rapi = RenderAPI::instance();
		
		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(LUT_SIZE / 8, LUT_SIZE / 8, LUT_SIZE);
	}

	POOLED_RENDER_TEXTURE_DESC CreateTonemapLUTMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create3D(PF_R8G8B8A8, LUT_SIZE, LUT_SIZE, LUT_SIZE, TU_LOADSTORE);
	}

	TonemappingParamDef gTonemappingParamDef;

	ShaderVariation TonemappingMat::VAR_Gamma_AutoExposure_MSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", true),
		ShaderVariation::Param("AUTO_EXPOSURE", true),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_Gamma_AutoExposure_NoMSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", true),
		ShaderVariation::Param("AUTO_EXPOSURE", true),
		ShaderVariation::Param("MSAA", false),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_Gamma_NoAutoExposure_MSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", true),
		ShaderVariation::Param("AUTO_EXPOSURE", false),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_Gamma_NoAutoExposure_NoMSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", true),
		ShaderVariation::Param("AUTO_EXPOSURE", false),
		ShaderVariation::Param("MSAA", false),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_NoGamma_AutoExposure_MSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", false),
		ShaderVariation::Param("AUTO_EXPOSURE", true),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_NoGamma_AutoExposure_NoMSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", false),
		ShaderVariation::Param("AUTO_EXPOSURE", true),
		ShaderVariation::Param("MSAA", false),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_NoGamma_NoAutoExposure_MSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", false),
		ShaderVariation::Param("AUTO_EXPOSURE", false),
		ShaderVariation::Param("MSAA", true),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	ShaderVariation TonemappingMat::VAR_NoGamma_NoAutoExposure_NoMSAA = ShaderVariation({
		ShaderVariation::Param("GAMMA_ONLY", false),
		ShaderVariation::Param("AUTO_EXPOSURE", false),
		ShaderVariation::Param("MSAA", false),
		ShaderVariation::Param("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE),
	});

	TonemappingMat::TonemappingMat()
	{
		mParamBuffer = gTonemappingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);

		if(!mVariation.getBool("GAMMA_ONLY"))
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorLUT", mColorLUT);
	}

	void TonemappingMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_Gamma_AutoExposure_MSAA);
		variations.add(VAR_Gamma_AutoExposure_NoMSAA);
		variations.add(VAR_Gamma_NoAutoExposure_MSAA);
		variations.add(VAR_Gamma_NoAutoExposure_NoMSAA);
		variations.add(VAR_NoGamma_AutoExposure_MSAA);
		variations.add(VAR_NoGamma_AutoExposure_NoMSAA);
		variations.add(VAR_NoGamma_NoAutoExposure_MSAA);
		variations.add(VAR_NoGamma_NoAutoExposure_NoMSAA);
	}

	void TonemappingMat::execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, 
		const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output, const RenderSettings& settings)
	{
		const TextureProperties& texProps = sceneColor->getProperties();

		gTonemappingParamDef.gRawGamma.set(mParamBuffer, 1.0f / settings.gamma);
		gTonemappingParamDef.gManualExposureScale.set(mParamBuffer, Math::pow(2.0f, settings.exposureScale));
		gTonemappingParamDef.gNumSamples.set(mParamBuffer, texProps.getNumSamples());

		// Set parameters
		mInputTex.set(sceneColor);
		mColorLUT.set(colorLUT);
		mEyeAdaptationTex.set(eyeAdaptation);

		// Render
		RenderAPI& rapi = RenderAPI::instance();

		rapi.setRenderTarget(output);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		if (mVariation.getBool("MSAA"))
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)texProps.getWidth(), (float)texProps.getHeight()));
		else
			gRendererUtility().drawScreenQuad();
	}

	TonemappingMat* TonemappingMat::getVariation(bool gammaOnly, bool autoExposure, bool MSAA)
	{
		if (gammaOnly)
		{
			if (autoExposure)
			{
				if (MSAA)
					return get(VAR_Gamma_AutoExposure_MSAA);
				else
					return get(VAR_Gamma_AutoExposure_NoMSAA);
			}
			else
			{
				if (MSAA)
					return get(VAR_Gamma_NoAutoExposure_MSAA);
				else
					return get(VAR_Gamma_NoAutoExposure_NoMSAA);
			}
		}
		else
		{
			if (autoExposure)
			{
				if (MSAA)
					return get(VAR_NoGamma_AutoExposure_MSAA);
				else
					return get(VAR_NoGamma_AutoExposure_NoMSAA);
			}
			else
			{
				if (MSAA)
					return get(VAR_NoGamma_NoAutoExposure_MSAA);
				else
					return get(VAR_NoGamma_NoAutoExposure_NoMSAA);
			}
		}
	}

	GaussianBlurParamDef gGaussianBlurParamDef;

	GaussianBlurMat::GaussianBlurMat()
	{
		mParamBuffer = gGaussianBlurParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void GaussianBlurMat::_initVariations(ShaderVariations& variations)
	{
		ShaderVariation variation({
			ShaderVariation::Param("MAX_NUM_SAMPLES", MAX_BLUR_SAMPLES)
		});

		variations.add(variation);
	}

	void GaussianBlurMat::execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination)
	{
		const TextureProperties& srcProps = source->getProperties();
		const RenderTextureProperties& dstProps = destination->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());

		std::array<float, MAX_BLUR_SAMPLES> sampleOffsets;
		std::array<float, MAX_BLUR_SAMPLES> sampleWeights;

		POOLED_RENDER_TEXTURE_DESC tempTextureDesc = POOLED_RENDER_TEXTURE_DESC::create2D(srcProps.getFormat(), 
			dstProps.getWidth(), dstProps.getHeight(), TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = GpuResourcePool::instance().get(tempTextureDesc);

		auto updateParamBuffer = [&](Direction direction)
		{
			float kernelRadius = calcKernelRadius(source, filterSize, direction);
			UINT32 numSamples = calcStdDistribution(kernelRadius, sampleWeights, sampleOffsets);

			for(UINT32 i = 0; i < (numSamples + 3) / 4; ++i)
			{
				UINT32 remainder = std::min(4U, numSamples - i * 4);

				Vector4 weights;
				for (UINT32 j = 0; j < remainder; ++j)
					weights[j] = sampleWeights[i * 4 + j];

				gGaussianBlurParamDef.gSampleWeights.set(mParamBuffer, weights, i);
			}

			UINT32 axis0 = direction == DirHorizontal ? 0 : 1;
			UINT32 axis1 = (axis0 + 1) % 2;

			for(UINT32 i = 0; i < (numSamples + 1) / 2; ++i)
			{
				UINT32 remainder = std::min(2U, numSamples - i * 2);

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

				gGaussianBlurParamDef.gSampleOffsets.set(mParamBuffer, offset, i);
			}

			gGaussianBlurParamDef.gNumSamples.set(mParamBuffer, numSamples);
		};

		// Horizontal pass
		{
			updateParamBuffer(DirHorizontal);
			mInputTexture.set(source);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(tempTexture->renderTexture);

			gRendererUtility().setPass(mMaterial);
			gRendererUtility().setPassParams(mParamsSet);
			gRendererUtility().drawScreenQuad();
		}

		// Vertical pass
		{
			updateParamBuffer(DirVertical);
			mInputTexture.set(tempTexture->texture);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(destination);

			gRendererUtility().setPass(mMaterial);
			gRendererUtility().setPassParams(mParamsSet);
			gRendererUtility().drawScreenQuad();
		}

		GpuResourcePool::instance().release(tempTexture);
	}

	UINT32 GaussianBlurMat::calcStdDistribution(float filterRadius, std::array<float, MAX_BLUR_SAMPLES>& weights, 
		std::array<float, MAX_BLUR_SAMPLES>& offsets)
	{
		filterRadius = Math::clamp(filterRadius, 0.00001f, (float)(MAX_BLUR_SAMPLES - 1));
		INT32 intFilterRadius = std::min(Math::ceilToInt(filterRadius), MAX_BLUR_SAMPLES - 1);

		auto normalDistribution = [](int i, float scale)
		{
			float samplePos = fabs((float)i) * scale;
			return exp(samplePos * samplePos);
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
			float o = w2/w3; // Relative to first sample

			weights[numSamples] = w3;
			offsets[numSamples] = o;

			numSamples++;
			totalWeight += w3;
		}

		// Special case for last weight, as it doesn't have a matching pair
		float w = normalDistribution(intFilterRadius, scale);
		weights[numSamples] = w;
		offsets[numSamples] = 0.0f;

		numSamples++;
		totalWeight += w;

		// Normalize weights
		float invTotalWeight = 1.0f / totalWeight;
		for(UINT32 i = 0; i < numSamples; i++)
			weights[i] *= invTotalWeight;

		return numSamples;
	}

	float GaussianBlurMat::calcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir)
	{
		scale = Math::clamp01(scale);

		UINT32 length;
		if (filterDir == DirHorizontal)
			length = source->getProperties().getWidth();
		else
			length = source->getProperties().getHeight();

		// Divide by two because we need the radius
		return std::min(length * scale / 2, (float)MAX_BLUR_SAMPLES - 1);
	}

	GaussianDOFParamDef gGaussianDOFParamDef;

	ShaderVariation GaussianDOFSeparateMat::VAR_Near_Far = ShaderVariation({
		ShaderVariation::Param("NEAR", true),
		ShaderVariation::Param("FAR", true),
		ShaderVariation::Param("NEAR_AND_FAR", true)
	});

	ShaderVariation GaussianDOFSeparateMat::VAR_NoNear_Far = ShaderVariation({
		ShaderVariation::Param("NEAR", false),
		ShaderVariation::Param("FAR", true),
		ShaderVariation::Param("NEAR_AND_FAR", false)
	});

	ShaderVariation GaussianDOFSeparateMat::VAR_Near_NoFar = ShaderVariation({
		ShaderVariation::Param("NEAR", true),
		ShaderVariation::Param("FAR", false),
		ShaderVariation::Param("NEAR_AND_FAR", false)
	});

	GaussianDOFSeparateMat::GaussianDOFSeparateMat()
	{
		mParamBuffer = gGaussianDOFParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorTex", mColorTexture);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		GpuParamSampState colorSampState;
		mParamsSet->getGpuParams()->getSamplerStateParam(GPT_FRAGMENT_PROGRAM, "gColorSamp", colorSampState);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::create(desc);
		colorSampState.set(samplerState);
	}

	void GaussianDOFSeparateMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_Near_Far);
		variations.add(VAR_Near_NoFar);
		variations.add(VAR_NoNear_Far);
	}

	void GaussianDOFSeparateMat::execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, 
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		const TextureProperties& srcProps = color->getProperties();

		UINT32 outputWidth = std::max(1U, srcProps.getWidth() / 2);
		UINT32 outputHeight = std::max(1U, srcProps.getHeight() / 2);

		POOLED_RENDER_TEXTURE_DESC outputTexDesc = POOLED_RENDER_TEXTURE_DESC::create2D(srcProps.getFormat(), 
			outputWidth, outputHeight, TU_RENDERTARGET);
		mOutput0 = GpuResourcePool::instance().get(outputTexDesc);

		bool near = mVariation.getBool("NEAR");
		bool far = mVariation.getBool("FAR");

		SPtr<RenderTexture> rt;
		if (near && far)
		{
			mOutput1 = GpuResourcePool::instance().get(outputTexDesc);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.colorSurfaces[0].texture = mOutput0->texture;
			rtDesc.colorSurfaces[1].texture = mOutput1->texture;

			rt = RenderTexture::create(rtDesc);
		}
		else
			rt = mOutput0->renderTexture;

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());

		gGaussianDOFParamDef.gHalfPixelOffset.set(mParamBuffer, invTexSize * 0.5f);
		gGaussianDOFParamDef.gNearBlurPlane.set(mParamBuffer, settings.focalDistance - settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gFarBlurPlane.set(mParamBuffer, settings.focalDistance + settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gInvNearBlurRange.set(mParamBuffer, 1.0f / settings.nearTransitionRange);
		gGaussianDOFParamDef.gInvFarBlurRange.set(mParamBuffer, 1.0f / settings.farTransitionRange);

		mColorTexture.set(color);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(rt);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SPtr<PooledRenderTexture> GaussianDOFSeparateMat::getOutput(UINT32 idx)
	{
		if (idx == 0)
			return mOutput0;
		else if (idx == 1)
			return mOutput1;

		return nullptr;
	}

	void GaussianDOFSeparateMat::release()
	{
		if (mOutput0 != nullptr)
			GpuResourcePool::instance().release(mOutput0);

		if (mOutput1 != nullptr)
			GpuResourcePool::instance().release(mOutput1);
	}

	GaussianDOFSeparateMat* GaussianDOFSeparateMat::getVariation(bool near, bool far)
	{
		if (near)
		{
			if (far)
				return get(VAR_Near_Far);
			else
				return get(VAR_Near_NoFar);
		}
		else
			return get(VAR_NoNear_Far);
	}

	ShaderVariation GaussianDOFCombineMat::VAR_Near_Far = ShaderVariation({
		ShaderVariation::Param("NEAR", true),
		ShaderVariation::Param("FAR", true),
		ShaderVariation::Param("NEAR_AND_FAR", true)
	});

	ShaderVariation GaussianDOFCombineMat::VAR_NoNear_Far = ShaderVariation({
		ShaderVariation::Param("NEAR", false),
		ShaderVariation::Param("FAR", true),
		ShaderVariation::Param("NEAR_AND_FAR", false)
	});

	ShaderVariation GaussianDOFCombineMat::VAR_Near_NoFar = ShaderVariation({
		ShaderVariation::Param("NEAR", true),
		ShaderVariation::Param("FAR", false),
		ShaderVariation::Param("NEAR_AND_FAR", false)
	});

	GaussianDOFCombineMat::GaussianDOFCombineMat()
	{
		mParamBuffer = gGaussianDOFParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFocusedTex", mFocusedTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		if(gpuParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gNearTex"))
			gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNearTex", mNearTexture);

		if(gpuParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gFarTex"))
			gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFarTex", mFarTexture);
	}

	void GaussianDOFCombineMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_Near_Far);
		variations.add(VAR_Near_NoFar);
		variations.add(VAR_NoNear_Far);
	}

	void GaussianDOFCombineMat::execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, 
		const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output,
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		const TextureProperties& srcProps = focused->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());

		gGaussianDOFParamDef.gHalfPixelOffset.set(mParamBuffer, invTexSize * 0.5f);
		gGaussianDOFParamDef.gNearBlurPlane.set(mParamBuffer, settings.focalDistance - settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gFarBlurPlane.set(mParamBuffer, settings.focalDistance + settings.focalRange * 0.5f);
		gGaussianDOFParamDef.gInvNearBlurRange.set(mParamBuffer, 1.0f / settings.nearTransitionRange);
		gGaussianDOFParamDef.gInvFarBlurRange.set(mParamBuffer, 1.0f / settings.farTransitionRange);

		mFocusedTexture.set(focused);
		mNearTexture.set(near);
		mFarTexture.set(far);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	GaussianDOFCombineMat* GaussianDOFCombineMat::getVariation(bool near, bool far)
	{
		if (near)
		{
			if (far)
				return get(VAR_Near_Far);
			else
				return get(VAR_Near_NoFar);
		}
		else
			return get(VAR_NoNear_Far);
	}

	BuildHiZMat::BuildHiZMat()
	{
		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mInputTexture);
	}

	void BuildHiZMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void BuildHiZMat::execute(const SPtr<Texture>& source, UINT32 srcMip, const Rect2& srcRect, const Rect2& dstRect,
		const SPtr<RenderTexture>& output)
	{
		RenderAPI& rapi = RenderAPI::instance();
		const TextureProperties& srcProps = source->getProperties();

		mInputTexture.set(source, TextureSurface(srcMip));
		rapi.setRenderTarget(output);
		rapi.setViewport(dstRect);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad(srcRect);

		rapi.setViewport(Rect2(0, 0, 1, 1));
	}

	FXAAParamDef gFXAAParamDef;

	FXAAMat::FXAAMat()
	{
		mParamBuffer = gFXAAParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void FXAAMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void FXAAMat::execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination)
	{
		const TextureProperties& srcProps = source->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());
		gFXAAParamDef.gInvTexSize.set(mParamBuffer, invTexSize);

		mInputTexture.set(source);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SSAOParamDef gSSAOParamDef;

#define VARIATION(QUALITY) \
		ShaderVariation SSAOMat::VAR_Upsample_Final_Quality##QUALITY = ShaderVariation({		\
			ShaderVariation::Param("MIX_WITH_UPSAMPLED", true),									\
			ShaderVariation::Param("FINAL_AO", true),											\
			ShaderVariation::Param("QUALITY", QUALITY)											\
		});																						\
		ShaderVariation SSAOMat::VAR_Upsample_NoFinal_Quality##QUALITY = ShaderVariation({		\
			ShaderVariation::Param("MIX_WITH_UPSAMPLED", true),									\
			ShaderVariation::Param("FINAL_AO", false),											\
			ShaderVariation::Param("QUALITY", QUALITY)											\
		});																						\
		ShaderVariation SSAOMat::VAR_NoUpsample_Final_Quality##QUALITY = ShaderVariation({		\
			ShaderVariation::Param("MIX_WITH_UPSAMPLED", false),								\
			ShaderVariation::Param("FINAL_AO", true),											\
			ShaderVariation::Param("QUALITY", QUALITY)											\
		});																						\
		ShaderVariation SSAOMat::VAR_NoUpsample_NoFinal_Quality##QUALITY = ShaderVariation({	\
			ShaderVariation::Param("MIX_WITH_UPSAMPLED", false),								\
			ShaderVariation::Param("FINAL_AO", false),											\
			ShaderVariation::Param("QUALITY", QUALITY)											\
		});

		VARIATION(0)
		VARIATION(1)
		VARIATION(2)
		VARIATION(3)
		VARIATION(4)

#undef VARIATION

	SSAOMat::SSAOMat()
	{
		mParamBuffer = gSSAOParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDownsampledAO", mDownsampledAOTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSetupAO", mSetupAOTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gRandomTex", mRandomTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);

		SAMPLER_STATE_DESC randomSampDesc;
		randomSampDesc.minFilter = FO_POINT;
		randomSampDesc.magFilter = FO_POINT;
		randomSampDesc.mipFilter = FO_POINT;
		randomSampDesc.addressMode.u = TAM_WRAP;
		randomSampDesc.addressMode.v = TAM_WRAP;
		randomSampDesc.addressMode.w = TAM_WRAP;

		SPtr<SamplerState> randomSampState = SamplerState::create(randomSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gRandomSamp", randomSampState);
	}

	void SSAOMat::_initVariations(ShaderVariations& variations)
	{
#define VARIATION(QUALITY) \
		variations.add(VAR_Upsample_Final_Quality##QUALITY);		\
		variations.add(VAR_Upsample_NoFinal_Quality##QUALITY);		\
		variations.add(VAR_NoUpsample_Final_Quality##QUALITY);		\
		variations.add(VAR_NoUpsample_NoFinal_Quality##QUALITY);	\

		VARIATION(0)
		VARIATION(1)
		VARIATION(2)
		VARIATION(3)
		VARIATION(4)

#undef VARIATION
	}

	void SSAOMat::execute(const RendererView& view, const SSAOTextureInputs& textures, 
		const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
	{
		// Scale that can be used to adjust how quickly does AO radius increase with downsampled AO. This yields a very
		// small AO radius at highest level, and very large radius at lowest level
		static const float DOWNSAMPLE_SCALE = 4.0f;

		const RendererViewProperties& viewProps = view.getProperties();
		const RenderTargetProperties& rtProps = destination->getProperties();

		Vector2 tanHalfFOV;
		tanHalfFOV.x = 1.0f / viewProps.projTransform[0][0];
		tanHalfFOV.y = 1.0f / viewProps.projTransform[1][1];

		float cotHalfFOV = viewProps.projTransform[0][0];

		// Downsampled AO uses a larger AO radius (in higher resolutions this would cause too much cache trashing). This
		// means if only full res AO is used, then only AO from nearby geometry will be calculated.
		float viewScale = viewProps.viewRect.width / (float)rtProps.getWidth();

		// Ramp up the radius exponentially. c^log2(x) function chosen arbitrarily, as it ramps up the radius in a nice way
		float scale = pow(DOWNSAMPLE_SCALE, Math::log2(viewScale)); 

		// Determine maximum radius scale (division by 4 because we don't downsample more than quarter-size)
		float maxScale = pow(DOWNSAMPLE_SCALE, Math::log2(4.0f));

		// Normalize the scale in [0, 1] range
		scale /= maxScale;

		float radius = settings.radius * scale;

		// Factors used for scaling the AO contribution with range
		Vector2 fadeMultiplyAdd;
		fadeMultiplyAdd.x = 1.0f / settings.fadeRange;
		fadeMultiplyAdd.y = -settings.fadeDistance / settings.fadeRange;

		gSSAOParamDef.gSampleRadius.set(mParamBuffer, radius);
		gSSAOParamDef.gCotHalfFOV.set(mParamBuffer, cotHalfFOV);
		gSSAOParamDef.gTanHalfFOV.set(mParamBuffer, tanHalfFOV);
		gSSAOParamDef.gWorldSpaceRadiusMask.set(mParamBuffer, 1.0f);
		gSSAOParamDef.gBias.set(mParamBuffer, (settings.bias * viewScale) / 1000.0f);
		gSSAOParamDef.gFadeMultiplyAdd.set(mParamBuffer, fadeMultiplyAdd);
		gSSAOParamDef.gPower.set(mParamBuffer, settings.power);
		gSSAOParamDef.gIntensity.set(mParamBuffer, settings.intensity);
		
		bool upsample = mVariation.getBool("MIX_WITH_UPSAMPLED");
		if(upsample)
		{
			const TextureProperties& props = textures.aoDownsampled->getProperties();

			Vector2 downsampledPixelSize;
			downsampledPixelSize.x = 1.0f / props.getWidth();
			downsampledPixelSize.y = 1.0f / props.getHeight();

			gSSAOParamDef.gDownsampledPixelSize.set(mParamBuffer, downsampledPixelSize);
		}

		// Generate a scale which we need to use in order to achieve tiling
		const TextureProperties& rndProps = textures.randomRotations->getProperties();
		UINT32 rndWidth = rndProps.getWidth();
		UINT32 rndHeight = rndProps.getHeight();

		//// Multiple of random texture size, rounded up
		UINT32 scaleWidth = (rtProps.getWidth() + rndWidth - 1) / rndWidth;
		UINT32 scaleHeight = (rtProps.getHeight() + rndHeight - 1) / rndHeight;

		Vector2 randomTileScale((float)scaleWidth, (float)scaleHeight);
		gSSAOParamDef.gRandomTileScale.set(mParamBuffer, randomTileScale);

		mSetupAOTexture.set(textures.aoSetup);

		bool finalPass = mVariation.getBool("FINAL_AO");
		if (finalPass)
		{
			mDepthTexture.set(textures.sceneDepth);
			mNormalsTexture.set(textures.sceneNormals);
		}

		if (upsample)
			mDownsampledAOTexture.set(textures.aoDownsampled);
		
		mRandomTexture.set(textures.randomRotations);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SSAOMat* SSAOMat::getVariation(bool upsample, bool finalPass, int quality)
	{
#define PICK_MATERIAL(QUALITY)															\
		if(upsample)																	\
			if(finalPass)																\
				return get(VAR_Upsample_Final_Quality##QUALITY);						\
			else																		\
				return get(VAR_Upsample_NoFinal_Quality##QUALITY);						\
		else																			\
			if(finalPass)																\
				return get(VAR_NoUpsample_Final_Quality##QUALITY);						\
			else																		\
				return get(VAR_NoUpsample_NoFinal_Quality##QUALITY);					\

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
		mParamBuffer = gSSAODownsampleParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_LINEAR;
		inputSampDesc.magFilter = FO_LINEAR;
		inputSampDesc.mipFilter = FO_LINEAR;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
	}

	void SSAODownsampleMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void SSAODownsampleMat::execute(const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals, 
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		const RenderTargetProperties& rtProps = destination->getProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / rtProps.getWidth();
		pixelSize.y = 1.0f / rtProps.getHeight();

		float scale = viewProps.viewRect.width / (float)rtProps.getWidth();

		gSSAODownsampleParamDef.gPixelSize.set(mParamBuffer, pixelSize);
		gSSAODownsampleParamDef.gInvDepthThreshold.set(mParamBuffer, (1.0f / depthRange) / scale);

		mDepthTexture.set(depth);
		mNormalsTexture.set(normals);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SSAOBlurParamDef gSSAOBlurParamDef;

	ShaderVariation SSAOBlurMat::VAR_Vertical = ShaderVariation({
		ShaderVariation::Param("DIR_HORZ", false)
	});

	ShaderVariation SSAOBlurMat::VAR_Horizontal = ShaderVariation({
		ShaderVariation::Param("DIR_HORZ", true)
	});

	SSAOBlurMat::SSAOBlurMat()
	{
		mParamBuffer = gSSAOBlurParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mAOTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
	}

	void SSAOBlurMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_Horizontal);
		variations.add(VAR_Vertical);
	}

	void SSAOBlurMat::execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth, 
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		const TextureProperties& texProps = ao->getProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / texProps.getWidth();
		pixelSize.y = 1.0f / texProps.getHeight();

		Vector2 pixelOffset(BsZero);
		if (mVariation.getBool("DIR_HORZ"))
			pixelOffset.x = pixelSize.x;
		else
			pixelOffset.y = pixelSize.y;

		float scale = viewProps.viewRect.width / (float)texProps.getWidth();

		gSSAOBlurParamDef.gPixelSize.set(mParamBuffer, pixelSize);
		gSSAOBlurParamDef.gPixelOffset.set(mParamBuffer, pixelOffset);
		gSSAOBlurParamDef.gInvDepthThreshold.set(mParamBuffer, (1.0f / depthRange) / scale);

		mAOTexture.set(ao);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SSAOBlurMat* SSAOBlurMat::getVariation(bool horizontal)
	{
		if (horizontal)
			return get(VAR_Horizontal);
		
		return get(VAR_Vertical);
	}

	SSRStencilParamDef gSSRStencilParamDef;

	SSRStencilMat::SSRStencilMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		mParamBuffer = gSSRStencilParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
	}

	void SSRStencilMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void SSRStencilMat::execute(const RendererView& view, GBufferInput gbuffer, 
		const ScreenSpaceReflectionsSettings& settings)
	{
		mGBufferParams.bind(gbuffer);

		Vector2 roughnessScaleBias = SSRTraceMat::calcRoughnessFadeScaleBias(settings.maxRoughness);
		gSSRStencilParamDef.gRoughnessScaleBias.set(mParamBuffer, roughnessScaleBias);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	SSRTraceParamDef gSSRTraceParamDef;

	SSRTraceMat::SSRTraceMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		mParamBuffer = gSSRTraceParamDef.createBuffer();

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHiZ", mHiZTexture);

		if(gpuParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			gpuParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);
	}

	void SSRTraceMat::_initVariations(ShaderVariations& variations)
	{
		// Do nothing
	}

	void SSRTraceMat::execute(const RendererView& view, GBufferInput gbuffer, const SPtr<Texture>& sceneColor, 
			const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings, 
			const SPtr<RenderTarget>& destination)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		const TextureProperties& hiZProps = hiZ->getProperties();

		mGBufferParams.bind(gbuffer);
		mSceneColorTexture.set(sceneColor);
		mHiZTexture.set(hiZ);
		
		Rect2I viewRect = viewProps.viewRect;

		// Maps from NDC to UV [0, 1]
		Vector4 ndcToHiZUV;
		ndcToHiZUV.x = 0.5f;
		ndcToHiZUV.y = -0.5f;
		ndcToHiZUV.z = 0.5f;
		ndcToHiZUV.w = 0.5f;

		// Either of these flips the Y axis, but if they're both true they cancel out
		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();
		if (rapiInfo.isFlagSet(RenderAPIFeatureFlag::UVYAxisUp) ^ rapiInfo.isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown))
			ndcToHiZUV.y = -ndcToHiZUV.y;
		
		// Maps from [0, 1] to area of HiZ where depth is stored in
		ndcToHiZUV.x *= (float)viewRect.width / hiZProps.getWidth();
		ndcToHiZUV.y *= (float)viewRect.height / hiZProps.getHeight();
		ndcToHiZUV.z *= (float)viewRect.width / hiZProps.getWidth();
		ndcToHiZUV.w *= (float)viewRect.height / hiZProps.getHeight();
		
		// Maps from HiZ UV to [0, 1] UV
		Vector2 HiZUVToScreenUV;
		HiZUVToScreenUV.x = hiZProps.getWidth() / (float)viewRect.width;
		HiZUVToScreenUV.y = hiZProps.getHeight() / (float)viewRect.height;

		// Used for roughness fading
		Vector2 roughnessScaleBias = calcRoughnessFadeScaleBias(settings.maxRoughness);

		Vector2I bufferSize(viewRect.width, viewRect.height);
		gSSRTraceParamDef.gHiZSize.set(mParamBuffer, bufferSize);
		gSSRTraceParamDef.gHiZNumMips.set(mParamBuffer, hiZProps.getNumMipmaps());
		gSSRTraceParamDef.gNDCToHiZUV.set(mParamBuffer, ndcToHiZUV);
		gSSRTraceParamDef.gHiZUVToScreenUV.set(mParamBuffer, HiZUVToScreenUV);
		gSSRTraceParamDef.gIntensity.set(mParamBuffer, settings.intensity);
		gSSRTraceParamDef.gRoughnessScaleBias.set(mParamBuffer, roughnessScaleBias);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	Vector2 SSRTraceMat::calcRoughnessFadeScaleBias(float maxRoughness)
	{
		const static float RANGE_SCALE = 2.0f;

		Vector2 scaleBias;
		scaleBias.x = -RANGE_SCALE / (-1.0f + maxRoughness);
		scaleBias.y = (RANGE_SCALE * maxRoughness) / (-1.0f + maxRoughness);

		return scaleBias;
	}

	TemporalResolveParamDef gTemporalResolveParamDef;
	SSRResolveParamDef gSSRResolveParamDef;

	ShaderVariation SSRResolveMat::VAR_EyeAdaptation = ShaderVariation({
		ShaderVariation::Param("EYE_ADAPTATION", true),
		ShaderVariation::Param("MSAA", false)
	});

	ShaderVariation SSRResolveMat::VAR_NoEyeAdaptation = ShaderVariation({
		ShaderVariation::Param("EYE_ADAPTATION", false),
		ShaderVariation::Param("MSAA", false)
	});

	SSRResolveMat::SSRResolveMat()
	{
		mSSRParamBuffer = gSSRResolveParamDef.createBuffer();
		mTemporalParamBuffer = gTemporalResolveParamDef.createBuffer();

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneDepth", mSceneDepthTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gPrevColor", mPrevColorTexture);

		gpuParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mSSRParamBuffer);
		gpuParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "TemporalInput", mTemporalParamBuffer);

		SAMPLER_STATE_DESC pointSampDesc;
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::create(pointSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler", pointSampState);

		SAMPLER_STATE_DESC linearSampDesc;
		linearSampDesc.minFilter = FO_POINT;
		linearSampDesc.magFilter = FO_POINT;
		linearSampDesc.mipFilter = FO_POINT;
		linearSampDesc.addressMode.u = TAM_CLAMP;
		linearSampDesc.addressMode.v = TAM_CLAMP;
		linearSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> linearSampState = SamplerState::create(linearSampDesc);
		gpuParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler", linearSampState);
	}

	void SSRResolveMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_EyeAdaptation);
		variations.add(VAR_NoEyeAdaptation);
	}

	void SSRResolveMat::execute(const RendererView& view, const SPtr<Texture>& prevFrame, 
		const SPtr<Texture>& curFrame, const SPtr<Texture>& sceneDepth, const SPtr<RenderTarget>& destination)
	{
		// TODO - MSAA not supported (remember UV must be in pixels) 
		
		// Note: This shader should not be called when temporal AA is turned on
		// Note: This shader doesn't have velocity texture enabled and will only account for camera movement (can be easily
		//		 enabled when velocity texture is added)
		//   - WHen added, velocity should use a 16-bit SNORM format

		mPrevColorTexture.set(prevFrame);
		mSceneColorTexture.set(curFrame);
		mSceneDepthTexture.set(sceneDepth);

		if(mVariation.getBool("EYE_ADAPTATION"))
		{
			// TODO - Set eye adaptation texture
		}

		auto& colorProps = curFrame->getProperties(); // Assuming prev and current frame are the same size
		auto& depthProps = sceneDepth->getProperties();

		Vector2 colorPixelSize(1.0f / colorProps.getWidth(), 1.0f / colorProps.getHeight());
		Vector2 depthPixelSize(1.0f / depthProps.getWidth(), 1.0f / depthProps.getHeight());

		gSSRResolveParamDef.gSceneColorTexelSize.set(mSSRParamBuffer, colorPixelSize);
		gSSRResolveParamDef.gSceneDepthTexelSize.set(mSSRParamBuffer, depthPixelSize);
		// TODO - Set manual exposure value

		// Generate samples
		// Note: Move this code to a more general spot where it can be used by other temporal shaders.
		
		float sampleWeights[9];
		float sampleWeightsLowPass[9];

		float totalWeights = 0.0f;
		float totalWeightsLowPass = 0.0f;

		Vector2 jitter(BsZero); // Only relevant for general case, not using this type of jitter for SSR

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
				Vector2 offset = sampleOffsets[i] - jitter;

				offset *= 1.0f + sharpness * 0.5f;
				sampleWeights[i] = exp(-2.29f * offset.dot(offset));
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
				Vector2 offset = sampleOffsets[i] - jitter;

				offset *= 1.0f + sharpness * 0.5f;
				sampleWeights[i] = exp(-2.29f * offset.dot(offset));
				totalWeights += sampleWeights[i];

				// Low pass
				offset *= 0.25f;
				sampleWeightsLowPass[i] = exp(-2.29f * offset.dot(offset));
				totalWeightsLowPass += sampleWeightsLowPass[i];
			}
		}

		for (UINT32 i = 0; i < 9; ++i)
		{
			gTemporalResolveParamDef.gSampleWeights.set(mTemporalParamBuffer, sampleWeights[i] / totalWeights);
			gTemporalResolveParamDef.gSampleWeightsLowpass.set(mTemporalParamBuffer, sampleWeightsLowPass[i] / totalWeightsLowPass);
		}
		
		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParamsSet->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}
}}
