//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPostProcessing.h"
#include "BsRenderTexture.h"
#include "BsRenderTexturePool.h"
#include "BsRendererUtility.h"
#include "BsTextureManager.h"
#include "BsCamera.h"
#include "BsGpuParamsSet.h"

namespace bs
{
	DownsampleMat::DownsampleMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void DownsampleMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void DownsampleMat::execute(const SPtr<RenderTextureCore>& target, PostProcessInfo& ppInfo)
	{
		// Set parameters
		SPtr<TextureCore> colorTexture = target->getColorTexture(0);
		mInputTexture.set(colorTexture);

		const RenderTextureProperties& rtProps = target->getProperties();
		Vector2 invTextureSize(1.0f / rtProps.getWidth(), 1.0f / rtProps.getHeight());

		mParams.gInvTexSize.set(invTextureSize);

		// Set output
		const TextureProperties& colorProps = colorTexture->getProperties();

		UINT32 width = std::max(1, Math::ceilToInt(colorProps.getWidth() * 0.5f));
		UINT32 height = std::max(1, Math::ceilToInt(colorProps.getHeight() * 0.5f));

		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(colorProps.getFormat(), width, height, TU_RENDERTARGET);

		// Render
		ppInfo.downsampledSceneTex = RenderTexturePool::instance().get(mOutputDesc);

		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(ppInfo.downsampledSceneTex->renderTexture, true);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);

