//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsRenderTexturePool.h"
#include "BsStandardPostProcessSettings.h"

namespace bs
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Contains per-camera data used by post process effects. */
	struct PostProcessInfo
	{
		SPtr<StandardPostProcessSettings> settings;
		bool settingDirty = true;

		SPtr<PooledRenderTexture> downsampledSceneTex;
		SPtr<PooledRenderTexture> histogramTex;
		SPtr<PooledRenderTexture> histogramReduceTex;
		SPtr<PooledRenderTexture> eyeAdaptationTex[2];
		SPtr<PooledRenderTexture> colorLUT;
		INT32 lastEyeAdaptationTex = 0;
	};

	BS_PARAM_BLOCK_BEGIN(DownsampleParams)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvTexSize)
	BS_PARAM_BLOCK_END

	/** Shader that downsamples a texture to half its size. */
	class DownsampleMat : public RendererMaterial<DownsampleMat>
	{
		RMAT_DEF("PPDownsample.bsl");

	public:
		DownsampleMat();

		/** Renders the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTextureCore>& target, PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output will be written. */
		SPtr<RenderTextureCore> getOutput() const { return mOutput; }
	private:
		DownsampleParams mParams;
		GpuParamTextureCore mInputTexture;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTextureCore> mOutput;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptHistogramParams)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gPixelOffsetAndSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHistogramParams)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gThreadGroupCount)
	BS_PARAM_BLOCK_END

	/** Shader that creates a luminance histogram used for eye adaptation. */
	class EyeAdaptHistogramMat : public RendererMaterial<EyeAdaptHistogramMat>
	{
		RMAT_DEF("PPEyeAdaptHistogram.bsl");

	public:
		EyeAdaptHistogramMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output was written. */
		SPtr<RenderTextureCore> getOutput() const { return mOutput; }

		/** Calculates the number of thread groups that need to execute to cover the provided render target. */
		static Vector2I getThreadGroupCount(const SPtr<RenderTextureCore>& target);

		/** 
		 * Returns a vector containing scale and offset (in that order) that will be applied to luminance values
		 * to determine their position in the histogram. 
		 */
		static Vector2 getHistogramScaleOffset(const PostProcessInfo& ppInfo);

		static const UINT32 THREAD_GROUP_SIZE_X = 8;
		static const UINT32 THREAD_GROUP_SIZE_Y = 8;
		
		static const UINT32 HISTOGRAM_NUM_TEXELS = (THREAD_GROUP_SIZE_X * THREAD_GROUP_SIZE_Y) / 4;
	private:
		EyeAdaptHistogramParams mParams;
		GpuParamTextureCore mSceneColor;
		GpuParamLoadStoreTextureCore mOutputTex;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTextureCore> mOutput;

		static const UINT32 LOOP_COUNT_X = 8;
		static const UINT32 LOOP_COUNT_Y = 8;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptHistogramReduceParams)
		BS_PARAM_BLOCK_ENTRY(int, gThreadGroupCount)
	BS_PARAM_BLOCK_END

	/** Shader that reduces the luminance histograms created by EyeAdaptHistogramMat into a single histogram. */
	class EyeAdaptHistogramReduceMat : public RendererMaterial<EyeAdaptHistogramReduceMat>
	{
		RMAT_DEF("PPEyeAdaptHistogramReduce.bsl");

	public:
		EyeAdaptHistogramReduceMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output was written. */
		SPtr<RenderTextureCore> getOutput() const { return mOutput; }
	private:
		EyeAdaptHistogramReduceParams mParams;

		GpuParamTextureCore mHistogramTex;
		GpuParamTextureCore mEyeAdaptationTex;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTextureCore> mOutput;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptationParams)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gEyeAdaptationParams, 3)
	BS_PARAM_BLOCK_END

	/** Shader that computes the eye adaptation value based on scene luminance. */
	class EyeAdaptationMat : public RendererMaterial<EyeAdaptationMat>
	{
		RMAT_DEF("PPEyeAdaptation.bsl");

	public:
		EyeAdaptationMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(PostProcessInfo& ppInfo, float frameDelta);
	private:
		EyeAdaptationParams mParams;
		GpuParamTextureCore mReducedHistogramTex;
	};

	BS_PARAM_BLOCK_BEGIN(CreateTonemapLUTParams)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gTonemapParams, 2)
		BS_PARAM_BLOCK_ENTRY(float, gGammaAdjustment)
		BS_PARAM_BLOCK_ENTRY(int, gGammaCorrectionType)
		BS_PARAM_BLOCK_ENTRY(Vector3, gSaturation)
		BS_PARAM_BLOCK_ENTRY(Vector3, gContrast)
		BS_PARAM_BLOCK_ENTRY(Vector3, gGain)
		BS_PARAM_BLOCK_ENTRY(Vector3, gOffset)
	BS_PARAM_BLOCK_END

	BS_PARAM_BLOCK_BEGIN(WhiteBalanceParams)
		BS_PARAM_BLOCK_ENTRY(float, gWhiteTemp)
		BS_PARAM_BLOCK_ENTRY(float, gWhiteOffset)
	BS_PARAM_BLOCK_END

	/** 
	 * Shader that creates a 3D lookup texture that is used to apply tonemapping, color grading, white balancing and gamma
	 * correction.
	 */
	class CreateTonemapLUTMat : public RendererMaterial<CreateTonemapLUTMat>
	{
		RMAT_DEF("PPCreateTonemapLUT.bsl");

	public:
		CreateTonemapLUTMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Size of the 3D color lookup table. */
		static const UINT32 LUT_SIZE = 32;
	private:
		CreateTonemapLUTParams mParams;
		WhiteBalanceParams mWhiteBalanceParams;
	};

	BS_PARAM_BLOCK_BEGIN(TonemappingParams)
		BS_PARAM_BLOCK_ENTRY(float, gRawGamma)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
	BS_PARAM_BLOCK_END

	/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
	template<bool GammaOnly, bool AutoExposure>
	class TonemappingMat : public RendererMaterial<TonemappingMat<GammaOnly, AutoExposure>>
	{
		RMAT_DEF("PPTonemapping.bsl");

	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTextureCore>& sceneColor, const SPtr<ViewportCore>& outputViewport,
			PostProcessInfo& ppInfo);

	private:
		TonemappingParams mParams;

		GpuParamTextureCore mInputTex;
		GpuParamTextureCore mColorLUT;
		GpuParamTextureCore mEyeAdaptationTex;
	};

	/**
	 * Renders post-processing effects for the provided render target.
	 *
	 * @note	Core thread only.
	 */
	class BS_BSRND_EXPORT PostProcessing : public Module<PostProcessing>
	{
	public:
		/** Renders post-processing effects for the provided render target. */
		void postProcess(const SPtr<RenderTextureCore>& sceneColor, const CameraCore* camera,
			PostProcessInfo& ppInfo, float frameDelta);
		
	private:
		DownsampleMat mDownsample;
		EyeAdaptHistogramMat mEyeAdaptHistogram;
		EyeAdaptHistogramReduceMat mEyeAdaptHistogramReduce;
		EyeAdaptationMat mEyeAdaptation;

		CreateTonemapLUTMat mCreateLUT;
		TonemappingMat<false, true> mTonemapping_AE;
		TonemappingMat<true, true> mTonemapping_AE_GO;
		TonemappingMat<false, false> mTonemapping;
		TonemappingMat<true, false> mTonemapping_GO;
	};

	/** @} */
}