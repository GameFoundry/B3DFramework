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
#include "BsRenderTargets.h"
#include "BsPixelUtil.h"
#include "BsBitwise.h"

namespace bs { namespace ct
{
	DownsampleParamDef gDownsampleParamDef;

	template<int Quality, bool MSAA>
	DownsampleMat<Quality, MSAA>::DownsampleMat()
	{
		mParamBuffer = gDownsampleParamDef.createBuffer();

		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		if(gpuParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Input"))
			mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	template<int Quality, bool MSAA>
	void DownsampleMat<Quality, MSAA>::_initDefines(ShaderDefines& defines)
	{
		defines.set("QUALITY", Quality);
		defines.set("MSAA", MSAA ? 1 : 0);
	}

	template<int Quality, bool MSAA>
	void DownsampleMat<Quality, MSAA>::execute(const SPtr<Texture>& target, PostProcessInfo& ppInfo)
	{
		// Set parameters
		mInputTexture.set(target);

		const TextureProperties& rtProps = target->getProperties();

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

		// Set output
		UINT32 width = std::max(1, Math::ceilToInt(rtProps.getWidth() * 0.5f));
		UINT32 height = std::max(1, Math::ceilToInt(rtProps.getHeight() * 0.5f));

		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(rtProps.getFormat(), width, height, TU_RENDERTARGET);

		// Render
		ppInfo.downsampledSceneTex = GpuResourcePool::instance().get(mOutputDesc);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(ppInfo.downsampledSceneTex->renderTexture, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		if (MSAA)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)rtProps.getWidth(), (float)rtProps.getHeight()));
		else
			gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);

		mOutput = ppInfo.downsampledSceneTex->renderTexture;
	}

	template<int Quality, bool MSAA>
	void DownsampleMat<Quality, MSAA>::release(PostProcessInfo& ppInfo)
	{
		GpuResourcePool::instance().release(ppInfo.downsampledSceneTex);
		mOutput = nullptr;
	}

	void DownsampleMaterials::execute(UINT32 quality, bool msaa, const SPtr<Texture>& target, PostProcessInfo& ppInfo)
	{
		if(quality == 0)
		{
			if(msaa)
				m0_MSAA.execute(target, ppInfo);
			else
				m0_NoMSAA.execute(target, ppInfo);
		}
		else
		{
			if (msaa)
				m1_MSAA.execute(target, ppInfo);
			else
				m1_NoMSAA.execute(target, ppInfo);
		}
	}

	void DownsampleMaterials::release(UINT32 quality, bool msaa, PostProcessInfo& ppInfo)
	{
		if(quality == 0)
		{
			if(msaa)
				m0_MSAA.release(ppInfo);
			else
				m0_NoMSAA.release(ppInfo);
		}
		else
		{
			if (msaa)
				m1_MSAA.release(ppInfo);
			else
				m1_NoMSAA.release(ppInfo);
		}
	}

	template class DownsampleMat<0, false>;
	template class DownsampleMat<0, true>;
	template class DownsampleMat<1, false>;
	template class DownsampleMat<1, true>;

	EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