		mOutput = ppInfo.downsampledSceneTex->renderTexture;
	}

	void DownsampleMat::release(PostProcessInfo& ppInfo)
	{
		RenderTexturePool::instance().release(ppInfo.downsampledSceneTex);
		mOutput = nullptr;
	}

	EyeAdaptHistogramMat::EyeAdaptHistogramMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());

		SPtr<GpuParamsCore> params = mParamsSet->getGpuParams();
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
		SPtr<RenderTextureCore> target = ppInfo.downsampledSceneTex->renderTexture;
		mSceneColor.set(ppInfo.downsampledSceneTex->texture);

		const RenderTextureProperties& props = target->getProperties();
		int offsetAndSize[4] = { 0, 0, (INT32)props.getWidth(), (INT32)props.getHeight() };

		mParams.gHistogramParams.set(getHistogramScaleOffset(ppInfo));
		mParams.gPixelOffsetAndSize.set(Vector4I(offsetAndSize));

		Vector2I threadGroupCount = getThreadGroupCount(target);
		mParams.gThreadGroupCount.set(threadGroupCount);

		// Set output
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, HISTOGRAM_NUM_TEXELS, numHistograms,
			TU_LOADSTORE);

		// Dispatch
		ppInfo.histogramTex = RenderTexturePool::instance().get(mOutputDesc);

		mOutputTex.set(ppInfo.histogramTex->texture);

		RenderAPICore& rapi = RenderAPICore::instance();
		gRendererUtility().setComputePass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		rapi.dispatchCompute(threadGroupCount.x, threadGroupCount.y);

		mOutput = ppInfo.histogramTex->renderTexture;
	}

	void EyeAdaptHistogramMat::release(PostProcessInfo& ppInfo)
	{
		RenderTexturePool::instance().release(ppInfo.histogramTex);
		mOutput = nullptr;
	}

	Vector2I EyeAdaptHistogramMat::getThreadGroupCount(const SPtr<RenderTextureCore>& target)
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

	EyeAdaptHistogramReduceMat::EyeAdaptHistogramReduceMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());

		SPtr<GpuParamsCore> params = mParamsSet->getGpuParams();
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
		SPtr<TextureCore> eyeAdaptationTex;

		if (eyeAdaptationRT != nullptr) // Could be that this is the first run
			eyeAdaptationTex = eyeAdaptationRT->texture;
		else
			eyeAdaptationTex = TextureCore::WHITE;

		mEyeAdaptationTex.set(eyeAdaptationTex);

		Vector2I threadGroupCount = EyeAdaptHistogramMat::getThreadGroupCount(ppInfo.downsampledSceneTex->renderTexture);
		UINT32 numHistograms = threadGroupCount.x * threadGroupCount.y;

		mParams.gThreadGroupCount.set(numHistograms);

		// Set output
		mOutputDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_FLOAT16_RGBA, EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2,
			TU_RENDERTARGET);

		// Render
		ppInfo.histogramReduceTex = RenderTexturePool::instance().get(mOutputDesc);

		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(ppInfo.histogramReduceTex->renderTexture, true);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		Rect2 drawUV(0.0f, 0.0f, (float)EyeAdaptHistogramMat::HISTOGRAM_NUM_TEXELS, 2.0f);
		gRendererUtility().drawScreenQuad(drawUV);

		rapi.setRenderTarget(nullptr);

		mOutput = ppInfo.histogramReduceTex->renderTexture;
	}

	void EyeAdaptHistogramReduceMat::release(PostProcessInfo& ppInfo)
	{
		RenderTexturePool::instance().release(ppInfo.histogramReduceTex);
		mOutput = nullptr;
	}

	EyeAdaptationMat::EyeAdaptationMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());
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
			ppInfo.eyeAdaptationTex[0] = RenderTexturePool::instance().get(outputDesc);
			ppInfo.eyeAdaptationTex[1] = RenderTexturePool::instance().get(outputDesc);
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

		mParams.gEyeAdaptationParams.set(eyeAdaptationParams[0], 0);
		mParams.gEyeAdaptationParams.set(eyeAdaptationParams[1], 1);
		mParams.gEyeAdaptationParams.set(eyeAdaptationParams[2], 2);

		// Render
		SPtr<PooledRenderTexture> eyeAdaptationRT = ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex];

		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(eyeAdaptationRT->renderTexture, true);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();

		rapi.setRenderTarget(nullptr);
	}

	CreateTonemapLUTMat::CreateTonemapLUTMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());
		mParamsSet->setParamBlockBuffer("WhiteBalanceInput", mWhiteBalanceParams.getBuffer());
	}

	void CreateTonemapLUTMat::_initDefines(ShaderDefines& defines)
	{
		defines.set("LUT_SIZE", LUT_SIZE);
	}

	void CreateTonemapLUTMat::execute(PostProcessInfo& ppInfo)
	{
		const StandardPostProcessSettings& settings = *ppInfo.settings;

		// Set parameters
		mParams.gGammaAdjustment.set(2.2f / settings.gamma);

		// Note: Assuming sRGB (PC monitor) for now, change to Rec.709 when running on console (value 1), or to raw 2.2
		// gamma when running on Mac (value 2)
		mParams.gGammaCorrectionType.set(0);

		Vector4 tonemapParams[2];
		tonemapParams[0].x = settings.tonemapping.filmicCurveShoulderStrength;
		tonemapParams[0].y = settings.tonemapping.filmicCurveLinearStrength;
		tonemapParams[0].z = settings.tonemapping.filmicCurveLinearAngle;
		tonemapParams[0].w = settings.tonemapping.filmicCurveToeStrength;

		tonemapParams[1].x = settings.tonemapping.filmicCurveToeNumerator;
		tonemapParams[1].y = settings.tonemapping.filmicCurveToeDenominator;
		tonemapParams[1].z = settings.tonemapping.filmicCurveLinearWhitePoint;
		tonemapParams[1].w = 0.0f; // Unused

		mParams.gTonemapParams.set(tonemapParams[0], 0);
		mParams.gTonemapParams.set(tonemapParams[1], 1);

		// Set color grading params
		mParams.gSaturation.set(settings.colorGrading.saturation);
		mParams.gContrast.set(settings.colorGrading.contrast);
		mParams.gGain.set(settings.colorGrading.gain);
		mParams.gOffset.set(settings.colorGrading.offset);

		// Set white balance params
		mWhiteBalanceParams.gWhiteTemp.set(settings.whiteBalance.temperature);
		mWhiteBalanceParams.gWhiteOffset.set(settings.whiteBalance.tint);

		// Set output
		POOLED_RENDER_TEXTURE_DESC outputDesc = POOLED_RENDER_TEXTURE_DESC::create3D(PF_B8G8R8X8, 
			LUT_SIZE, LUT_SIZE, LUT_SIZE, TU_RENDERTARGET);

		// Render
		ppInfo.colorLUT = RenderTexturePool::instance().get(outputDesc);

		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setRenderTarget(ppInfo.colorLUT->renderTexture);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad(LUT_SIZE);
	}

	void CreateTonemapLUTMat::release(PostProcessInfo& ppInfo)
	{
		RenderTexturePool::instance().release(ppInfo.colorLUT);
	}

	template<bool GammaOnly, bool AutoExposure>
	TonemappingMat<GammaOnly, AutoExposure>::TonemappingMat()
	{
		mParamsSet->setParamBlockBuffer("Input", mParams.getBuffer());

		SPtr<GpuParamsCore> params = mParamsSet->getGpuParams();
		params->getTextureParam(GPT_VERTEX_PROGRAM, "gEyeAdaptationTex", mEyeAdaptationTex);
		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTex);

		if(!GammaOnly)
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gColorLUT", mColorLUT);
	}

	template<bool GammaOnly, bool AutoExposure>
	void TonemappingMat<GammaOnly, AutoExposure>::_initDefines(ShaderDefines& defines)
	{
		if(GammaOnly)
			defines.set("GAMMA_ONLY", 1);

		if (AutoExposure)
			defines.set("AUTO_EXPOSURE", 1);

		defines.set("LUT_SIZE", CreateTonemapLUTMat::LUT_SIZE);
	}

	template<bool GammaOnly, bool AutoExposure>
	void TonemappingMat<GammaOnly, AutoExposure>::execute(const SPtr<RenderTextureCore>& sceneColor, const SPtr<ViewportCore>& outputViewport,
		PostProcessInfo& ppInfo)
	{
		mParams.gRawGamma.set(1.0f / ppInfo.settings->gamma);
		mParams.gManualExposureScale.set(Math::pow(2.0f, ppInfo.settings->exposureScale));

		// Set parameters
		SPtr<TextureCore> colorTexture = sceneColor->getColorTexture(0);
		mInputTex.set(colorTexture);

		SPtr<TextureCore> colorLUT;
		if(ppInfo.colorLUT != nullptr)
			colorLUT = ppInfo.colorLUT->texture;

		mColorLUT.set(colorLUT);

		SPtr<TextureCore> eyeAdaptationTexture;
		if(ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex] != nullptr)
			eyeAdaptationTexture = ppInfo.eyeAdaptationTex[ppInfo.lastEyeAdaptationTex]->texture;

		mEyeAdaptationTex.set(eyeAdaptationTexture);

		// Render
		RenderAPICore& rapi = RenderAPICore::instance();
		SPtr<RenderTargetCore> target = outputViewport->getTarget();

		rapi.setRenderTarget(target);
		rapi.setViewport(outputViewport->getNormArea());

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	template class TonemappingMat<true, true>;
	template class TonemappingMat<false, true>;
	template class TonemappingMat<true, false>;
	template class TonemappingMat<false, false>;

	void PostProcessing::postProcess(const SPtr<RenderTextureCore>& sceneColor, const CameraCore* camera, 
		PostProcessInfo& ppInfo, float frameDelta)
	{
		const StandardPostProcessSettings& settings = *ppInfo.settings;

		SPtr<ViewportCore> outputViewport = camera->getViewport();
		bool hdr = camera->getFlags().isSet(CameraFlag::HDR);

		if(hdr && settings.enableAutoExposure)
		{
			mDownsample.execute(sceneColor, ppInfo);
			mEyeAdaptHistogram.execute(ppInfo);
			mDownsample.release(ppInfo);

			mEyeAdaptHistogramReduce.execute(ppInfo);
			mEyeAdaptHistogram.release(ppInfo);

			mEyeAdaptation.execute(ppInfo, frameDelta);
			mEyeAdaptHistogramReduce.release(ppInfo);
		}

		if (hdr && settings.enableTonemapping)
		{
			if (ppInfo.settingDirty) // Rebuild LUT if PP settings changed
				mCreateLUT.execute(ppInfo);

			if (settings.enableAutoExposure)
				mTonemapping_AE.execute(sceneColor, outputViewport, ppInfo);
			else
				mTonemapping.execute(sceneColor, outputViewport, ppInfo);
		}
		else
		{
			if (hdr && settings.enableAutoExposure)
				mTonemapping_AE_GO.execute(sceneColor, outputViewport, ppInfo);
			else
				mTonemapping_GO.execute(sceneColor, outputViewport, ppInfo);
		}

		if (ppInfo.settingDirty)
			ppInfo.settingDirty = false;

		// TODO - External code depends on the main RT being bound when this exits, make this clearer
	}
}