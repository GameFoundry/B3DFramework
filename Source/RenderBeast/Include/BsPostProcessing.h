//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsRenderTexturePool.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Settings that control the post-process operation. */
	struct PostProcessSettings
	{
		PostProcessSettings();

		/** 
		 * Determines minimum luminance value in the eye adaptation histogram. In log2 units (-8 = 1/256). In the range
		 * [-16, 0]. 
		 */
		float histogramLog2Min; 

		/** 
		 * Determines maximum luminance value in the eye adaptation histogram. In log2 units (4 = 16). In the range
		 * [0, 16]. 
		 */
		float histogramLog2Max;

		/** Percentage below which to ignore values in the eye adaptation histogram. In range [0.0f, 1.0f]. */
		float histogramPctLow;

		/** Percentage above which to ignore values in the eye adaptation histogram. In range [0.0f, 1.0f]. */
		float histogramPctHigh;

		/** Clamps the minimum eye adaptation scale (pre-exposure scale) to this value. In range [0.0f, 10.0f]. */
		float minEyeAdaptation;

		/** Clamps the maximum eye adaptation scale (pre-exposure scale) to this value. In range [0.0f, 10.0f]. */
		float maxEyeAdaptation;

		/** 
		 * Log2 value to scale the eye adaptation by. Smaller values yield darker image, while larger yield brighter image. 
		 * In range [-8, 8].
		 */
		float exposureScale;

		/** Determines how quickly does the eye adaptation adjust to larger values. In range [0.01f, 20.0f]. */
		float eyeAdaptationSpeedUp;

		/** Determines how quickly does the eye adaptation adjust to smaller values. In range [0.01f, 20.0f]. */
		float eyeAdaptationSpeedDown;
	};

	/** Contains per-camera data used by post process effects. */
	struct PostProcessInfo
	{
		PostProcessSettings settings;

		SPtr<PooledRenderTexture> downsampledSceneTex;
		SPtr<PooledRenderTexture> histogramTex;
		SPtr<PooledRenderTexture> histogramReduceTex;
		SPtr<PooledRenderTexture> eyeAdaptationTex[2];
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
		MaterialParamVec2Core mInvTexSize;
		MaterialParamTextureCore mInputTexture;

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
		MaterialParamTextureCore mSceneColor;
		MaterialParamLoadStoreTextureCore mOutputTex;

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

		MaterialParamTextureCore mHistogramTex;
		MaterialParamTextureCore mEyeAdaptationTex;

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
		MaterialParamTextureCore mReducedHistogramTex;
	};

	/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
	class TonemappingMat : public RendererMaterial<TonemappingMat>
	{
		RMAT_DEF("PPTonemapping.bsl");

	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTextureCore>& sceneColor, const SPtr<ViewportCore>& outputViewport,
			PostProcessInfo& ppInfo);
	private:
		MaterialParamTextureCore mInputTex;
		MaterialParamTextureCore mEyeAdaptationTex;
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
		void postProcess(const SPtr<RenderTextureCore>& sceneColor, const SPtr<ViewportCore>& outputViewport,
			PostProcessInfo& ppInfo, float frameDelta);
		
	private:
		DownsampleMat mDownsample;
		EyeAdaptHistogramMat mEyeAdaptHistogram;
		EyeAdaptHistogramReduceMat mEyeAdaptHistogramReduce;
		EyeAdaptationMat mEyeAdaptation;
		TonemappingMat mTonemapping;
	};

	/** @} */
}