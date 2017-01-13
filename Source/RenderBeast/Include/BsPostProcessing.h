//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsRenderTexturePool.h"
#include "BsStandardPostProcessSettings.h"

namespace bs { namespace ct
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

	BS_PARAM_BLOCK_BEGIN(DownsampleParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvTexSize)
	BS_PARAM_BLOCK_END

	extern DownsampleParamDef gDownsampleParamDef;

	/** Shader that downsamples a texture to half its size. */
	class DownsampleMat : public RendererMaterial<DownsampleMat>
	{
		RMAT_DEF("PPDownsample.bsl");

	public:
		DownsampleMat();

		/** Renders the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTexture>& target, PostProcessInfo& ppInfo);

		/** Releases the output render target. */
		void release(PostProcessInfo& ppInfo);

		/** Returns the render texture where the output will be written. */
		SPtr<RenderTexture> getOutput() const { return mOutput; }
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTexture> mOutput;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptHistogramParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gPixelOffsetAndSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHistogramParams)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gThreadGroupCount)
	BS_PARAM_BLOCK_END

	extern EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

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
		SPtr<RenderTexture> getOutput() const { return mOutput; }

		/** Calculates the number of thread groups that need to execute to cover the provided render target. */
		static Vector2I getThreadGroupCount(const SPtr<RenderTexture>& target);

		/** 
		 * Returns a vector containing scale and offset (in that order) that will be applied to luminance values
		 * to determine their position in the histogram. 
		 */
		static Vector2 getHistogramScaleOffset(const PostProcessInfo& ppInfo);

		static const UINT32 THREAD_GROUP_SIZE_X = 8;
		static const UINT32 THREAD_GROUP_SIZE_Y = 8;
		
		static const UINT32 HISTOGRAM_NUM_TEXELS = (THREAD_GROUP_SIZE_X * THREAD_GROUP_SIZE_Y) / 4;
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mSceneColor;
		GpuParamLoadStoreTexture mOutputTex;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTexture> mOutput;

		static const UINT32 LOOP_COUNT_X = 8;
		static const UINT32 LOOP_COUNT_Y = 8;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptHistogramReduceParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gThreadGroupCount)
	BS_PARAM_BLOCK_END

	extern EyeAdaptHistogramReduceParamDef gEyeAdaptHistogramReduceParamDef;

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
		SPtr<RenderTexture> getOutput() const { return mOutput; }
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mHistogramTex;
		GpuParamTexture mEyeAdaptationTex;

		POOLED_RENDER_TEXTURE_DESC mOutputDesc;
		SPtr<RenderTexture> mOutput;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptationParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gEyeAdaptationParams, 3)
	BS_PARAM_BLOCK_END

	extern EyeAdaptationParamDef gEyeAdaptationParamDef;

	/** Shader that computes the eye adaptation value based on scene luminance. */
	class EyeAdaptationMat : public RendererMaterial<EyeAdaptationMat>
	{
		RMAT_DEF("PPEyeAdaptation.bsl");

	public:
		EyeAdaptationMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(PostProcessInfo& ppInfo, float frameDelta);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mReducedHistogramTex;
	};

	BS_PARAM_BLOCK_BEGIN(CreateTonemapLUTParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gTonemapParams, 2)
		BS_PARAM_BLOCK_ENTRY(float, gGammaAdjustment)
		BS_PARAM_BLOCK_ENTRY(int, gGammaCorrectionType)
		BS_PARAM_BLOCK_ENTRY(Vector3, gSaturation)
		BS_PARAM_BLOCK_ENTRY(Vector3, gContrast)
		BS_PARAM_BLOCK_ENTRY(Vector3, gGain)
		BS_PARAM_BLOCK_ENTRY(Vector3, gOffset)
	BS_PARAM_BLOCK_END

	extern CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;

	BS_PARAM_BLOCK_BEGIN(WhiteBalanceParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gWhiteTemp)
		BS_PARAM_BLOCK_ENTRY(float, gWhiteOffset)
	BS_PARAM_BLOCK_END

	extern WhiteBalanceParamDef gWhiteBalanceParamDef;

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
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mWhiteBalanceParamBuffer;

		GpuParamLoadStoreTexture mOutputTex;
	};

	BS_PARAM_BLOCK_BEGIN(TonemappingParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gRawGamma)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
	BS_PARAM_BLOCK_END

	extern TonemappingParamDef gTonemappingParamDef;

	/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
	template<bool GammaOnly, bool AutoExposure>
	class TonemappingMat : public RendererMaterial<TonemappingMat<GammaOnly, AutoExposure>>
	{
		RMAT_DEF("PPTonemapping.bsl");

	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<RenderTexture>& sceneColor, const SPtr<RenderTarget>& outputRT, const Rect2& outputRect,
			PostProcessInfo& ppInfo);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mColorLUT;
		GpuParamTexture mEyeAdaptationTex;
	};

	/**
	 * Renders post-processing effects for the provided render target.
	 *
	 * @note	Core thread only.
	 */
	class BS_BSRND_EXPORT PostProcessing : public Module<PostProcessing>
	{
	public:
		/** 
		 * Renders post-processing effects for the provided render target. Resolves provided scene color texture into the
		 * view's final output render target. Once the method exits, final render target is guaranteed to be currently
		 * bound for rendering. 
		 */
		void postProcess(RendererCamera* viewInfo, const SPtr<RenderTexture>& sceneColor, float frameDelta);
		
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
}}