	EyeAdaptHistogramMat::EyeAdaptHistogramMat()
	{
		mParamBuffer = gEyeAdaptHistogramParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_COMPUTE_PROGRAM, "gSceneColorTex", mSceneColor);
		params->getLoadStoreTextureParam(GPT_COMPUTE_PROGRAM, "gOutputTex", mOutputTex);
	}

	void EyeAdaptHistogramMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE_X", THREAD_GROUP_SIZE_X);
		defines.set("THREADGROUP_SIZE_Y", THREAD_GROUP_SIZE_Y);
		defines.set("LOOP_COUNT_X", LOOP_COUNT_X);
		defines.set("LOOP_COUNT_Y", LOOP_COUNT_Y);
	}

	void EyeAdaptHistogramMat::execute(PostProcessInfo& ppInfo)
	{
		// Set parameters
		SPtr<RenderTexture> target = ppInfo.downsampledSceneTex->renderTexture;
		mSceneColor.set(ppInfo.downsampledSceneTex->texture);

		const RenderTextureProperties& props = target->getProperties();
		int offsetAndSize[4] = { 0, 0, (INT32)props.getWidth(), (INT32)props.getHeight() };

		gEyeAdaptHistogramParamDef.gHistogramParams.set(mParamBuffer, getHistogramScaleOffset(ppInfo));
		gEyeAdaptHistogramParamDef.gPixelOffsetAndSize.set(mParamBuffer, Vector4I(offsetAndSize));

		Vector2I threadGroupCount = getThreadGroupCount(target);
		gEyeAdaptHistogramParamDef.gThreadGroupCount.set(mParamBuffer, threadGroupCount);

		// Set output
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, HISTOGRAM_NUM_TEXELS, numHistograms,
			TU_LOADSTORE);

		// Dispatch
		ppInfo.histogramTex = GpuResourcePool::instance().get(mOutputDesc);

		mOutputTex.set(ppInfo.histogramTex->texture);

		RenderAPI& rapi = RenderAPI::instance();
		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(threadGroupCount.x, threadGroupCount.y);

		mOutput = ppInfo.histogramTex->renderTexture;
	}

	void EyeAdaptHistogramMat::release(PostProcessInfo& ppInfo)
	{
		GpuResourcePool::instance().release(ppInfo.histogramTex);
		mOutput = nullptr;
	}

	Vector2I EyeAdaptHistogramMat::getThreadGroupCount(const SPtr<RenderTexture>& target)
	{
		const UINT32 texelsPerThreadGroupX = THREAD_GROUP_SIZE_X * LOOP_COUNT_X;
		const UINT32 texelsPerThreadGroupY = THREAD_GROUP_SIZE_Y * LOOP_COUNT_Y;

		const RenderTextureProperties& props = target->getProperties();
	
		Vector2I threadGroupCount;
		threadGroupCount.x = ((INT32)props.getWidth() + texelsPerThreadGroupX - 1) / texelsPerThreadGroupX;
		threadGroupCount.y = ((INT32)props.getHeight() + texelsPerThreadGroupY - 1) / texelsPerThreadGroupY;

		return threadGroupCount;
	}

	Vector2 EyeAdaptHistogramMat::getHistogramScaleOffset(const PostProcessInfo& ppInfo)
	{
		const StandardPostProcessSettings& settings = *ppInfo.settings;

		float diff = settings.autoExposure.histogramLog2Max - settings.autoExposure.histogramLog2Min;
		float scale = 1.0f / diff;
		float offset = -settings.autoExposure.histogramLog2Min * scale;

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

	void EyeAdaptHistogramReduceMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void EyeAdaptHistogramReduceMat::execute(PostProcessInfo& ppInfo)
	{
		// Set parameters
		mHistogramTex.set(ppInfo.histogramTex->texture);

		SPtr<PooledRenderTexture> eyeAdaptationRT = ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex];
		SPtr<Texture> eyeAdaptationTex;

		if (eyeAdaptationRT != nullptr) // Could be that this is the first run
			eyeAdaptationTex = eyeAdaptationRT->texture;
		else
			eyeAdaptationTex = Texture::WHITE;

		mEyeAdaptationTex.set(eyeAdaptationTex);

		Vector2I threadGroupCount = EyeAdaptHistogramMat::getThreadGroupCount(ppInfo.downsampledSceneTex->renderTexture);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		gEyeAdaptHistogramReduceParamDef.gThreadGroupCount.set(mParamBuffer, numHistograms);

		// Set output
		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2,
			TU_RENDERTARGET);

		// Render
		ppInfo.histogramReduceTex = GpuResourcePool::instance().get(mOutputDesc);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(ppInfo.histogramReduceTex->renderTexture, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2.0f);
		gRendererUtility().drawScreenQuad(drawUV);

		rapi.setRenderTarget(nullptr);

		mOutput = ppInfo.histogramReduceTex->renderTexture;
	}

	void EyeAdaptHistogramReduceMat::release(PostProcessInfo& ppInfo)
	{
		GpuResourcePool::instance().release(ppInfo.histogramReduceTex);
		mOutput = nullptr;
	}

	EyeAdaptationParamDef gEyeAdaptationParamDef;

	EyeAdaptationMat::EyeAdaptationMat()
	{
		mParamBuffer = gEyeAdaptationParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gHistogramTex", mReducedHistogramTex);
	}

	void EyeAdaptationMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("THREADGROUP_SIZE_X", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_X);
		defines.set("THREADGROUP_SIZE_Y", EyeAdaptHistogramMat::THREAD_GROUP_SIZE_Y);
	}

	void EyeAdaptationMat::execute(PostProcessInfo& ppInfo, float frameDelta)
	{
		bool texturesInitialized = ppInfo.eyeAdaptationTex[0] != nullptr && ppInfo.eyeAdaptationTex[1] != nullptr;
		if(!texturesInitialized)
		{
			POOLED_RENDER_TEXTURE_DESC outputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT32_R, 1, 1, TU_RENDERTARGET);
			ppInfo.eyeAdaptationTex[0] = GpuResourcePool::instance().get(outputDesc);
			ppInfo.eyeAdaptationTex[1] = GpuResourcePool::instance().get(outputDesc);
		}

		ppInfo.lastEyeAdaptationTex = (ppInfo.lastEyeAdaptationTex + 1) % 2; // TODO - Do I really need two targets?

		// Set parameters
		mReducedHistogramTex.set(ppInfo.histogramReduceTex->texture);

		Vector2 histogramScaleAndOffset = EyeAdaptHistogramMat::getHistogramScaleOffset(ppInfo);

		const StandardPostProcessSettings& settings = *ppInfo.settings;

		Vector4 eyeAdaptationParams[3];
		eyeAdaptationParams[0].x = histogramScaleAndOffset.x;
		eyeAdaptationParams[0].y = histogramScaleAndOffset.y;

		float histogramPctHigh = Math::clamp01(settings.autoExposure.histogramPctHigh);

		eyeAdaptationParams[0].z = std::min(Math::clamp01(settings.autoExposure.histogramPctLow), histogramPctHigh);
		eyeAdaptationParams[0].w = histogramPctHigh;

		eyeAdaptationParams[1].x = std::min(settings.autoExposure.minEyeAdaptation, settings.autoExposure.maxEyeAdaptation);
		eyeAdaptationParams[1].y = settings.autoExposure.maxEyeAdaptation;

		eyeAdaptationParams[1].z = settings.autoExposure.eyeAdaptationSpeedUp;
		eyeAdaptationParams[1].w = settings.autoExposure.eyeAdaptationSpeedDown;

		eyeAdaptationParams[2].x = Math::pow(2.0f, settings.exposureScale);
		eyeAdaptationParams[2].y = frameDelta;

		eyeAdaptationParams[2].z = 0.0f; // Unused
		eyeAdaptationParams[2].w = 0.0f; // Unused

		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[0], 0);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[1], 1);
		gEyeAdaptationParamDef.gEyeAdaptationParams.set(mParamBuffer, eyeAdaptationParams[2], 2);

		// Render
		SPtr<PooledRenderTexture> eyeAdaptationRT = ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex];

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(eyeAdaptationRT->renderTexture, FBT_DEPTH | FBT_STENCIL);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
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

	void CreateTonemapLUTMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("LUT_SIZE", LUT_SIZE);
	}

	void CreateTonemapLUTMat::execute(PostProcessInfo& ppInfo)
	{
		const StandardPostProcessSettings& settings = *ppInfo.settings;

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

		// Set output
		POOLED_RENDER_TEXTURE_DESC outputDesc = POOLED_RENDER_TEXTURE_DESC::create3D(PF_R8G8B8A8, 
			LUT_SIZE, LUT_SIZE, LUT_SIZE, TU_LOADSTORE);

		// Dispatch
		ppInfo.colorLUT = GpuResourcePool::instance().get(outputDesc);

		mOutputTex.set(ppInfo.colorLUT->texture);

		RenderAPI& rapi = RenderAPI::instance();
		
		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(LUT_SIZE / 8, LUT_SIZE / 8, LUT_SIZE);
	}

	void CreateTonemapLUTMat::release(PostProcessInfo& ppInfo)
	{
		GpuResourcePool::instance().release(ppInfo.colorLUT);
	}

	TonemappingParamDef gTonemappingParamDef;

	template<bool GammaOnly, bool AutoExposure, bool MSAA>
	TonemappingMat<GammaOnly, AutoExposure, MSAA>::TonemappingMat()
	{
		mParamBuffer = gTonemappingParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);

		SPtr<GpuParams> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);

		if(!GammaOnly)
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorLUT", mColorLUT);
	}

	template<bool GammaOnly, bool AutoExposure, bool MSAA>
	void TonemappingMat<GammaOnly, AutoExposure, MSAA>::_initDefines(ShaderDefines& defines)
	{
		if(GammaOnly)
			defines.set("GAMMA_ONLY", 1);

		if (AutoExposure)
			defines.set("AUTO_EXPOSURE", 1);

		defines.set("MSAA", MSAA ? 1 : 0);
		defines.set("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE);
	}

	template<bool GammaOnly, bool AutoExposure, bool MSAA>
	void TonemappingMat<GammaOnly, AutoExposure, MSAA>::execute(const SPtr<Texture>& sceneColor, 
		const SPtr<RenderTarget>& outputRT, const Rect2& outputRect, PostProcessInfo& ppInfo)
	{
		const TextureProperties& texProps = sceneColor->getProperties();

		gTonemappingParamDef.gRawGamma.set(mParamBuffer, 1.0f / ppInfo.settings->gamma);
		gTonemappingParamDef.gManualExposureScale.set(mParamBuffer, Math::pow(2.0f, ppInfo.settings->exposureScale));
		gTonemappingParamDef.gNumSamples.set(mParamBuffer, texProps.getNumSamples());

		// Set parameters
		mInputTex.set(sceneColor);

		SPtr<Texture> colorLUT;
		if(ppInfo.colorLUT != nullptr)
			colorLUT = ppInfo.colorLUT->texture;

		mColorLUT.set(colorLUT);

		SPtr<Texture> eyeAdaptationTexture;
		if(ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex] != nullptr)
			eyeAdaptationTexture = ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex]->texture;

		mEyeAdaptationTex.set(eyeAdaptationTexture);

		// Render
		RenderAPI& rapi = RenderAPI::instance();

		rapi.setRenderTarget(outputRT);
		rapi.setViewport(outputRect);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		if (MSAA)
			gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)texProps.getWidth(), (float)texProps.getHeight()));
		else
			gRendererUtility().drawScreenQuad();
	}

	template class TonemappingMat<false, false, false>;
	template class TonemappingMat<false, false, true>;
	template class TonemappingMat<false, true, false>;
	template class TonemappingMat<false, true, true>;
	template class TonemappingMat<true, false, false>;
	template class TonemappingMat<true, false, true>;
	template class TonemappingMat<true, true, false>;
	template class TonemappingMat<true, true, true>;

	void TonemappingMaterials::execute(bool gammaOnly, bool autoExposure, bool MSAA, const SPtr<Texture>& sceneColor, 
		const SPtr<RenderTarget>& outputRT, const Rect2& outputRect, PostProcessInfo& ppInfo)
	{
		if (gammaOnly)
		{
			if (autoExposure)
			{
				if (MSAA)
					mTTT.execute(sceneColor, outputRT, outputRect, ppInfo);
				else
					mTTF.execute(sceneColor, outputRT, outputRect, ppInfo);
			}
			else
			{
				if (MSAA)
					mTFT.execute(sceneColor, outputRT, outputRect, ppInfo);
				else
					mTFF.execute(sceneColor, outputRT, outputRect, ppInfo);
			}
		}
		else
		{
			if (autoExposure)
			{
				if (MSAA)
					mFTT.execute(sceneColor, outputRT, outputRect, ppInfo);
				else
					mFTF.execute(sceneColor, outputRT, outputRect, ppInfo);
			}
			else
			{
				if (MSAA)
					mFFT.execute(sceneColor, outputRT, outputRect, ppInfo);
				else
					mFFF.execute(sceneColor, outputRT, outputRect, ppInfo);
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

	void GaussianBlurMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("MAX_NUM_SAMPLES", MAX_BLUR_SAMPLES);
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

	template<bool Near, bool Far>
	GaussianDOFSeparateMat<Near, Far>::GaussianDOFSeparateMat()
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

	template<bool Near, bool Far>
	void GaussianDOFSeparateMat<Near, Far>::_initDefines(ShaderDefines& defines)
	{
		defines.set("NEAR", Near ? 1 : 0);
		defines.set("FAR", Far ? 1 : 0);
		defines.set("NEAR_AND_FAR", (Near && Far) ? 1 : 0);
	}

	template<bool Near, bool Far>
	void GaussianDOFSeparateMat<Near, Far>::execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, 
		const RendererView& view, const DepthOfFieldSettings& settings)
	{
		const TextureProperties& srcProps = color->getProperties();

		UINT32 outputWidth = std::max(1U, srcProps.getWidth() / 2);
		UINT32 outputHeight = std::max(1U, srcProps.getHeight() / 2);

		POOLED_RENDER_TEXTURE_DESC outputTexDesc = POOLED_RENDER_TEXTURE_DESC::create2D(srcProps.getFormat(), 
			outputWidth, outputHeight, TU_RENDERTARGET);
		mOutput0 = GpuResourcePool::instance().get(outputTexDesc);

		SPtr<RenderTexture> rt;
		if (Near && Far)
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

	template <bool Near, bool Far>
	SPtr<PooledRenderTexture> GaussianDOFSeparateMat<Near, Far>::getOutput(UINT32 idx)
	{
		if (idx == 0)
			return mOutput0;
		else if (idx == 1)
			return mOutput1;

		return nullptr;
	}

	template<bool Near, bool Far>
	void GaussianDOFSeparateMat<Near, Far>::release()
	{
		if (mOutput0 != nullptr)
			GpuResourcePool::instance().release(mOutput0);

		if (mOutput1 != nullptr)
			GpuResourcePool::instance().release(mOutput1);
	}

	template class GaussianDOFSeparateMat<true, true>;
	template class GaussianDOFSeparateMat<true, false>;
	template class GaussianDOFSeparateMat<false, true>;

	template<bool Near, bool Far>
	GaussianDOFCombineMat<Near, Far>::GaussianDOFCombineMat()
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

	template<bool Near, bool Far>
	void GaussianDOFCombineMat<Near, Far>::_initDefines(ShaderDefines& defines)
	{
		defines.set("NEAR", Near ? 1 : 0);
		defines.set("FAR", Far ? 1 : 0);
		defines.set("NEAR_AND_FAR", (Near && Far) ? 1 : 0);
	}

	template<bool Near, bool Far>
	void GaussianDOFCombineMat<Near, Far>::execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, 
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

	template class GaussianDOFCombineMat<true, true>;
	template class GaussianDOFCombineMat<true, false>;
	template class GaussianDOFCombineMat<false, true>;

	void GaussianDOF::execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& sceneDepth, 
		const SPtr<RenderTarget>& output, const RendererView& view, const DepthOfFieldSettings& settings)
	{
		bool near = settings.nearBlurAmount > 0.0f;
		bool far = settings.farBlurAmount > 0.0f;

		// This shouldn't have been called if both are false
		assert(near || far);

		IGaussianDOFSeparateMat* separateMat;
		IGaussianDOFCombineMat* combineMat;

		if (near && far)
		{
			separateMat = &mSeparateNF;
			combineMat = &mCombineNF;
		}
		else
		{
			if(near)
			{
				separateMat = &mSeparateN;
				combineMat = &mCombineN;
			}
			else
			{
				separateMat = &mSeparateF;
				combineMat = &mCombineF;
			}
		}

		separateMat->execute(sceneColor, sceneDepth, view, settings);

		SPtr<PooledRenderTexture> nearTex, farTex;
		if(near && far)
		{
			nearTex = separateMat->getOutput(0);
			farTex = separateMat->getOutput(1);
		}
		else
		{
			if (near)
				nearTex = separateMat->getOutput(0);
			else
				farTex = separateMat->getOutput(0);
		}

		// Blur the out of focus pixels
		// Note: Perhaps set up stencil so I can avoid performing blur on unused parts of the textures?
		const TextureProperties& texProps = nearTex ? nearTex->texture->getProperties() : farTex->texture->getProperties();
		POOLED_RENDER_TEXTURE_DESC tempTexDesc = POOLED_RENDER_TEXTURE_DESC::create2D(texProps.getFormat(), 
			texProps.getWidth(), texProps.getHeight(), TU_RENDERTARGET);
		SPtr<PooledRenderTexture> tempTexture = GpuResourcePool::instance().get(tempTexDesc);

		SPtr<Texture> blurredNearTex;
		if(nearTex)
		{
			mBlur.execute(nearTex->texture, settings.nearBlurAmount, tempTexture->renderTexture);
			blurredNearTex = tempTexture->texture;
		}

		SPtr<Texture> blurredFarTex;
		if(farTex)
		{
			// If temporary texture is used up, re-use the original near texture for the blurred result
			if(blurredNearTex)
			{
				mBlur.execute(farTex->texture, settings.farBlurAmount, nearTex->renderTexture);
				blurredFarTex = nearTex->texture;
			}
			else // Otherwise just use the temporary
			{
				mBlur.execute(farTex->texture, settings.farBlurAmount, tempTexture->renderTexture);
				blurredFarTex = tempTexture->texture;
			}
		}
		
		combineMat->execute(sceneColor, blurredNearTex, blurredFarTex, sceneDepth, output, view, settings);

		separateMat->release();
		GpuResourcePool::instance().release(tempTexture);
	}

	bool GaussianDOF::requiresDOF(const DepthOfFieldSettings& settings)
	{
		bool near = settings.nearBlurAmount > 0.0f;
		bool far = settings.farBlurAmount > 0.0f;

		return settings.enabled && (near || far);
	}
	
	BuildHiZMat::BuildHiZMat()
	{
		SPtr<GpuParams> gpuParams = mParamsSet->getGpuParams();
		gpuParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthTex", mInputTexture);
	}

	void BuildHiZMat::_initDefines(ShaderDefines& defines)
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

	void BuildHiZ::execute(const RendererViewTargetData& viewInfo, const SPtr<Texture>& source, const SPtr<Texture>& output)
	{
		Rect2 srcRect = viewInfo.nrmViewRect;

		// If viewport size is odd, adjust UV
		srcRect.width += (viewInfo.viewRect.width % 2) * (1.0f / viewInfo.viewRect.width);
		srcRect.height += (viewInfo.viewRect.height % 2) * (1.0f / viewInfo.viewRect.height);

		// Generate first mip
		RENDER_TEXTURE_DESC rtDesc;
		rtDesc.colorSurfaces[0].texture = output;
		rtDesc.colorSurfaces[0].mipLevel = 0;

		SPtr<RenderTexture> rt = RenderTexture::create(rtDesc);
		const TextureProperties& outProps = output->getProperties();

		Rect2 destRect;
		bool downsampledFirstMip = false; // Not used currently
		if (downsampledFirstMip)
		{
			// Make sure that 1 pixel in HiZ maps to a 2x2 block in source
			destRect = Rect2(0, 0,
				Math::ceilToInt(viewInfo.viewRect.width / 2.0f) / (float)outProps.getWidth(),
				Math::ceilToInt(viewInfo.viewRect.height / 2.0f) / (float)outProps.getHeight());

			mHiZMat.execute(source, 0, srcRect, destRect, rt);
		}
		else // First level is just a copy of the depth buffer
		{
			destRect = Rect2(0, 0,
				viewInfo.viewRect.width / (float)outProps.getWidth(),
				viewInfo.viewRect.height / (float)outProps.getHeight());

			RenderAPI& rapi = RenderAPI::instance();
			rapi.setRenderTarget(rt);
			rapi.setViewport(destRect);

			Rect2I srcAreaInt;
			srcAreaInt.x = (INT32)(srcRect.x * viewInfo.viewRect.width);
			srcAreaInt.y = (INT32)(srcRect.y * viewInfo.viewRect.height);
			srcAreaInt.width = (UINT32)(srcRect.width * viewInfo.viewRect.width);
			srcAreaInt.height = (UINT32)(srcRect.height * viewInfo.viewRect.height);

			gRendererUtility().blit(source, srcAreaInt);			
			rapi.setViewport(Rect2(0, 0, 1, 1));
		}

		// Generate remaining mip levels
		for(UINT32 i = 1; i <= outProps.getNumMipmaps(); i++)
		{
			rtDesc.colorSurfaces[0].mipLevel = i;

			rt = RenderTexture::create(rtDesc);
			mHiZMat.execute(output, i - 1, destRect, destRect, rt);
		}
	}

	POOLED_RENDER_TEXTURE_DESC BuildHiZ::getHiZTextureDesc(UINT32 viewWidth, UINT32 viewHeight)
	{
		UINT32 size = Bitwise::nextPow2(std::max(viewWidth, viewHeight));
		UINT32 numMips = PixelUtil::getMaxMipmaps(size, size, 1, PF_FLOAT32_R);
		size = 1 << numMips;

		// Note: Use the 32-bit buffer here as 16-bit causes too much banding (most of the scene gets assigned 4-5 different
		// depth values). 
		//  - When I add UNORM 16-bit format I should be able to switch to that
		return POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT32_R, size, size, TU_RENDERTARGET, 1, false, 1, numMips);
	}

	FXAAParamDef gFXAAParamDef;

	FXAAMat::FXAAMat()
	{
		mParamBuffer = gFXAAParamDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void FXAAMat::_initDefines(ShaderDefines& defines)
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

	template<bool UPSAMPLE, bool FINAL_PASS, int QUALITY>
	SSAOMat<UPSAMPLE, FINAL_PASS, QUALITY>::SSAOMat()
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

	template<bool UPSAMPLE, bool FINAL_PASS, int QUALITY>
	void SSAOMat<UPSAMPLE, FINAL_PASS, QUALITY>::_initDefines(ShaderDefines& defines)
	{
		defines.set("MIX_WITH_UPSAMPLED", UPSAMPLE ? 1 : 0);
		defines.set("FINAL_AO", FINAL_PASS ? 1 : 0);
		defines.set("QUALITY", QUALITY);
	}

	template <bool UPSAMPLE, bool FINAL_PASS, int QUALITY>
	void SSAOMat<UPSAMPLE, FINAL_PASS, QUALITY>::execute(const RendererView& view, const SSAOTextureInputs& textures, 
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
		
		if(UPSAMPLE)
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

		if (FINAL_PASS)
		{
			mDepthTexture.set(textures.sceneDepth);
			mNormalsTexture.set(textures.sceneNormals);
		}

		if (UPSAMPLE)
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

	void SSAODownsampleMat::_initDefines(ShaderDefines& defines)
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

	template<bool HORIZONTAL>
	SSAOBlurMat<HORIZONTAL>::SSAOBlurMat()
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

	template<bool HORIZONTAL>
	void SSAOBlurMat<HORIZONTAL>::_initDefines(ShaderDefines& defines)
	{
		defines.set("DIR_HORZ", HORIZONTAL ? 1 : 0);
	}

	template<bool HORIZONTAL>
	void SSAOBlurMat<HORIZONTAL>::execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& depth, 
		const SPtr<RenderTexture>& destination, float depthRange)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		const TextureProperties& texProps = ao->getProperties();

		Vector2 pixelSize;
		pixelSize.x = 1.0f / texProps.getWidth();
		pixelSize.y = 1.0f / texProps.getHeight();

		Vector2 pixelOffset(BsZero);
		if (HORIZONTAL)
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

	SSAO::SSAO()
	{
		mSSAORandomizationTex = generate4x4RandomizationTexture();
	}

	void SSAO::execute(const RendererView& view, const SPtr<RenderTexture>& destination, 
		const AmbientOcclusionSettings& settings)
	{
		/** Maximum valid depth range within samples in a sample set. In meters. */
		static const float DEPTH_RANGE = 1.0f;

		const RendererViewProperties& viewProps = view.getProperties();
		SPtr<RenderTargets> renderTargets = view.getRenderTargets();

		SPtr<Texture> sceneDepth = renderTargets->get(RTT_ResolvedDepth);
		SPtr<Texture> sceneNormals = renderTargets->get(RTT_GBuffer, RT_COLOR1);

		const TextureProperties& normalsProps = sceneNormals->getProperties();
		SPtr<PooledRenderTexture> resolvedNormals;
		if(sceneNormals->getProperties().getNumSamples() > 1)
		{
			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(normalsProps.getFormat(), 
				normalsProps.getWidth(), normalsProps.getHeight(), TU_RENDERTARGET);
			resolvedNormals = GpuResourcePool::instance().get(desc);

			RenderAPI::instance().setRenderTarget(resolvedNormals->renderTexture);
			gRendererUtility().blit(sceneNormals);

			sceneNormals = resolvedNormals->texture;
		}

		// Multiple downsampled AO levels are used to minimize cache trashing. Downsampled AO targets use larger radius,
		// whose contents are then blended with the higher level.

		UINT32 quality = settings.quality;
		UINT32 numDownsampleLevels = 0;
		if (quality > 1)
			numDownsampleLevels = 1;
		else if (quality > 2)
			numDownsampleLevels = 2;

		SPtr<PooledRenderTexture> setupTex0;
		if(numDownsampleLevels > 0)
		{
			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 2)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			setupTex0 = GpuResourcePool::instance().get(desc);

			mDownsample.execute(view, sceneDepth, sceneNormals, setupTex0->renderTexture, DEPTH_RANGE);
		}

		SPtr<PooledRenderTexture> setupTex1;
		if(numDownsampleLevels > 1)
		{
			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 4)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			setupTex1 = GpuResourcePool::instance().get(desc);

			mDownsample.execute(view, sceneDepth, sceneNormals, setupTex1->renderTexture, DEPTH_RANGE);
		}

		SSAOTextureInputs textures;
		textures.sceneDepth = sceneDepth;
		textures.sceneNormals = sceneNormals;
		textures.randomRotations = mSSAORandomizationTex;

		SPtr<PooledRenderTexture> downAOTex1;
		if(numDownsampleLevels > 1)
		{
			textures.aoSetup = setupTex1->texture;

			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 4)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 4))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex1 = GpuResourcePool::instance().get(desc);

			executeSSAOMat(false, false, quality, view, textures, downAOTex1->renderTexture, settings);

			GpuResourcePool::instance().release(setupTex1);
			setupTex1 = nullptr;
		}

		SPtr<PooledRenderTexture> downAOTex0;
		if(numDownsampleLevels > 0)
		{
			textures.aoSetup = setupTex0->texture;

			if(downAOTex1)
				textures.aoDownsampled = downAOTex1->texture;

			Vector2I downsampledSize(
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.width, 2)),
				std::max(1, Math::divideAndRoundUp((INT32)viewProps.viewRect.height, 2))
			);

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, downsampledSize.x, 
				downsampledSize.y, TU_RENDERTARGET);
			downAOTex0 = GpuResourcePool::instance().get(desc);

			bool upsample = numDownsampleLevels > 1;
			executeSSAOMat(upsample, false, quality, view, textures, downAOTex0->renderTexture, settings);

			if(upsample)
			{
				GpuResourcePool::instance().release(downAOTex1);
				downAOTex1 = nullptr;
			}
		}

		{
			if(setupTex0)
				textures.aoSetup = setupTex0->texture;

			if(downAOTex0)
				textures.aoDownsampled = downAOTex0->texture;

			bool upsample = numDownsampleLevels > 0;
			executeSSAOMat(upsample, true, quality, view, textures, destination, settings);
		}

		if(resolvedNormals)
		{
			GpuResourcePool::instance().release(resolvedNormals);
			resolvedNormals = nullptr;
		}

		if(numDownsampleLevels > 0)
		{
			GpuResourcePool::instance().release(setupTex0);
			GpuResourcePool::instance().release(downAOTex0);
		}

		// Blur the output
		// Note: If I implement temporal AA then this can probably be avoided. I can instead jitter the sample offsets
		// each frame, and averaging them out should yield blurred AO.
		if(quality > 1) // On level 0 we don't blur at all, on level 1 we use the ad-hoc blur in shader
		{
			const RenderTargetProperties& rtProps = destination->getProperties();

			POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8, rtProps.getWidth(), 
				rtProps.getHeight(), TU_RENDERTARGET);
			SPtr<PooledRenderTexture> blurIntermediateTex = GpuResourcePool::instance().get(desc);

			mBlurHorz.execute(view, destination->getColorTexture(0), sceneDepth, blurIntermediateTex->renderTexture, 
				DEPTH_RANGE);
			mBlurVert.execute(view, blurIntermediateTex->texture, sceneDepth, destination, DEPTH_RANGE);

			GpuResourcePool::instance().release(blurIntermediateTex);
		}
	}

	void SSAO::executeSSAOMat(bool upsample, bool final, int quality, const RendererView& view, 
		const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings)
	{
#define PICK_MATERIAL(QUALITY)															\
		if(upsample)																	\
			if(final)																	\
				mSSAO_TT_##QUALITY.execute(view, textures, destination, settings);		\
			else																		\
				mSSAO_TF_##QUALITY.execute(view, textures, destination, settings);		\
		else																			\
			if(final)																	\
				mSSAO_FT_##QUALITY.execute(view, textures, destination, settings);		\
			else																		\
				mSSAO_FF_##QUALITY.execute(view, textures, destination, settings);		\

		switch(quality)
		{
		case 0:
			PICK_MATERIAL(0)
			break;
		case 1:
			PICK_MATERIAL(1)
			break;
		case 2:
			PICK_MATERIAL(2)
			break;
		case 3:
			PICK_MATERIAL(3)
			break;
		default:
		case 4:
			PICK_MATERIAL(4)
			break;
		}

#undef PICK_MATERIAL
	}

	SPtr<Texture> SSAO::generate4x4RandomizationTexture() const
	{
		UINT32 mapping[16] = { 13, 5, 1, 9, 14, 3, 7, 11, 15, 2, 6, 12, 4, 8, 0, 10 };
		Vector2 bases[16];
		for (UINT32 i = 0; i < 16; ++i)
		{
			float angle = (mapping[i] / 16.0f) * Math::PI;
			bases[i].x = cos(angle);
			bases[i].y = sin(angle);
		}

		SPtr<PixelData> pixelData = PixelData::create(4, 4, 1, PF_R8G8);
		for(UINT32 y = 0; y < 4; ++y)
			for(UINT32 x = 0; x < 4; ++x)
			{
				UINT32 base = (y * 4) + x;

				Color color;
				color.r = bases[base].x * 0.5f + 0.5f;
				color.g = bases[base].y * 0.5f + 0.5f;

				pixelData->setColorAt(color, x, y);
			}

		return Texture::create(pixelData);
	}

	SSRStencilParamDef gSSRStencilParamDef;

	SSRStencilMat::SSRStencilMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		mParamBuffer = gSSRStencilParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
	}

	void SSRStencilMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void SSRStencilMat::execute(const RendererView& view, const ScreenSpaceReflectionsSettings& settings)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		RenderTargets& renderTargets = *view.getRenderTargets();

		mGBufferParams.bind(renderTargets);

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

	void SSRTraceMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void SSRTraceMat::execute(const RendererView& view, const ScreenSpaceReflectionsSettings& settings, 
		const SPtr<RenderTarget>& destination)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		RenderTargets& renderTargets = *view.getRenderTargets();

		SPtr<Texture> hiZ = renderTargets.get(RTT_HiZ);
		const TextureProperties& hiZProps = hiZ->getProperties();

		mGBufferParams.bind(renderTargets);
		mSceneColorTexture.set(renderTargets.get(RTT_ResolvedSceneColor));
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

		Vector2I bufferSize(viewRect.width, viewRect.height);
		gSSRTraceParamDef.gHiZSize.set(mParamBuffer, bufferSize);
		gSSRTraceParamDef.gHiZNumMips.set(mParamBuffer, hiZProps.getNumMipmaps());
		gSSRTraceParamDef.gNDCToHiZUV.set(mParamBuffer, ndcToHiZUV);
		gSSRTraceParamDef.gHiZUVToScreenUV.set(mParamBuffer, HiZUVToScreenUV);

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

	template<bool EyeAdaptation>
	SSRResolveMat<EyeAdaptation>::SSRResolveMat()
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

	template<bool EyeAdaptation>
	void SSRResolveMat<EyeAdaptation>::_initDefines(ShaderDefines& defines)
	{
		defines.set("EYE_ADAPTATION", EyeAdaptation ? 1 : 0);
		defines.set("MSAA", 0);
	}

	template <bool EyeAdaptation>
	void SSRResolveMat<EyeAdaptation>::execute(const RendererView& view, const SPtr<Texture>& prevFrame, 
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

		if(EyeAdaptation)
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

	void PostProcessing::postProcess(RendererView* viewInfo, const SPtr<RenderTargets>& renderTargets, float frameDelta)
	{
		auto& viewProps = viewInfo->getProperties();

		PostProcessInfo& ppInfo = viewInfo->getPPInfo();
		const StandardPostProcessSettings& settings = *ppInfo.settings;

		SPtr<Texture> sceneColor = renderTargets->get(RTT_SceneColor);
		Rect2 viewportRect = viewProps.nrmViewRect;

		bool hdr = viewProps.isHDR;
		bool msaa = viewProps.numSamples > 1;

		if(hdr && settings.enableAutoExposure)
		{
			mDownsample.execute(1, msaa, sceneColor, ppInfo);
			mEyeAdaptHistogram.execute(ppInfo);
			mDownsample.release(1, msaa, ppInfo);

			mEyeAdaptHistogramReduce.execute(ppInfo);
			mEyeAdaptHistogram.release(ppInfo);

			mEyeAdaptation.execute(ppInfo, frameDelta);
			mEyeAdaptHistogramReduce.release(ppInfo);
		}

		bool gammaOnly;
		bool autoExposure;
		if (hdr)
		{
			if (settings.enableTonemapping)
			{
				if (ppInfo.settingDirty) // Rebuild LUT if PP settings changed
					mCreateLUT.execute(ppInfo);

				gammaOnly = false;
			}
			else
				gammaOnly = true;

			autoExposure = settings.enableAutoExposure;
		}
		else
		{
			gammaOnly = true;
			autoExposure = false;
		}





		// DEBUG ONLY
		//SSRTraceMat ssrTrace;

		//renderTargets->allocate(RTT_ResolvedSceneColorSecondary);
		//SPtr<RenderTarget> target = renderTargets->getRT(RTT_ResolvedSceneColorSecondary);

		//ssrTrace.execute(*viewInfo, ppInfo.settings->screenSpaceReflections, target);

		//RenderAPI::instance().setRenderTarget(renderTargets->getRT(RTT_ResolvedSceneColor));
		//gRendererUtility().blit(renderTargets->get(RTT_ResolvedSceneColorSecondary));

		bool performDOF = GaussianDOF::requiresDOF(settings.depthOfField);

		SPtr<RenderTarget> tonemapTarget;
		if (!performDOF && !settings.enableFXAA)
			tonemapTarget = viewProps.target;
		else
		{
			renderTargets->allocate(RTT_ResolvedSceneColorSecondary);
			tonemapTarget = renderTargets->getRT(RTT_ResolvedSceneColorSecondary);
		}

		mTonemapping.execute(gammaOnly, autoExposure, msaa, sceneColor, tonemapTarget, viewportRect, ppInfo);



		// DEBUG ONLY
		//renderTargets->release(RTT_ResolvedSceneColorSecondary);
		//return;


		if(performDOF)
		{
			SPtr<RenderTarget> dofTarget;

			// If DOF is the final effect, output to final target, otherwise use a temporary
			if (settings.enableFXAA)
			{
				renderTargets->allocate(RTT_ResolvedSceneColor);
				dofTarget = renderTargets->getRT(RTT_ResolvedSceneColor);
			}
			else
				dofTarget = viewProps.target;

			SPtr<Texture> sceneDepth = renderTargets->get(RTT_ResolvedDepth);

			mGaussianDOF.execute(renderTargets->get(RTT_ResolvedSceneColorSecondary), sceneDepth, dofTarget, *viewInfo, 
				settings.depthOfField);

			renderTargets->release(RTT_ResolvedSceneColorSecondary);
		}

		if(settings.enableFXAA)
		{
			SPtr<Texture> fxaaSource;
			if (performDOF)
				fxaaSource = renderTargets->get(RTT_ResolvedSceneColor);
			else
				fxaaSource = renderTargets->get(RTT_ResolvedSceneColorSecondary);

			// Note: I could skip executing FXAA over DOF and motion blurred pixels
			mFXAA.execute(fxaaSource, viewProps.target);

			if (performDOF)
				renderTargets->release(RTT_ResolvedSceneColor);
			else
				renderTargets->release(RTT_ResolvedSceneColorSecondary);
		}

		if (ppInfo.settingDirty)
			ppInfo.settingDirty = false;
	}

	void PostProcessing::buildSSAO(const RendererView& view)
	{
		const SPtr<RenderTargets> renderTargets = view.getRenderTargets();
		const PostProcessInfo& ppInfo = view.getPPInfo();

		mSSAO.execute(view, renderTargets->getRT(RTT_AmbientOcclusion), ppInfo.settings->ambientOcclusion);
	}
}}
