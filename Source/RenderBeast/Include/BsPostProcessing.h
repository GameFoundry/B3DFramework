//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsGpuResourcePool.h"
#include "BsStandardPostProcessSettings.h"
#include "BsLightRendering.h"

namespace bs { namespace ct
{
	struct RendererViewTargetData;

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
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector2, gOffsets, 4)
	BS_PARAM_BLOCK_END

	extern DownsampleParamDef gDownsampleParamDef;

	/** 
	 * Shader that downsamples a texture to half its size. 
	 * 
	 * @tparam	Quality		0 for a 2x2 filtered sample, 1 or higher for 4x4 filtered sample
	 * @tparam	MSAA		True if the input texture is multi-sampled. Only first sample will be used, the rest will be
	 *						discarded.
	 */
	template<int Quality, bool MSAA>
	class DownsampleMat : public RendererMaterial<DownsampleMat<Quality, MSAA>>
	{
		RMAT_DEF("PPDownsample.bsl");

	public:
		DownsampleMat();

		/** Renders the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& target, PostProcessInfo& ppInfo);

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

	/** Contains all variations of the DownsampleMat material. */
	class DownsampleMaterials
	{
	public:
		/**
		 * Executes the appropriate downsampling material.
		 * 
		 * @param[in]	quality		Determines quality of the downsampling filer. Specify 0 to use a 2x2 filter block, and
		 *							1 or higher for a 4x4 filter block.
		 * @param[in]	msaa		If true the input texture is assumed to have multiple samples. The downsampling shader
		 *							will discard all samples except the first one.
		 * @param[in]	target		Texture to downsample.
		 * @param[in]	ppInfo		Information about the current post processing pass.
		 */
		void execute(UINT32 quality, bool msaa, const SPtr<Texture>& target, PostProcessInfo& ppInfo);

		/**
		 * Releases any resources allocated by execute(). Must be called using the same @p quality and @p msaa parameters as
		 * the corresponding execute() call. @see execute().
		 */
		void release(UINT32 quality, bool msaa, PostProcessInfo& ppInfo);
	private:
		DownsampleMat<0, false> m0_NoMSAA;
		DownsampleMat<0, true> m0_MSAA;
		
		DownsampleMat<1, false> m1_NoMSAA;
		DownsampleMat<1, true> m1_MSAA;
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
		BS_PARAM_BLOCK_ENTRY(int, gNumSamples)
	BS_PARAM_BLOCK_END

	extern TonemappingParamDef gTonemappingParamDef;

	/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
	template<bool GammaOnly, bool AutoExposure, bool MSAA>
	class TonemappingMat : public RendererMaterial<TonemappingMat<GammaOnly, AutoExposure, MSAA>>
	{
		RMAT_DEF("PPTonemapping.bsl");

	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& sceneColor, const SPtr<RenderTarget>& outputRT, const Rect2& outputRect,
			PostProcessInfo& ppInfo);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mColorLUT;
		GpuParamTexture mEyeAdaptationTex;
	};

	/** Container for all variations of the TonemappingMat material. */
	class TonemappingMaterials
	{
	public:
		/** 
		 * Finds a valid tonemapping material according to the requested parameters and executes it. 
		 *
		 * @param[in]	gammaOnly		If true no color correction, white balancing or curve tonemapping will take place. 
		 *								Instead the image will only be scaled by the exposure value and gamma corrected.
		 * @param[in]	autoExposure	If true, the automatically calculated eye-adapatation exposure value will be used
		 *								as the exposure scale. If false, the user provided value will be used instead.
		 * @param[in]	MSAA			True if the input texture has multiple samples. This will ensure that the samples
		 *								are resolved properly into a non-MSAA output texture.
		 * @param[in]	sceneColor		Texture to apply tonemapping to.
		 * @param[in]	outputRT		Render target to write the results to.
		 * @param[in]	outputRect		Normalized rectangle determining to which part of the output texture to write to.
		 * @param[in]	ppInfo			Information about the current post processing pass.
		 */
		void execute(bool gammaOnly, bool autoExposure, bool MSAA, const SPtr<Texture>& sceneColor,
			const SPtr<RenderTarget>& outputRT, const Rect2& outputRect, PostProcessInfo& ppInfo);

