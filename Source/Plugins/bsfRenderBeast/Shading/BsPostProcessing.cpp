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
		if (params->hasSamplerState(gpType, name))
			params->setSamplerState(gpType, name, samplerState);
		else
		{
			if(optional)
			{
				if (params->hasSamplerState(gpType, secondaryName))
					params->setSamplerState(gpType, secondaryName, samplerState);
			}
			else
				params->setSamplerState(gpType, secondaryName, samplerState);
		}
	}

	DownsampleParamDef gDownsampleParamDef;

	DownsampleMat::DownsampleMat()
	{
		mParamBuffer = gDownsampleParamDef.createBuffer();

		if(mParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			mParams->setParamBlockBuffer("Input", mParamBuffer);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void DownsampleMat::execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

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

		bind();

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
				return get(getVariation<0, true>());
			else
				return get(getVariation<0, false>());
		}
		else
		{
			if (msaa)
				return get(getVariation<1, true>());
			else
				return get(getVariation<1, false>());
		}
	}

	EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

	EyeAdaptHistogramMat::EyeAdaptHistogramMat()
	{
		mParamBuffer = gEyeAdaptHistogramParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_COMPUTE_PROGRAM, "gSceneColorTex", mSceneColor);
		mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
	}

	void EyeAdaptHistogramMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE_X", THREAD_GROUP_SIZE_X);
		defines.set("THREADGROUP_SIZE_Y", THREAD_GROUP_SIZE_Y);
		defines.set("LOOP_COUNT_X", LOOP_COUNT_X);
		defines.set("LOOP_COUNT_Y", LOOP_COUNT_Y);
	}

	void EyeAdaptHistogramMat::execute(const SPtr<Texture>& input, const SPtr<Texture>& output,
		const AutoExposureSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mSceneColor.set(input);

		const TextureProperties& props = input->getProperties();
		Vector4I offsetAndSize(0, 0, (INT32)props.getWidth(), (INT32)props.getHeight());

		gEyeAdaptHistogramParamDef.gHistogramParams.set(mParamBuffer, getHistogramScaleOffset(settings));
		gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.set(mParamBuffer, offsetAndSize);

		Vector2I threadGroupCount = getThreadGroupCount(input);
		gEyeAdaptHistogramParamDef.gThreadGroupCount.set(mParamBuffer, threadGroupCount);

		// Dispatch
		mOutputTex.set(output);

		bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.dispatchCompute(threadGroupCount.x, threadGroupCount.y);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramMat::getOutputDesc(const SPtr<Texture>& target)
	{
		Vector2I threadGroupCount = getThreadGroupCount(target);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, HISTOGRAM_NUM_TEXELS, numHistograms,
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

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mHistogramTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
	}

	void EyeAdaptHistogramReduceMat::execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram,
		const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

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

		bind();

		Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2.0f);
		gRendererUtility().drawScreenQuad(drawUV);

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptHistogramReduceMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2,
			TU_RENDERTARGET);
	}

	EyeAdaptationParamDef gEyeAdaptationParamDef;

	EyeAdaptationMat::EyeAdaptationMat()
	{
		mParamBuffer = gEyeAdaptationParamDef.createBuffer();

		mParams->setParamBlockBuffer("EyeAdaptationParams", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mReducedHistogramTex);
	}

	void EyeAdaptationMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_X);
		defines.set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_Y);
	}

	void EyeAdaptationMat::execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output,
		float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mReducedHistogramTex.set(reducedHistogram);

		populateParams(mParamBuffer, frameDelta, settings, exposureScale);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptationMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
	}

	void EyeAdaptationMat::populateParams(const SPtr<GpuParamBlockBuffer>& paramBuffer, float frameDelta,
		const AutoExposureSettings& settings, float exposureScale)
	{
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

		eyeAdaptationParams[2].z = Math::pow(2.0f, settings.histogramLog2Min);
		eyeAdaptationParams[2].w = 0.0f; // Unused

		gEyeAdaptationParamDef.gEyeAdaptationParams.set(paramBuffer, eyeAdaptationParams[0], 0);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(paramBuffer, eyeAdaptationParams[1], 1);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(paramBuffer, eyeAdaptationParams[2], 2);
	}

	EyeAdaptationBasicSetupMat::EyeAdaptationBasicSetupMat()
	{
		mParamBuffer = gEyeAdaptationParamDef.createBuffer();

		mParams->setParamBlockBuffer("EyeAdaptationParams", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;

		SPtr<SamplerState> samplerState = SamplerState::create(desc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gInputSamp", "gInputTex", samplerState);
	}

	void EyeAdaptationBasicSetupMat::execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output,
		float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mInputTex.set(input);

		EyeAdaptationMat::populateParams(mParamBuffer, frameDelta, settings, exposureScale);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptationBasicSetupMat::getOutputDesc(const SPtr<Texture>& input)
	{
		auto& props = input->getProperties();
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, props.getWidth(), props.getHeight(), TU_RENDERTARGET);
	}

	EyeAdaptationBasicParamsMatDef gEyeAdaptationBasicParamsMatDef;

	EyeAdaptationBasicMat::EyeAdaptationBasicMat()
	{
		mEyeAdaptationParamsBuffer = gEyeAdaptationParamDef.createBuffer();
		mParamsBuffer = gEyeAdaptationBasicParamsMatDef.createBuffer();

		mParams->setParamBlockBuffer("EyeAdaptationParams", mEyeAdaptationParamsBuffer);
		mParams->setParamBlockBuffer("Input", mParamsBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gCurFrameTex", mCurFrameTexParam);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gPrevFrameTex", mPrevFrameTexParam);
	}

	void EyeAdaptationBasicMat::execute(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame,
		const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mCurFrameTexParam.set(curFrame);

		if (prevFrame == nullptr) // Could be that this is the first run
			mPrevFrameTexParam.set(Texture::WHITE);
		else
			mPrevFrameTexParam.set(prevFrame);

		EyeAdaptationMat::populateParams(mEyeAdaptationParamsBuffer, frameDelta, settings, exposureScale);

		auto& texProps = curFrame->getProperties();
		Vector2I texSize = { (INT32)texProps.getWidth(), (INT32)texProps.getHeight() };

		gEyeAdaptationBasicParamsMatDef.gInputTexSize.set(mParamsBuffer, texSize);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	POOLED_RENDER_TEXTURE_DESC EyeAdaptationBasicMat::getOutputDesc()
	{
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_R32F, 1, 1, TU_RENDERTARGET);
	}

	CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;
	WhiteBalanceParamDef gWhiteBalanceParamDef;

	CreateTonemapLUTMat::CreateTonemapLUTMat()
	{
		mIs3D = mVariation.getBool("VOLUME_LUT");

		mParamBuffer = gCreateTonemapLUTParamDef.createBuffer();
		mWhiteBalanceParamBuffer = gWhiteBalanceParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->setParamBlockBuffer("WhiteBalanceInput", mWhiteBalanceParamBuffer);

		if(mIs3D)
			mParams->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
	}

	void CreateTonemapLUTMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("LUT_SIZE", LUT_SIZE);
	}

	void CreateTonemapLUTMat::execute3D(const SPtr<Texture>& output, const RenderSettings& settings)
	{
		assert(mIs3D);
		BS_RENMAT_PROFILE_BLOCK

		populateParamBuffers(settings);

		// Dispatch
		mOutputTex.set(output);

		bind();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.dispatchCompute(LUT_SIZE / 8, LUT_SIZE / 8, LUT_SIZE);
	}

	void CreateTonemapLUTMat::execute2D(const SPtr<RenderTexture>& output, const RenderSettings& settings)
	{
		assert(!mIs3D);
		BS_RENMAT_PROFILE_BLOCK

		populateParamBuffers(settings);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	void CreateTonemapLUTMat::populateParamBuffers(const RenderSettings& settings)
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
	}

	POOLED_RENDER_TEXTURE_DESC CreateTonemapLUTMat::getOutputDesc() const
	{
		if(mIs3D)
			return POOLED_RENDER_TEXTURE_DESC::create3D(PF_RGBA8, LUT_SIZE, LUT_SIZE, LUT_SIZE, TU_LOADSTORE);
		
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA8, LUT_SIZE * LUT_SIZE, LUT_SIZE, TU_RENDERTARGET);
	}

	CreateTonemapLUTMat* CreateTonemapLUTMat::getVariation(bool is3D)
	{
		if(is3D)
			return get(getVariation<true>());
		
		return get(getVariation<false>());
	}

	TonemappingParamDef gTonemappingParamDef;

	TonemappingMat::TonemappingMat()
	{
		mParamBuffer = gTonemappingParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gBloomTex", mBloomTex);

		if(!mVariation.getBool("GAMMA_ONLY"))
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorLUT", mColorLUT);
	}

	void TonemappingMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE);
	}

	void TonemappingMat::execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation,
		const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output,
		const RenderSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& texProps = sceneColor->getProperties();

		gTonemappingParamDef.gRawGamma.set(mParamBuffer, 1.0f / settings.gamma);
		gTonemappingParamDef.gManualExposureScale.set(mParamBuffer, Math::pow(2.0f, settings.exposureScale));
		gTonemappingParamDef.gTexSize.set(mParamBuffer, Vector2((float)texProps.getWidth(), (float)texProps.getHeight()));
		gTonemappingParamDef.gBloomTint.set(mParamBuffer, settings.bloom.tint);
		gTonemappingParamDef.gNumSamples.set(mParamBuffer, texProps.getNumSamples());

		// Set parameters
		mInputTex.set(sceneColor);
		mColorLUT.set(colorLUT);
		mEyeAdaptationTex.set(eyeAdaptation);
		mBloomTex.set(bloom != nullptr ? bloom : Texture::BLACK);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	TonemappingMat* TonemappingMat::getVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA)
	{
		if(volumeLUT)
		{
			if (gammaOnly)
			{
				if (autoExposure)
				{
					if (MSAA)
						return get(getVariation<true, true, true, true>());
					else
						return get(getVariation<true, true, true, false>());
				}
				else
				{
					if (MSAA)
						return get(getVariation<true, true, false, true>());
					else
						return get(getVariation<true, true, false, false>());
				}
			}
			else
			{
				if (autoExposure)
				{
					if (MSAA)
						return get(getVariation<true, false, true, true>());
					else
						return get(getVariation<true, false, true, false>());
				}
				else
				{
					if (MSAA)
						return get(getVariation<true, false, false, true>());
					else
						return get(getVariation<true, false, false, false>());
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
						return get(getVariation<false, true, true, true>());
					else
						return get(getVariation<false, true, true, false>());
				}
				else
				{
					if (MSAA)
						return get(getVariation<false, true, false, true>());
					else
						return get(getVariation<false, true, false, false>());
				}
			}
			else
			{
				if (autoExposure)
				{
					if (MSAA)
						return get(getVariation<false, false, true, true>());
					else
						return get(getVariation<false, false, true, false>());
				}
				else
				{
					if (MSAA)
						return get(getVariation<false, false, false, true>());
					else
						return get(getVariation<false, false, false, false>());
				}
			}
		}
	}

	BloomClipParamDef gBloomClipParamDef;

	BloomClipMat::BloomClipMat()
	{
		mParamBuffer = gBloomClipParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	}

	void BloomClipMat::execute(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation,
		const RenderSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		gBloomClipParamDef.gThreshold.set(mParamBuffer, threshold);
		gBloomClipParamDef.gManualExposureScale.set(mParamBuffer, Math::pow(2.0f, settings.exposureScale));

		// Set parameters
		mInputTex.set(input);
		mEyeAdaptationTex.set(eyeAdaptation);

		// Render
		RenderAPI& rapi = RenderAPI::instance();

		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	BloomClipMat* BloomClipMat::getVariation(bool autoExposure)
	{
		if (autoExposure)
			return get(getVariation<true>());
		
		return get(getVariation<false>());
	}

	ScreenSpaceLensFlareParamDef gScreenSpaceLensFlareParamDef;

	ScreenSpaceLensFlareMat::ScreenSpaceLensFlareMat()
	{
		mParamBuffer = gScreenSpaceLensFlareParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gGradientTex", mGradientTex);
	}

	void ScreenSpaceLensFlareMat::execute(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		gScreenSpaceLensFlareParamDef.gThreshold.set(mParamBuffer, settings.threshold);
		gScreenSpaceLensFlareParamDef.gGhostCount.set(mParamBuffer, settings.ghostCount);
		gScreenSpaceLensFlareParamDef.gGhostSpacing.set(mParamBuffer, settings.ghostSpacing);
		gScreenSpaceLensFlareParamDef.gHaloRadius.set(mParamBuffer, settings.haloRadius);
		gScreenSpaceLensFlareParamDef.gHaloThickness.set(mParamBuffer, settings.haloThickness);
		gScreenSpaceLensFlareParamDef.gHaloThreshold.set(mParamBuffer, settings.haloThreshold);
		gScreenSpaceLensFlareParamDef.gHaloAspectRatio.set(mParamBuffer, settings.haloAspectRatio);
		gScreenSpaceLensFlareParamDef.gChromaticAberration.set(mParamBuffer, settings.chromaticAberrationOffset);

		mInputTex.set(input);
		mGradientTex.set(RendererTextures::lensFlareGradient);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	ScreenSpaceLensFlareMat* ScreenSpaceLensFlareMat::getVariation(bool halo, bool haloAspect, bool chromaticAberration)
	{
		if(halo)
		{
			if(haloAspect)
			{
				if(chromaticAberration)
					return get(getVariation<1, true>());
				
				return get(getVariation<1, false>());
			}
			else
			{
				if(chromaticAberration)
					return get(getVariation<2, true>());
				
				return get(getVariation<2, false>());
			}
		}
		else
		{
			if (chromaticAberration)
				return get(getVariation<0, true>());

			return get(getVariation<0, false>());
		}
	}

	ChromaticAberrationParamDef gChromaticAberrationParamDef;

	constexpr int ChromaticAberrationMat::MAX_SAMPLES;

	ChromaticAberrationMat::ChromaticAberrationMat()
	{
		mParamBuffer = gChromaticAberrationParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFringeTex", mFringeTex);
	}

	void ChromaticAberrationMat::execute(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& texProps = input->getProperties();
		
		// Set parameters
		gChromaticAberrationParamDef.gInputSize.set(mParamBuffer,
			Vector2((float)texProps.getWidth(), (float)texProps.getHeight()));

		gChromaticAberrationParamDef.gShiftAmount.set(mParamBuffer, settings.shiftAmount);
		
		SPtr<Texture> fringeTex;
		if (settings.fringeTexture)
			fringeTex = settings.fringeTexture;
		else
			fringeTex = RendererTextures::chromaticAberrationFringe;
		
		mInputTex.set(input);
		mFringeTex.set(fringeTex);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	ChromaticAberrationMat* ChromaticAberrationMat::getVariation(ChromaticAberrationType type)
	{
		if (type == ChromaticAberrationType::Complex)
			return get(getVariation<false>());

		return get(getVariation<true>());
	}

	void ChromaticAberrationMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("MAX_SAMPLES", MAX_SAMPLES);
	}

	FilmGrainParamDef gFilmGrainParamDef;

	FilmGrainMat::FilmGrainMat()
	{
		mParamBuffer = gFilmGrainParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);
	}

	void FilmGrainMat::execute(const SPtr<Texture>& input, float time,
		const FilmGrainSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		gFilmGrainParamDef.gIntensity.set(mParamBuffer, settings.intensity);
		gFilmGrainParamDef.gTime.set(mParamBuffer, settings.speed * time);

		mInputTex.set(input);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	GaussianBlurParamDef gGaussianBlurParamDef;

	GaussianBlurMat::GaussianBlurMat()
	{
		mParamBuffer = gGaussianBlurParamDef.createBuffer();
		mIsAdditive = mVariation.getBool("ADDITIVE");

		mParams->setParamBlockBuffer("GaussianBlurParams", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);

		if(mIsAdditive)
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gAdditiveTex", mAdditiveTexture);
	}

	void GaussianBlurMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("MAX_NUM_SAMPLES", MAX_BLUR_SAMPLES);
	}

	void GaussianBlurMat::execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination,
		const Color& tint, const SPtr<Texture>& additive)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = source->getProperties();
		const RenderTextureProperties& dstProps = destination->getProperties();

		POOLED_RENDER_TEXTURE_DESC tempTextureDesc = POOLED_RENDER_TEXTURE_DESC::create2D(srcProps.getFormat(),
			dstProps.width, dstProps.height, TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = gGpuResourcePool().get(tempTextureDesc);

		// Horizontal pass
		{
			populateBuffer(mParamBuffer, DirHorizontal, source, filterSize, Color::White);
			mInputTexture.set(source);

			if(mIsAdditive)
				mAdditiveTexture.set(Texture::BLACK);

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(tempTexture->renderTexture);

			bind();
			gRendererUtility().drawScreenQuad();
		}

		// Vertical pass
		{
			populateBuffer(mParamBuffer, DirVertical, source, filterSize, tint);
			mInputTexture.set(tempTexture->texture);

			if(mIsAdditive)
			{
				if(additive)
					mAdditiveTexture.set(additive);
				else
					mAdditiveTexture.set(Texture::BLACK);
			}

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(destination);

			bind();
			gRendererUtility().drawScreenQuad();
		}
	}

	UINT32 GaussianBlurMat::calcStdDistribution(float filterRadius, std::array<float, MAX_BLUR_SAMPLES>& weights,
		std::array<float, MAX_BLUR_SAMPLES>& offsets)
	{
		filterRadius = Math::clamp(filterRadius, 0.00001f, (float)(MAX_BLUR_SAMPLES - 1));
		INT32 intFilterRadius = std::min(Math::ceilToInt(filterRadius), MAX_BLUR_SAMPLES - 1);

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

	void GaussianBlurMat::populateBuffer(const SPtr<GpuParamBlockBuffer>& buffer, Direction direction,
		const SPtr<Texture>& source, float filterSize, const Color& tint)
	{
		const TextureProperties& srcProps = source->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());

		std::array<float, MAX_BLUR_SAMPLES> sampleOffsets;
		std::array<float, MAX_BLUR_SAMPLES> sampleWeights;

		const float kernelRadius = calcKernelRadius(source, filterSize, direction);
		const UINT32 numSamples = calcStdDistribution(kernelRadius, sampleWeights, sampleOffsets);

		for (UINT32 i = 0; i < numSamples; ++i)
		{
			Vector4 weight(tint.r, tint.g, tint.b, tint.a);
			weight *= sampleWeights[i];

			gGaussianBlurParamDef.gSampleWeights.set(buffer, weight, i);
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

			gGaussianBlurParamDef.gSampleOffsets.set(buffer, offset, i);
		}

		gGaussianBlurParamDef.gNumSamples.set(buffer, numSamples);
	}

	GaussianBlurMat* GaussianBlurMat::getVariation(bool additive)
	{
		if(additive)
			return get(getVariation<true>());

		return get(getVariation<false>());
	}

	GaussianDOFParamDef gGaussianDOFParamDef;

	GaussianDOFSeparateMat::GaussianDOFSeparateMat()
	{
		mParamBuffer = gGaussianDOFParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorTex", mColorTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::create(desc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gColorSamp", "gColorTex", samplerState);
	}

	void GaussianDOFSeparateMat::execute(const SPtr<Texture>& color, const SPtr<Texture>& depth,
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = color->getProperties();

		UINT32 outputWidth = std::max(1U, srcProps.getWidth() / 2);
		UINT32 outputHeight = std::max(1U, srcProps.getHeight() / 2);

		POOLED_RENDER_TEXTURE_DESC outputTexDesc = POOLED_RENDER_TEXTURE_DESC::create2D(srcProps.getFormat(),
			outputWidth, outputHeight, TU_RENDERTARGET);
		mOutput0 = gGpuResourcePool().get(outputTexDesc);

		bool near = mVariation.getBool("NEAR");
		bool far = mVariation.getBool("FAR");

		SPtr<RenderTexture> rt;
		if (near && far)
		{
			mOutput1 = gGpuResourcePool().get(outputTexDesc);

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
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(rt);

		bind();
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
		mOutput0 = nullptr;
		mOutput1 = nullptr;
	}

	GaussianDOFSeparateMat* GaussianDOFSeparateMat::getVariation(bool near, bool far)
	{
		if (near)
		{
			if (far)
				return get(getVariation<true, true>());
			else
				return get(getVariation<true, false>());
		}
		else
			return get(getVariation<false, true>());
	}

	GaussianDOFCombineMat::GaussianDOFCombineMat()
	{
		mParamBuffer = gGaussianDOFParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFocusedTex", mFocusedTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		if(mParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gNearTex"))
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNearTex", mNearTexture);

		if(mParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gFarTex"))
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFarTex", mFarTexture);
	}

	void GaussianDOFCombineMat::execute(const SPtr<Texture>& focused, const SPtr<Texture>& near,
		const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output,
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

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
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	GaussianDOFCombineMat* GaussianDOFCombineMat::getVariation(bool near, bool far)
	{
		if (near)
		{
			if (far)
				return get(getVariation<true, true>());
			else
				return get(getVariation<true, false>());
		}
		else
			return get(getVariation<false, true>());
	}

	DepthOfFieldCommonParamDef gDepthOfFieldCommonParamDef;
	BokehDOFPrepareParamDef gBokehDOFPrepareParamDef;

	BokehDOFPrepareMat::BokehDOFPrepareMat()
	{
		mParamBuffer = gBokehDOFPrepareParamDef.createBuffer();
		mCommonParamBuffer = gDepthOfFieldCommonParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->setParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);
	}

	void BokehDOFPrepareMat::execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
		const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = input->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());
		gBokehDOFPrepareParamDef.gInvInputSize.set(mParamBuffer, invTexSize);

		BokehDOFMat::populateDOFCommonParams(mCommonParamBuffer, settings, view);

		mInputTexture.set(input);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();

		bool MSAA = mVariation.getInt("MSAA_COUNT") > 1;
		if (MSAA)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)srcProps.getWidth(), (float)srcProps.getHeight()));
		else
			gRendererUtility().drawScreenQuad();
	}

	POOLED_RENDER_TEXTURE_DESC BokehDOFPrepareMat::getOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->getProperties();

		UINT32 width = std::max(1U, Math::divideAndRoundUp(rtProps.getWidth(), 2U));
		UINT32 height = std::max(1U, Math::divideAndRoundUp(rtProps.getHeight(), 2U));

		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
	}

	BokehDOFPrepareMat* BokehDOFPrepareMat::getVariation(bool msaa)
	{
		if (msaa)
			return get(getVariation<true>());
		else
			return get(getVariation<false>());
	}

	BokehDOFParamDef gBokehDOFParamDef;

	constexpr UINT32 BokehDOFMat::NEAR_FAR_PADDING;
	constexpr UINT32 BokehDOFMat::QUADS_PER_TILE;

	BokehDOFMat::BokehDOFMat()
	{
		mParamBuffer = gBokehDOFParamDef.createBuffer();
		mCommonParamBuffer = gDepthOfFieldCommonParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->setParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);
		mParams->getTextureParam(GPT_VERTEX_PROGRAM, "gInputTex", mInputTextureVS);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTextureFS);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gBokehTex", mBokehTexture);

		// Prepare vertex declaration for rendering tiles
		SPtr<VertexDataDesc> tileVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		tileVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		mTileVertexDecl = VertexDeclaration::create(tileVertexDesc);

		// Prepare vertex buffer for rendering tiles
		VERTEX_BUFFER_DESC tileVertexBufferDesc;
		tileVertexBufferDesc.numVerts = QUADS_PER_TILE * 4;
		tileVertexBufferDesc.vertexSize = tileVertexDesc->getVertexStride();

		mTileVertexBuffer = VertexBuffer::create(tileVertexBufferDesc);

		auto* const vertexData = (Vector2*)mTileVertexBuffer->lock(GBL_WRITE_ONLY_DISCARD);
		for (UINT32 i = 0; i < QUADS_PER_TILE; i++)
		{
			vertexData[i * 4 + 0] = Vector2(0.0f, 0.0f);
			vertexData[i * 4 + 1] = Vector2(1.0f, 0.0f);
			vertexData[i * 4 + 2] = Vector2(0.0f, 1.0f);
			vertexData[i * 4 + 3] = Vector2(1.0f, 1.0f);
		}

		mTileVertexBuffer->unlock();

		// Prepare indices for rendering tiles
		INDEX_BUFFER_DESC tileIndexBufferDesc;
		tileIndexBufferDesc.indexType = IT_16BIT;
		tileIndexBufferDesc.numIndices = QUADS_PER_TILE * 6;

		mTileIndexBuffer = IndexBuffer::create(tileIndexBufferDesc);

		auto* const indices = (UINT16*)mTileIndexBuffer->lock(GBL_WRITE_ONLY_DISCARD);

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

		mTileIndexBuffer->unlock();
	}

	void BokehDOFMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("QUADS_PER_TILE", QUADS_PER_TILE);
	}

	void BokehDOFMat::execute(const SPtr<Texture>& input, const RendererView& view,
		const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = input->getProperties();
		const RenderTargetProperties& dstProps = output->getProperties();

		Vector2 inputInvTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());
		Vector2 outputInvTexSize(1.0f / dstProps.width, 1.0f / dstProps.height);
		gBokehDOFParamDef.gInvInputSize.set(mParamBuffer, inputInvTexSize);
		gBokehDOFParamDef.gInvOutputSize.set(mParamBuffer, outputInvTexSize);
		gBokehDOFParamDef.gAdaptiveThresholdCOC.set(mParamBuffer, settings.adaptiveRadiusThreshold);
		gBokehDOFParamDef.gAdaptiveThresholdColor.set(mParamBuffer, settings.adaptiveColorThreshold);
		gBokehDOFParamDef.gLayerPixelOffset.set(mParamBuffer, (INT32)srcProps.getHeight() + (INT32)NEAR_FAR_PADDING);
		gBokehDOFParamDef.gInvDepthRange.set(mParamBuffer, 1.0f / settings.occlusionDepthRange);

		float bokehSize = settings.maxBokehSize * srcProps.getWidth();
		gBokehDOFParamDef.gBokehSize.set(mParamBuffer, Vector2(bokehSize, bokehSize));

		Vector2I imageSize(srcProps.getWidth(), srcProps.getHeight());

		// TODO - Allow tile count to halve (i.e. half sampling rate)
		Vector2I tileCount = imageSize / 1;
		gBokehDOFParamDef.gTileCount.set(mParamBuffer, tileCount);

		populateDOFCommonParams(mCommonParamBuffer, settings, view);
		mInputTextureVS.set(input);
		mInputTextureFS.set(input);

		SPtr<Texture> bokehTexture = settings.bokehShape;
		if(bokehTexture == nullptr)
			bokehTexture = RendererTextures::bokehFlare;

		mBokehTexture.set(bokehTexture);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);
		rapi.clearRenderTarget(FBT_COLOR, Color::ZERO);
		rapi.setVertexDeclaration(mTileVertexDecl);

		SPtr<VertexBuffer> buffers[] = { mTileVertexBuffer };
		rapi.setVertexBuffers(0, buffers, (UINT32)bs_size(buffers));
		rapi.setIndexBuffer(mTileIndexBuffer);
		rapi.setDrawOperation(DOT_TRIANGLE_LIST);

		bind();
		const UINT32 numInstances = Math::divideAndRoundUp((UINT32)(tileCount.x * tileCount.y), QUADS_PER_TILE);
		rapi.drawIndexed(0, QUADS_PER_TILE * 6, 0, QUADS_PER_TILE * 4, numInstances);
	}

	POOLED_RENDER_TEXTURE_DESC BokehDOFMat::getOutputDesc(const SPtr<Texture>& target)
	{
		const TextureProperties& rtProps = target->getProperties();

		UINT32 width = rtProps.getWidth();
		UINT32 height = rtProps.getHeight() * 2 + NEAR_FAR_PADDING;

		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_RGBA16F, width, height, TU_RENDERTARGET);
	}

	void BokehDOFMat::populateDOFCommonParams(const SPtr<GpuParamBlockBuffer>& buffer, const DepthOfFieldSettings& settings,
		const RendererView& view)
	{
		gDepthOfFieldCommonParamDef.gFocalPlaneDistance.set(buffer, settings.focalDistance);
		gDepthOfFieldCommonParamDef.gApertureSize.set(buffer, settings.apertureSize * 0.001f); // mm to m
		gDepthOfFieldCommonParamDef.gFocalLength.set(buffer, settings.focalLength * 0.001f); // mm to m
		gDepthOfFieldCommonParamDef.gInFocusRange.set(buffer, settings.focalRange);
		gDepthOfFieldCommonParamDef.gNearTransitionRegion.set(buffer, settings.nearTransitionRange);
		gDepthOfFieldCommonParamDef.gFarTransitionRegion.set(buffer, settings.farTransitionRange);

		float sensorSize, imageSize;
		if(settings.sensorSize.x < settings.sensorSize.y)
		{
			sensorSize = settings.sensorSize.x;
			imageSize = (float)view.getProperties().target.targetWidth;
		}
		else
		{
			sensorSize = settings.sensorSize.y;
			imageSize = (float)view.getProperties().target.targetHeight;
		}

		gDepthOfFieldCommonParamDef.gSensorSize.set(buffer, sensorSize);
		gDepthOfFieldCommonParamDef.gImageSize.set(buffer, imageSize);
		gDepthOfFieldCommonParamDef.gMaxBokehSize.set(buffer, Math::clamp01(settings.maxBokehSize) * imageSize);
	}

	BokehDOFMat* BokehDOFMat::getVariation(bool depthOcclusion)
	{
		if (depthOcclusion)
			return get(getVariation<true>());
		else
			return get(getVariation<false>());
	}

	BokehDOFCombineParamDef gBokehDOFCombineParamDef;

	BokehDOFCombineMat::BokehDOFCombineMat()
	{
		mParamBuffer = gBokehDOFPrepareParamDef.createBuffer();
		mCommonParamBuffer = gDepthOfFieldCommonParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->setParamBlockBuffer("DepthOfFieldParams", mCommonParamBuffer);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gUnfocusedTex", mUnfocusedTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gFocusedTex", mFocusedTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);
	}

	void BokehDOFCombineMat::execute(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused,
		const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings,
		const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& focusedProps = focused->getProperties();
		const TextureProperties& unfocusedProps = unfocused->getProperties();
		UINT32 halfHeight = std::max(1U, Math::divideAndRoundUp(focusedProps.getHeight(), 2U));

		float uvScale = halfHeight / (float)unfocusedProps.getHeight();
		float uvOffset = (halfHeight + BokehDOFMat::NEAR_FAR_PADDING) / (float)unfocusedProps.getHeight();

		Vector2 layerScaleOffset(uvScale, uvOffset);
		Vector2 focusedImageSize((float)focusedProps.getWidth(), (float)focusedProps.getHeight());
		gBokehDOFCombineParamDef.gLayerAndScaleOffset.set(mParamBuffer, layerScaleOffset);
		gBokehDOFCombineParamDef.gFocusedImageSize.set(mParamBuffer, focusedImageSize);

		BokehDOFMat::populateDOFCommonParams(mCommonParamBuffer, settings, view);

		mUnfocusedTexture.set(unfocused);
		mFocusedTexture.set(focused);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	BokehDOFCombineMat* BokehDOFCombineMat::getVariation(MSAAMode msaaMode)
	{
		switch(msaaMode)
		{
		default:
		case MSAAMode::None: 
			return get(getVariation<MSAAMode::None>());
		case MSAAMode::Single: 
			return get(getVariation<MSAAMode::Single>());
		case MSAAMode::Full: 
			return get(getVariation<MSAAMode::Full>());
		}
	}

	MotionBlurParamDef gMotionBlurParamDef;

	MotionBlurMat::MotionBlurMat()
	{
		mParamBuffer = gBokehDOFPrepareParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthTexture);

		SAMPLER_STATE_DESC pointSampDesc;
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::create(pointSampDesc);

		if (mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp", pointSampState);
	}

	void MotionBlurMat::execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
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
		
		gMotionBlurParamDef.gHalfNumSamples.set(mParamBuffer, numSamples / 2);

		mInputTexture.set(input);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	BuildHiZFParamDef gBuildHiZParamDef;

	BuildHiZMat::BuildHiZMat()
	{
		mNoTextureViews = mVariation.getBool("NO_TEXTURE_VIEWS");

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mInputTexture);

		// If no texture view support, we must manually pick a valid mip level in the shader
		if(mNoTextureViews)
		{
			mParamBuffer = gBuildHiZParamDef.createBuffer();
			mParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);

			SAMPLER_STATE_DESC inputSampDesc;
			inputSampDesc.minFilter = FO_POINT;
			inputSampDesc.magFilter = FO_POINT;
			inputSampDesc.mipFilter = FO_POINT;

			SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
			setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gDepthSamp", "gDepthTex", inputSampState);
		}
	}

	void BuildHiZMat::execute(const SPtr<Texture>& source, UINT32 srcMip, const Rect2& srcRect, const Rect2& dstRect,
		const SPtr<RenderTexture>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		RenderAPI& rapi = RenderAPI::instance();

		// If no texture view support, we must manually pick a valid mip level in the shader
		if(mNoTextureViews)
		{
			mInputTexture.set(source);

			auto& props = source->getProperties();
			float pixelWidth = (float)props.getWidth();
			float pixelHeight = (float)props.getHeight();

			Vector2 halfPixelOffset(0.5f / pixelWidth, 0.5f / pixelHeight);

			gBuildHiZParamDef.gHalfPixelOffset.set(mParamBuffer, halfPixelOffset);
			gBuildHiZParamDef.gMipLevel.set(mParamBuffer, srcMip);
		}
		else
			mInputTexture.set(source, TextureSurface(srcMip));

		rapi.setRenderTarget(output);
		rapi.setViewport(dstRect);

		bind();
		gRendererUtility().drawScreenQuad(srcRect);

		rapi.setViewport(Rect2(0, 0, 1, 1));
	}

	BuildHiZMat* BuildHiZMat::getVariation(bool noTextureViews)
	{
		if (noTextureViews)
			return get(getVariation<true>());
		
		return get(getVariation<false>());
	}

	FXAAParamDef gFXAAParamDef;

	FXAAMat::FXAAMat()
	{
		mParamBuffer = gFXAAParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void FXAAMat::execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination)
	{
		BS_RENMAT_PROFILE_BLOCK

		const TextureProperties& srcProps = source->getProperties();

		Vector2 invTexSize(1.0f / srcProps.getWidth(), 1.0f / srcProps.getHeight());
		gFXAAParamDef.gInvTexSize.set(mParamBuffer, invTexSize);

		mInputTexture.set(source);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	SSAOParamDef gSSAOParamDef;

	SSAOMat::SSAOMat()
	{
		bool isFinal = mVariation.getBool("FINAL_AO");
		bool mixWithUpsampled = mVariation.getBool("MIX_WITH_UPSAMPLED");

		mParamBuffer = gSSAOParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);

		if (isFinal)
		{
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);
		}
		
		if(!isFinal || mixWithUpsampled)
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSetupAO", mSetupAOTexture);

		if(mixWithUpsampled)
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDownsampledAO", mDownsampledAOTexture);

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gRandomTex", mRandomTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			if (isFinal)
			{
				mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
				mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gNormalsTex", inputSampState);
			}
			
			if(!isFinal || mixWithUpsampled)
				mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gSetupAO", inputSampState);

			if(mixWithUpsampled)
				mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDownsampledAO", inputSampState);
		}

		SAMPLER_STATE_DESC randomSampDesc;
		randomSampDesc.minFilter = FO_POINT;
		randomSampDesc.magFilter = FO_POINT;
		randomSampDesc.mipFilter = FO_POINT;
		randomSampDesc.addressMode.u = TAM_WRAP;
		randomSampDesc.addressMode.v = TAM_WRAP;
		randomSampDesc.addressMode.w = TAM_WRAP;

		SPtr<SamplerState> randomSampState = SamplerState::create(randomSampDesc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gRandomSamp", "gRandomTex", randomSampState);
	}

	void SSAOMat::execute(const RendererView& view, const SSAOTextureInputs& textures,
		const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

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
		float viewScale = viewProps.target.viewRect.width / (float)rtProps.width;

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
		UINT32 scaleWidth = (rtProps.width + rndWidth - 1) / rndWidth;
		UINT32 scaleHeight = (rtProps.height + rndHeight - 1) / rndHeight;

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
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	SSAOMat* SSAOMat::getVariation(bool upsample, bool finalPass, int quality)
	{
#define PICK_MATERIAL(QUALITY)															\
		if(upsample)																	\
			if(finalPass)																\
				return get(getVariation<true, true, QUALITY>());						\
			else																		\
				return get(getVariation<true, false, QUALITY>());						\
		else																			\
			if(finalPass)																\
				return get(getVariation<false, true, QUALITY>());						\
			else																		\
				return get(getVariation<false, false, QUALITY>());						\

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

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gNormalsTex", mNormalsTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_LINEAR;
		inputSampDesc.magFilter = FO_LINEAR;
		inputSampDesc.mipFilter = FO_LINEAR;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);

		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gNormalsTex", inputSampState);
		}
	}

	void SSAODownsampleMat::execute(const RendererView& view, const SPtr<Texture>& depth, const SPtr<Texture>& normals,
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.getProperties();
		const RenderTargetProperties& rtProps = destination->getProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / rtProps.width;
		pixelSize.y = 1.0f / rtProps.height;

		float scale = viewProps.target.viewRect.width / (float)rtProps.width;

		gSSAODownsampleParamDef.gPixelSize.set(mParamBuffer, pixelSize);
		gSSAODownsampleParamDef.gInvDepthThreshold.set(mParamBuffer, (1.0f / depthRange) / scale);

		mDepthTexture.set(depth);
		mNormalsTexture.set(normals);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	SSAOBlurParamDef gSSAOBlurParamDef;

	SSAOBlurMat::SSAOBlurMat()
	{
		mParamBuffer = gSSAOBlurParamDef.createBuffer();

		mParams->setParamBlockBuffer("Input", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mAOTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mDepthTexture);

		SAMPLER_STATE_DESC inputSampDesc;
		inputSampDesc.minFilter = FO_POINT;
		inputSampDesc.magFilter = FO_POINT;
		inputSampDesc.mipFilter = FO_POINT;
		inputSampDesc.addressMode.u = TAM_CLAMP;
		inputSampDesc.addressMode.v = TAM_CLAMP;
		inputSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> inputSampState = SamplerState::create(inputSampDesc);
		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputSamp", inputSampState);
		else
		{
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gInputTex", inputSampState);
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthTex", inputSampState);
		}
	}

	void SSAOBlurMat::execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth,
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		BS_RENMAT_PROFILE_BLOCK

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

		float scale = viewProps.target.viewRect.width / (float)texProps.getWidth();

		gSSAOBlurParamDef.gPixelSize.set(mParamBuffer, pixelSize);
		gSSAOBlurParamDef.gPixelOffset.set(mParamBuffer, pixelOffset);
		gSSAOBlurParamDef.gInvDepthThreshold.set(mParamBuffer, (1.0f / depthRange) / scale);

		mAOTexture.set(ao);
		mDepthTexture.set(depth);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	SSAOBlurMat* SSAOBlurMat::getVariation(bool horizontal)
	{
		if (horizontal)
			return get(getVariation<true>());
		
		return get(getVariation<false>());
	}

	SSRStencilParamDef gSSRStencilParamDef;

	SSRStencilMat::SSRStencilMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParamBuffer = gSSRStencilParamDef.createBuffer();
		mParams->setParamBlockBuffer("Input", mParamBuffer);
	}

	void SSRStencilMat::execute(const RendererView& view, GBufferTextures gbuffer,
		const ScreenSpaceReflectionsSettings& settings)
	{
		BS_RENMAT_PROFILE_BLOCK

		mGBufferParams.bind(gbuffer);

		Vector2 roughnessScaleBias = SSRTraceMat::calcRoughnessFadeScaleBias(settings.maxRoughness);
		gSSRStencilParamDef.gRoughnessScaleBias.set(mParamBuffer, roughnessScaleBias);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		const RendererViewProperties& viewProps = view.getProperties();
		const Rect2I& viewRect = viewProps.target.viewRect;
		bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
			gRendererUtility().drawScreenQuad();
	}

	SSRStencilMat* SSRStencilMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}
		else
			return get(getVariation<false, false>());
	}

	SSRTraceParamDef gSSRTraceParamDef;

	SSRTraceMat::SSRTraceMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mParamBuffer = gSSRTraceParamDef.createBuffer();

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHiZ", mHiZTexture);

		if(mParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			mParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);

		SAMPLER_STATE_DESC desc;
		desc.minFilter = FO_POINT;
		desc.magFilter = FO_POINT;
		desc.mipFilter = FO_POINT;
		desc.addressMode.u = TAM_CLAMP;
		desc.addressMode.v = TAM_CLAMP;
		desc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> hiZSamplerState = SamplerState::create(desc);
		if (mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZSamp", hiZSamplerState);
		else if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gHiZ", hiZSamplerState);
	}

	void SSRTraceMat::execute(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor,
			const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings,
			const SPtr<RenderTarget>& destination)
	{
		BS_RENMAT_PROFILE_BLOCK

		const RendererViewProperties& viewProps = view.getProperties();

		const TextureProperties& hiZProps = hiZ->getProperties();

		mGBufferParams.bind(gbuffer);
		mSceneColorTexture.set(sceneColor);
		mHiZTexture.set(hiZ);
		
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

		UINT32 temporalJitter = (viewProps.frameIdx % 8) * 1503;

		Vector2I bufferSize(viewRect.width, viewRect.height);
		gSSRTraceParamDef.gHiZSize.set(mParamBuffer, bufferSize);
		gSSRTraceParamDef.gHiZNumMips.set(mParamBuffer, hiZProps.getNumMipmaps());
		gSSRTraceParamDef.gNDCToHiZUV.set(mParamBuffer, ndcToHiZUV);
		gSSRTraceParamDef.gHiZUVToScreenUV.set(mParamBuffer, HiZUVToScreenUV);
		gSSRTraceParamDef.gIntensity.set(mParamBuffer, settings.intensity);
		gSSRTraceParamDef.gRoughnessScaleBias.set(mParamBuffer, roughnessScaleBias);
		gSSRTraceParamDef.gTemporalJitter.set(mParamBuffer, temporalJitter);

		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination, FBT_DEPTH | FBT_STENCIL, RT_DEPTH_STENCIL);

		bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
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

	SSRTraceMat* SSRTraceMat::getVariation(UINT32 quality, bool msaa, bool singleSampleMSAA)
	{
#define PICK_MATERIAL(QUALITY)											\
		if(msaa)														\
			if(singleSampleMSAA)										\
				return get(getVariation<QUALITY, true, true>());		\
			else														\
				return get(getVariation<QUALITY, true, false>());		\
		else															\
				return get(getVariation<QUALITY, false, false>());		\

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
		mParamBuffer = gTemporalFilteringParamDef.createBuffer();
		mTemporalParamBuffer = gTemporalResolveParamDef.createBuffer();

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneDepth", mSceneDepthTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSceneColor", mSceneColorTexture);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gPrevColor", mPrevColorTexture);

		mHasVelocityTexture = mVariation.getBool("PER_PIXEL_VELOCITY");
		if(mHasVelocityTexture)
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gVelocity", mVelocityTexture);

		mParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "Input", mParamBuffer);
		mParams->setParamBlockBuffer(GPT_FRAGMENT_PROGRAM, "TemporalInput", mTemporalParamBuffer);

		SAMPLER_STATE_DESC pointSampDesc;
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::create(pointSampDesc);

		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gPointSampler", pointSampState);
		else
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneDepth", pointSampState);

		SAMPLER_STATE_DESC linearSampDesc;
		linearSampDesc.minFilter = FO_POINT;
		linearSampDesc.magFilter = FO_POINT;
		linearSampDesc.mipFilter = FO_POINT;
		linearSampDesc.addressMode.u = TAM_CLAMP;
		linearSampDesc.addressMode.v = TAM_CLAMP;
		linearSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> linearSampState = SamplerState::create(linearSampDesc);
		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gLinearSampler", linearSampState);
		else
		{
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gSceneColor", linearSampState);
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gPrevColor", linearSampState);
		}
	}

	void TemporalFilteringMat::execute(const RendererView& view, const SPtr<Texture>& prevFrame,
		const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth,
		const Vector2& jitter, float exposure, const SPtr<RenderTarget>& destination)
	{
		BS_RENMAT_PROFILE_BLOCK

		SPtr<Texture> velocityTex = velocity;
		if (!velocityTex)
			velocityTex = Texture::BLACK;

		mPrevColorTexture.set(prevFrame);
		mSceneColorTexture.set(curFrame);
		mSceneDepthTexture.set(sceneDepth);

		if(mHasVelocityTexture)
			mVelocityTexture.set(velocityTex);

		auto& colorProps = curFrame->getProperties(); // Assuming prev and current frame are the same size
		auto& depthProps = sceneDepth->getProperties();

		Vector4 colorPixelSize(1.0f / colorProps.getWidth(), 1.0f / colorProps.getHeight(),
			(float)colorProps.getWidth(), (float)colorProps.getHeight());
		Vector4 depthPixelSize(1.0f / depthProps.getWidth(), 1.0f / depthProps.getHeight(),
			(float)depthProps.getWidth(), (float)depthProps.getHeight());

		Vector4 velocityPixelSize(1.0f, 1.0f, 1.0f, 1.0f);
		if(mHasVelocityTexture)
		{
			auto& velocityProps = velocityTex->getProperties();
			velocityPixelSize = Vector4(1.0f / velocityProps.getWidth(), 1.0f / velocityProps.getHeight(),
				(float)velocityProps.getWidth(), (float)velocityProps.getHeight());
		}

		gTemporalFilteringParamDef.gSceneColorTexelSize.set(mParamBuffer, colorPixelSize);
		gTemporalFilteringParamDef.gSceneDepthTexelSize.set(mParamBuffer, depthPixelSize);
		gTemporalFilteringParamDef.gVelocityTexelSize.set(mParamBuffer, velocityPixelSize);
		gTemporalFilteringParamDef.gManualExposure.set(mParamBuffer, 1.0f / exposure);

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
				Vector2 offset = sampleOffsets[i] - jitterUV;

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
			gTemporalResolveParamDef.gSampleWeights.set(mTemporalParamBuffer, sampleWeights[i] / totalWeights, i);
			gTemporalResolveParamDef.gSampleWeightsLowpass.set(mTemporalParamBuffer, sampleWeightsLowPass[i] / totalWeightsLowPass, i);
		}
		
		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(destination);

		const RendererViewProperties& viewProps = view.getProperties();
		const Rect2I& viewRect = viewProps.target.viewRect;

		bind();

		if(viewProps.target.numSamples > 1)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)viewRect.width, (float)viewRect.height));
		else
			gRendererUtility().drawScreenQuad();
	}

	TemporalFilteringMat* TemporalFilteringMat::getVariation(TemporalFilteringType type, bool velocity, bool msaa)
	{
		switch(type)
		{
		default:
		case TemporalFilteringType::FullScreenAA:
			if(velocity)
			{
				if (msaa)
					return get(getVariation<TemporalFilteringType::FullScreenAA, true, true>());

				return get(getVariation<TemporalFilteringType::FullScreenAA, true, false>());
			}

			if (msaa)
				return get(getVariation<TemporalFilteringType::FullScreenAA, false, true>());

			return get(getVariation<TemporalFilteringType::FullScreenAA, false, false>());
		case TemporalFilteringType::SSR:
			if(velocity)
			{
				if (msaa)
					return get(getVariation<TemporalFilteringType::SSR, true, true>());

				return get(getVariation<TemporalFilteringType::SSR, true, false>());
			}

			if (msaa)
				return get(getVariation<TemporalFilteringType::SSR, false, true>());

			return get(getVariation<TemporalFilteringType::SSR, false, false>());
		}
	}

	EncodeDepthParamDef gEncodeDepthParamDef;

	EncodeDepthMat::EncodeDepthMat()
	{
		mParamBuffer = gEncodeDepthParamDef.createBuffer();

		mParams->setParamBlockBuffer("Params", mParamBuffer);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);

		SAMPLER_STATE_DESC sampDesc;
		sampDesc.minFilter = FO_POINT;
		sampDesc.magFilter = FO_POINT;
		sampDesc.mipFilter = FO_POINT;
		sampDesc.addressMode.u = TAM_CLAMP;
		sampDesc.addressMode.v = TAM_CLAMP;
		sampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> samplerState = SamplerState::create(sampDesc);
		setSamplerState(mParams, GPT_FRAGMENT_PROGRAM, "gInputSamp", "gInputTex", samplerState);
	}

	void EncodeDepthMat::execute(const SPtr<Texture>& depth, float near, float far, const SPtr<RenderTarget>& output)
	{
		BS_RENMAT_PROFILE_BLOCK

		mInputTexture.set(depth);

		gEncodeDepthParamDef.gNear.set(mParamBuffer, near);
		gEncodeDepthParamDef.gFar.set(mParamBuffer, far);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, 0, RT_COLOR0);

		bind();
		gRendererUtility().drawScreenQuad();
	}

	MSAACoverageMat::MSAACoverageMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{ }

	void MSAACoverageMat::execute(const RendererView& view, GBufferTextures gbuffer)
	{
		BS_RENMAT_PROFILE_BLOCK

		mGBufferParams.bind(gbuffer);

		const Rect2I& viewRect = view.getProperties().target.viewRect;
		SPtr<GpuParamBlockBuffer> perView = view.getPerViewBuffer();
		mParams->setParamBlockBuffer("PerCamera", perView);

		bind();
		gRendererUtility().drawScreenQuad(Rect2(0, 0, (float)viewRect.width, (float)viewRect.height));
	}

	MSAACoverageMat* MSAACoverageMat::getVariation(UINT32 msaaCount)
	{
		switch(msaaCount)
		{
		case 2:
			return get(getVariation<2>());
		case 4:
			return get(getVariation<4>());
		case 8:
		default:
			return get(getVariation<8>());
		}
	}

	MSAACoverageStencilMat::MSAACoverageStencilMat()
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gMSAACoverage", mCoverageTexParam);
	}

	void MSAACoverageStencilMat::execute(const RendererView& view, const SPtr<Texture>& coverage)
	{
		BS_RENMAT_PROFILE_BLOCK

		const Rect2I& viewRect = view.getProperties().target.viewRect;
		mCoverageTexParam.set(coverage);

		bind();
		gRendererUtility().drawScreenQuad(Rect2(0, 0, (float)viewRect.width, (float)viewRect.height));
	}
}}