	private:
		TonemappingMat<false, false, false> mFFF;
		TonemappingMat<false, false, true> mFFT;
		TonemappingMat<false, true, false> mFTF;
		TonemappingMat<false, true, true> mFTT;
		TonemappingMat<true, false, false> mTFF;
		TonemappingMat<true, false, true> mTFT;
		TonemappingMat<true, true, false> mTTF;
		TonemappingMat<true, true, true> mTTT;
	};

	const int MAX_BLUR_SAMPLES = 128;

	BS_PARAM_BLOCK_BEGIN(GaussianBlurParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleOffsets, (MAX_BLUR_SAMPLES + 1) / 2)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleWeights, (MAX_BLUR_SAMPLES + 3) / 4)
		BS_PARAM_BLOCK_ENTRY(int, gNumSamples)
	BS_PARAM_BLOCK_END

	extern GaussianBlurParamDef gGaussianBlurParamDef;

	/** Shader that performs Gaussian blur filtering on the provided texture. */
	class GaussianBlurMat : public RendererMaterial<GaussianBlurMat>
	{
		// Direction of the Gaussian filter pass
		enum Direction
		{
			DirVertical,
			DirHorizontal
		};

		RMAT_DEF("PPGaussianBlur.bsl");

	public:
		GaussianBlurMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	source		Input texture to blur.
		 * @param[in]	filterSize	Size of the blurring filter, in percent of the source texture. In range [0, 1].
		 * @param[in]	destination	Output texture to which to write the blurred image to.
		 */
		void execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination);

	private:
		/** Calculates weights and offsets for the standard distribution of the specified filter size. */
		static UINT32 calcStdDistribution(float filterRadius, std::array<float, MAX_BLUR_SAMPLES>& weights,
			std::array<float, MAX_BLUR_SAMPLES>& offsets);

		/** Calculates the radius of the blur kernel depending on the source texture size and provided scale. */
		static float calcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir);

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	BS_PARAM_BLOCK_BEGIN(GaussianDOFParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gNearBlurPlane)
		BS_PARAM_BLOCK_ENTRY(float, gFarBlurPlane)
		BS_PARAM_BLOCK_ENTRY(float, gInvNearBlurRange)
		BS_PARAM_BLOCK_ENTRY(float, gInvFarBlurRange)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHalfPixelOffset)
	BS_PARAM_BLOCK_END

	extern GaussianDOFParamDef sGaussianDOFParamDef;

	/** Common interface for all variations of GaussianDOFSeparateMat. */
	class IGaussianDOFSeparateMat
	{
	public:
		virtual ~IGaussianDOFSeparateMat() { }

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	color		Input color texture to process.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering. 
		 */
		virtual void execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, 
			const DepthOfFieldSettings& settings) = 0;

		/**
		 * Returns the texture generated after the shader was executed. Only valid to call this in-between calls to
		 * execute() & release(), with @p idx value 0 or 1.
		 */
		virtual SPtr<PooledRenderTexture> getOutput(UINT32 idx) = 0;

		/**
		 * Releases the interally allocated output render textures. Must be called after each call to execute(), when the 
		 * caller is done using the textures.
		 */
		virtual void release() = 0;
	};

	/** 
	 * Shader that masks pixels from the input color texture into one or two output textures. The masking is done by
	 * determining if the pixel falls into near or far unfocused plane, as determined by depth-of-field parameters. User
	 * can pick whether to output pixels just on the near plane, just on the far plane, or both.
	 *
	 * @tparam	Near	If true, near plane pixels are output to the first render target.
	 * @tparam	Far		If true, far plane pixels are output to the first render target. If @p Near is also enabled, the
	 *					pixels are output to the second render target instead.
	 */
	template<bool Near, bool Far>
	class GaussianDOFSeparateMat : public IGaussianDOFSeparateMat, public RendererMaterial<GaussianDOFSeparateMat<Near, Far>>
	{
		RMAT_DEF("PPGaussianDOFSeparate.bsl");

	public:
		GaussianDOFSeparateMat();

		/** @copydoc IGaussianDOFSeparateMat::execute() */
		void execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, 
			const DepthOfFieldSettings& settings) override;

		/** @copydoc IGaussianDOFSeparateMat::getOutput() */
		SPtr<PooledRenderTexture> getOutput(UINT32 idx) override;

		/** @copydoc IGaussianDOFSeparateMat::release() */
		void release() override;

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mColorTexture;
		GpuParamTexture mDepthTexture;

		SPtr<PooledRenderTexture> mOutput0;
		SPtr<PooledRenderTexture> mOutput1;
	};

	/** Common interface for all variations of GaussianDOFCombineMat. */
	class IGaussianDOFCombineMat
	{
	public:
		virtual ~IGaussianDOFCombineMat() { }

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	focused		Input texture containing focused (default) scene color.
		 * @param[in]	near		Input texture containing filtered (blurred) values for the unfocused foreground area.
		 *							Can be null if no near plane needs to be blended.
		 * @param[in]	far			Input texture containing filtered (blurred) values for the unfocused background area.
		 *							Can be null if no far plane needs to be blended.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	output		Texture to output the results to.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering. 
		 */
		virtual void execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, 
			const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, 
			const DepthOfFieldSettings& settings) = 0;
	};

	/** 
	 * Shader that combines pixels for near unfocused, focused and far unfocused planes, as calculated by 
	 * GaussianDOFSeparateMat. Outputs final depth-of-field filtered image.
	 *
	 * @tparam	Near	If true, near plane pixels are read from the near plane texture, otherwise near plane is assumed
	 *					not to exist.
	 * @tparam	Far		If true, far plane pixels are read from the far plane texture, otherwise far plane is assumed not
	 *					to exist.
	 */
	template<bool Near, bool Far>
	class GaussianDOFCombineMat : public IGaussianDOFCombineMat, public RendererMaterial<GaussianDOFCombineMat<Near, Far>>
	{
		RMAT_DEF("PPGaussianDOFCombine.bsl");

	public:
		GaussianDOFCombineMat();

		void execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, 
			const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, 
			const DepthOfFieldSettings& settings) override;
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mFocusedTexture;
		GpuParamTexture mNearTexture;
		GpuParamTexture mFarTexture;
		GpuParamTexture mDepthTexture;
	};

	/** Performs Gaussian depth of field effect with the help of various related shaders. */
	class GaussianDOF
	{
	public:
		/** 
		 * Executes the depth of field effect on the provided scene color texture.
		 * 
		 * @param[in]	sceneColor	Input texture containing scene color.
		 * @param[in]	sceneDepth	Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	output		Texture to output the results to.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering. 
		 */
		void execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& sceneDepth, const SPtr<RenderTarget>& output, 
			const RendererView& view, const DepthOfFieldSettings& settings);

		/** Checks does the depth of field effect need to execute. */
		static bool requiresDOF(const DepthOfFieldSettings& settings);
	private:
		GaussianDOFSeparateMat<true, true> mSeparateNF;
		GaussianDOFSeparateMat<false, true> mSeparateF;
		GaussianDOFSeparateMat<true, false> mSeparateN;

		GaussianDOFCombineMat<true, true> mCombineNF;
		GaussianDOFCombineMat<false, true> mCombineF;
		GaussianDOFCombineMat<true, false> mCombineN;

		GaussianBlurMat mBlur;
	};

	/** Shader that calculates a single level of the hierarchical Z mipmap chain. */
	class BuildHiZMat : public RendererMaterial<BuildHiZMat>
	{
		RMAT_DEF("PPBuildHiZ.bsl");

	public:
		BuildHiZMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	source		Input depth texture to use as the source.
		 * @param[in]	srcMip		Mip level to read from the @p source texture.
		 * @param[in]	srcRect		Rectangle in normalized coordinates, describing from which portion of the source
		 *							texture to read the input.
		 * @param[in]	dstRect		Destination rectangle to limit the writes to.
		 * @param[in]	output		Output target to which to write to results.
		 */
		void execute(const SPtr<Texture>& source, UINT32 srcMip, const Rect2& srcRect, const Rect2& dstRect,
			const SPtr<RenderTexture>& output);
	private:
		GpuParamTexture mInputTexture;
	};

	/** Builds a hierarchical Z mipmap chain from the source depth texture. */
	class BuildHiZ
	{
	public:
		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	viewInfo	Information about the view we're rendering from.
		 * @param[in]	source		Input depth texture to use as the source.
		 * @param[in]	output		Output target to which to write to results. This texture should be created using the
		 *							descriptor returned by getHiZTextureDesc().
		 */
		void execute(const RendererViewTargetData& viewInfo, const SPtr<Texture>& source, const SPtr<Texture>& output);

		/** Generates a descriptor that can be used for creating a texture to contain the HiZ mipmap chain. */
		static POOLED_RENDER_TEXTURE_DESC getHiZTextureDesc(UINT32 viewWidth, UINT32 viewHeight);

	private:
		BuildHiZMat mHiZMat;
	};

	BS_PARAM_BLOCK_BEGIN(FXAAParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvTexSize)
	BS_PARAM_BLOCK_END

	extern FXAAParamDef gFXAAParamDef;

	/** Shader that performs Fast Approximate anti-aliasing. */
	class FXAAMat : public RendererMaterial<FXAAMat>
	{
		RMAT_DEF("PPFXAA.bsl");

	public:
		FXAAMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	source		Input texture to apply FXAA to.
		 * @param[in]	destination	Output target to which to write the antialiased image to.
		 */
		void execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	BS_PARAM_BLOCK_BEGIN(SSAOParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gSampleRadius)
		BS_PARAM_BLOCK_ENTRY(float, gWorldSpaceRadiusMask)
		BS_PARAM_BLOCK_ENTRY(Vector2, gTanHalfFOV)
		BS_PARAM_BLOCK_ENTRY(Vector2, gRandomTileScale)
		BS_PARAM_BLOCK_ENTRY(float, gCotHalfFOV)
		BS_PARAM_BLOCK_ENTRY(float, gBias)
		BS_PARAM_BLOCK_ENTRY(Vector2, gDownsampledPixelSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gFadeMultiplyAdd)
		BS_PARAM_BLOCK_ENTRY(float, gPower)
		BS_PARAM_BLOCK_ENTRY(float, gIntensity)
	BS_PARAM_BLOCK_END

	extern SSAOParamDef gSSAOParamDef;

	/** Textures used as input when calculating SSAO. */
	struct SSAOTextureInputs
	{
		/** Full resolution scene depth. Only used by final SSAO pass. */
		SPtr<Texture> sceneDepth;

		/** Full resolution buffer containing scene normals. Only used by final SSAO pass. */
		SPtr<Texture> sceneNormals;

		/** Precalculated texture containing downsampled normals/depth, to be used for AO input. */
		SPtr<Texture> aoSetup;

		/** Texture containing AO from the previous pass. Only used if upsampling is enabled. */
		SPtr<Texture> aoDownsampled;

		/** Tileable texture containing random rotations that will be applied to AO samples. */
		SPtr<Texture> randomRotations;
	};

	/** 
	 * Shader that computes ambient occlusion using screen based methods.
	 *
	 * @tparam	UPSAMPLE	If true the shader will blend the calculated AO with AO data from the previous pass.
	 * @tparam	FINAL_PASS	If true the shader will use the full screen normal/depth information and perform
	 *						intensity scaling, as well as distance fade. Otherwise the shader will use the
	 *						downsampled AO setup information, with no scaling/fade.
	 * @tparam	QUALITY		Integer in range [0, 4] that controls the quality of SSAO sampling. Higher numbers yield
	 *						better quality at the cost of performance. 
	 */
	template<bool UPSAMPLE, bool FINAL_PASS, int QUALITY>
	class SSAOMat : public RendererMaterial<SSAOMat<UPSAMPLE, FINAL_PASS, QUALITY>>
	{
		RMAT_DEF("PPSSAO.bsl");

	public:
		SSAOMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	textures		Set of textures to be used as input. Which textures are used depends on the
		 *								template parameters of this class.
		 * @param[in]	destination		Output texture to which to write the ambient occlusion data to.
		 * @param[in]	settings		Settings used to control the ambient occlusion effect.
		 */
		void execute(const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, 
			const AmbientOcclusionSettings& settings);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mDepthTexture;
		GpuParamTexture mNormalsTexture;
		GpuParamTexture mDownsampledAOTexture;
		GpuParamTexture mSetupAOTexture;
		GpuParamTexture mRandomTexture;
	};

	BS_PARAM_BLOCK_BEGIN(SSAODownsampleParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gPixelSize)
		BS_PARAM_BLOCK_ENTRY(float, gInvDepthThreshold)
	BS_PARAM_BLOCK_END

	extern SSAODownsampleParamDef gSSAODownsampleParamDef;

	/** 
	 * Shader that downsamples the depth & normal buffer and stores their results in a common texture, to be consumed
	 * by SSAOMat. 
	 */
	class SSAODownsampleMat : public RendererMaterial<SSAODownsampleMat>
	{
		RMAT_DEF("PPSSAODownsample.bsl");

	public:
		SSAODownsampleMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	sceneDepth		Input texture containing scene depth.
		 * @param[in]	sceneNormals	Input texture containing scene world space normals.
		 * @param[in]	destination		Output texture to which to write the downsampled data to.
		 * @param[in]	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
		 */
		void execute(const RendererView& view, const SPtr<Texture>& sceneDepth, const SPtr<Texture>& sceneNormals,
			const SPtr<RenderTexture>& destination, float depthRange);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mDepthTexture;
		GpuParamTexture mNormalsTexture;
	};

	BS_PARAM_BLOCK_BEGIN(SSAOBlurParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gPixelSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gPixelOffset)
		BS_PARAM_BLOCK_ENTRY(float, gInvDepthThreshold)
	BS_PARAM_BLOCK_END

	extern SSAOBlurParamDef gSSAOBlurParamDef;

	/** 
	 * Shaders that blurs the ambient occlusion output, in order to hide the noise caused by the randomization texture.
	 */
	template<bool HORIZONTAL>
	class SSAOBlurMat : public RendererMaterial<SSAOBlurMat<HORIZONTAL>>
	{
		RMAT_DEF("PPSSAOBlur.bsl");

	public:
		SSAOBlurMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	ao				Input texture containing ambient occlusion data to be blurred.
		 * @param[in]	sceneDepth		Input texture containing scene depth.
		 * @param[in]	destination		Output texture to which to write the blurred data to.
		 * @param[in]	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
		 */
		void execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& sceneDepth,
			const SPtr<RenderTexture>& destination, float depthRange);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mAOTexture;
		GpuParamTexture mDepthTexture;
	};

	/** Helper class that is used for calculating the SSAO information. */
	class SSAO
	{
	public:
		SSAO();

		/** 
		 * Calculates SSAO for the specified view. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	destination		Output texture to which to write the SSAO data to.
		 * @param[in]	settings		Settings that control how is SSAO calculated.
		 */
		void execute(const RendererView& view, const SPtr<RenderTexture>& destination, 
			const AmbientOcclusionSettings& settings);

		/**
		 * Generates a texture that is used for randomizing sample locations during SSAO calculation. The texture contains
		 * 16 different rotations in a 4x4 tile.
		 */
		SPtr<Texture> generate4x4RandomizationTexture() const;

	private:
		/** @copydoc SSAOMat::execute() */
		void executeSSAOMat(bool upsample, bool final, int quality, const RendererView& view, 
			const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, 
			const AmbientOcclusionSettings& settings);

		SSAODownsampleMat mDownsample;
		SSAOBlurMat<true> mBlurHorz;
		SSAOBlurMat<false> mBlurVert;
		SPtr<Texture> mSSAORandomizationTex;

#define DEFINE_MATERIAL(QUALITY)							\
		SSAOMat<false, false, QUALITY> mSSAO_FF_##QUALITY;	\
		SSAOMat<true, false, QUALITY> mSSAO_TF_##QUALITY;	\
		SSAOMat<false, true, QUALITY> mSSAO_FT_##QUALITY;	\
		SSAOMat<true, true, QUALITY> mSSAO_TT_##QUALITY;	\

		DEFINE_MATERIAL(0)
		DEFINE_MATERIAL(1)
		DEFINE_MATERIAL(2)
		DEFINE_MATERIAL(3)
		DEFINE_MATERIAL(4)

#undef DEFINE_MATERIAL
	};

	BS_PARAM_BLOCK_BEGIN(SSRStencilParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gRoughnessScaleBias)
	BS_PARAM_BLOCK_END

	extern SSRStencilParamDef gSSRStencilParamDef;

	/** Shader used for marking which parts of the screen require screen space reflections. */
	class SSRStencilMat : public RendererMaterial<SSRStencilMat>
	{
		RMAT_DEF("PPSSRStencil.bsl");

	public:
		SSRStencilMat();

		/** 
		 * Renders the effect with the provided parameters, using the currently bound render target. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	settings		Parameters used for controling the SSR effect.
		 */
		void execute(const RendererView& view, const ScreenSpaceReflectionsSettings& settings);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GBufferParams mGBufferParams;
	};

	BS_PARAM_BLOCK_BEGIN(SSRTraceParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gHiZUVMapping)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gHiZSize)
		BS_PARAM_BLOCK_ENTRY(int, gHiZNumMips)
	BS_PARAM_BLOCK_END

	extern SSRTraceParamDef gSSRTraceParamDef;

	/** Shader used for tracing rays for screen space reflections. */
	class SSRTraceMat : public RendererMaterial<SSRTraceMat>
	{
		RMAT_DEF("PPSSRTrace.bsl");

	public:
		SSRTraceMat();

		/** 
		 * Renders the effect with the provided parameters. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	settings		Parameters used for controling the SSR effect.
		 * @param[in]	destination		Output texture to which to write the results to.
		 */
		void execute(const RendererView& view, const ScreenSpaceReflectionsSettings& settings, 
			const SPtr<RenderTexture>& destination);

		/**
		 * Calculates a scale & bias that is used for transforming roughness into a fade out value. Anything that is below
		 * @p maxRoughness will have the fade value of 1. Values above @p maxRoughness is slowly fade out over a range that
		 * is 1/2 the length of @p maxRoughness.
		 */
		static Vector2 calcRoughnessFadeScaleBias(float maxRoughness);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GBufferParams mGBufferParams;
		GpuParamTexture mSceneColorTexture;
	};

	/**
	 * Renders post-processing effects for the provided render target.
	 *
	 * @note	Core thread only.
	 */
	class PostProcessing : public Module<PostProcessing>
	{
	public:
		/** 
		 * Renders post-processing effects for the provided render target. Resolves provided scene color texture into the
		 * view's final output render target. Once the method exits, final render target is guaranteed to be currently
		 * bound for rendering. 
		 */
		void postProcess(RendererView* viewInfo, const SPtr<RenderTargets>& renderTargets, float frameDelta);
		
		/**
		 * Populates the ambient occlusion texture of the specified view with screen-space ambient occlusion information.
		 * Ambient occlusion texture must be allocated on the view's render targets before calling this method.
		 */
		void buildSSAO(const RendererView& view);
	private:
		DownsampleMaterials mDownsample;
		EyeAdaptHistogramMat mEyeAdaptHistogram;
		EyeAdaptHistogramReduceMat mEyeAdaptHistogramReduce;
		EyeAdaptationMat mEyeAdaptation;

		CreateTonemapLUTMat mCreateLUT;
		TonemappingMaterials mTonemapping;
		GaussianDOF mGaussianDOF;
		FXAAMat mFXAA;
		SSAO mSSAO;
	};

	/** @} */
}}
