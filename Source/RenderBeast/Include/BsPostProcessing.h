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
	};

	BS_PARAM_BLOCK_BEGIN(DownsampleParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector2, gOffsets, 4)
	BS_PARAM_BLOCK_END

	extern DownsampleParamDef gDownsampleParamDef;

	/** Shader that downsamples a texture to half its size. */
	class DownsampleMat : public RendererMaterial<DownsampleMat>
	{
		RMAT_DEF("PPDownsample.bsl");

	public:
		DownsampleMat();

		/** Renders the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& target);

		/** Returns the downsample material variation matching the provided parameters. */
		static DownsampleMat* getVariation(UINT32 quality, bool msaa);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;

		static ShaderVariation VAR_LowQuality_NoMSAA;
		static ShaderVariation VAR_LowQuality_MSAA;
		static ShaderVariation VAR_HighQuality_NoMSAA;
		static ShaderVariation VAR_HighQuality_MSAA;
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
		void execute(const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& target);

		/** Calculates the number of thread groups that need to execute to cover the provided texture. */
		static Vector2I getThreadGroupCount(const SPtr<Texture>& target);

		/** 
		 * Returns a vector containing scale and offset (in that order) that will be applied to luminance values
		 * to determine their position in the histogram. 
		 */
		static Vector2 getHistogramScaleOffset(const AutoExposureSettings& settings);

		static const UINT32 THREAD_GROUP_SIZE_X = 8;
		static const UINT32 THREAD_GROUP_SIZE_Y = 8;
		
		static const UINT32 HISTOGRAM_NUM_TEXELS = (THREAD_GROUP_SIZE_X * THREAD_GROUP_SIZE_Y) / 4;
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mSceneColor;
		GpuParamLoadStoreTexture mOutputTex;

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
		void execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame,
			const SPtr<RenderTarget>& output);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mHistogramTex;
		GpuParamTexture mEyeAdaptationTex;
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
		void execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, float frameDelta, 
			const AutoExposureSettings& settings, float exposureScale);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();
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
		void execute(const SPtr<Texture>& output, const StandardPostProcessSettings& settings);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();

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
	class TonemappingMat : public RendererMaterial<TonemappingMat>
	{
		RMAT_DEF("PPTonemapping.bsl");

	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& colorLUT,
			const SPtr<RenderTarget>& output, const StandardPostProcessSettings& settings);

		/** Returns the material variation matching the provided parameters. */
		static TonemappingMat* getVariation(bool gammaOnly, bool autoExposure, bool MSAA);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mColorLUT;
		GpuParamTexture mEyeAdaptationTex;

		static ShaderVariation VAR_Gamma_AutoExposure_MSAA;
		static ShaderVariation VAR_Gamma_AutoExposure_NoMSAA;
		static ShaderVariation VAR_Gamma_NoAutoExposure_MSAA;
		static ShaderVariation VAR_Gamma_NoAutoExposure_NoMSAA;
		static ShaderVariation VAR_NoGamma_AutoExposure_MSAA;
		static ShaderVariation VAR_NoGamma_AutoExposure_NoMSAA;
		static ShaderVariation VAR_NoGamma_NoAutoExposure_MSAA;
		static ShaderVariation VAR_NoGamma_NoAutoExposure_NoMSAA;
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

	/** 
	 * Shader that masks pixels from the input color texture into one or two output textures. The masking is done by
	 * determining if the pixel falls into near or far unfocused plane, as determined by depth-of-field parameters. User
	 * can pick whether to output pixels just on the near plane, just on the far plane, or both.
	 *
	 */
	class GaussianDOFSeparateMat : public RendererMaterial<GaussianDOFSeparateMat>
	{
		RMAT_DEF("PPGaussianDOFSeparate.bsl");

	public:
		GaussianDOFSeparateMat();

		/** 
		 * Renders the post-process effect with the provided parameters. 
		 * 
		 * @param[in]	color		Input color texture to process.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering. 
		 */
		void execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, 
			const DepthOfFieldSettings& settings);

		/**
		 * Returns the texture generated after the shader was executed. Only valid to call this in-between calls to
		 * execute() & release(), with @p idx value 0 or 1.
		 */
		SPtr<PooledRenderTexture> getOutput(UINT32 idx);

		/**
		 * Releases the interally allocated output render textures. Must be called after each call to execute(), when the 
		 * caller is done using the textures.
		 */
		void release();

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param	near	If true, near plane pixels are output to the first render target.
		 * @param	far		If true, far plane pixels are output to the first render target. If @p near is also enabled, the
		 *					pixels are output to the second render target instead.
		 */
		static GaussianDOFSeparateMat* getVariation(bool near, bool far);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mColorTexture;
		GpuParamTexture mDepthTexture;

		SPtr<PooledRenderTexture> mOutput0;
		SPtr<PooledRenderTexture> mOutput1;

		static ShaderVariation VAR_Near_Far;
		static ShaderVariation VAR_NoNear_Far;
		static ShaderVariation VAR_Near_NoFar;
	};

	/** 
	 * Shader that combines pixels for near unfocused, focused and far unfocused planes, as calculated by 
	 * GaussianDOFSeparateMat. Outputs final depth-of-field filtered image.
	 */
	class GaussianDOFCombineMat : public RendererMaterial<GaussianDOFCombineMat>
	{
		RMAT_DEF("PPGaussianDOFCombine.bsl");

	public:
		GaussianDOFCombineMat();

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
		void execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, 
			const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, 
			const DepthOfFieldSettings& settings);

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param	near	If true, near plane pixels are read from the near plane texture, otherwise near plane is assumed
		 *					not to exist.
		 * @param	far		If true, far plane pixels are read from the far plane texture, otherwise far plane is assumed not
		 *					to exist.
		 */
		static GaussianDOFCombineMat* getVariation(bool near, bool far);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mFocusedTexture;
		GpuParamTexture mNearTexture;
		GpuParamTexture mFarTexture;
		GpuParamTexture mDepthTexture;

		static ShaderVariation VAR_Near_Far;
		static ShaderVariation VAR_NoNear_Far;
		static ShaderVariation VAR_Near_NoFar;
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

	/** Shader that computes ambient occlusion using screen based methods. */
	class SSAOMat : public RendererMaterial<SSAOMat>
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

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param	upsample	If true the shader will blend the calculated AO with AO data from the previous pass.
		 * @param	finalPass	If true the shader will use the full screen normal/depth information and perform
		 *						intensity scaling, as well as distance fade. Otherwise the shader will use the
		 *						downsampled AO setup information, with no scaling/fade.
		 * @param	quality		Integer in range [0, 4] that controls the quality of SSAO sampling. Higher numbers yield
		 *						better quality at the cost of performance. 
		 */
		static SSAOMat* getVariation(bool upsample, bool finalPass, int quality);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mDepthTexture;
		GpuParamTexture mNormalsTexture;
		GpuParamTexture mDownsampledAOTexture;
		GpuParamTexture mSetupAOTexture;
		GpuParamTexture mRandomTexture;

#define VARIATION(QUALITY) \
		static ShaderVariation VAR_Upsample_Final_Quality##QUALITY;		\
		static ShaderVariation VAR_Upsample_NoFinal_Quality##QUALITY;		\
		static ShaderVariation VAR_NoUpsample_Final_Quality##QUALITY;		\
		static ShaderVariation VAR_NoUpsample_NoFinal_Quality##QUALITY;		\

		VARIATION(0)
		VARIATION(1)
		VARIATION(2)
		VARIATION(3)
		VARIATION(4)

#undef VARIATION
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
	class SSAOBlurMat : public RendererMaterial<SSAOBlurMat>
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

		/** Returns the material variation matching the provided parameters. */
		static SSAOBlurMat* getVariation(bool horizontal);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mAOTexture;
		GpuParamTexture mDepthTexture;

		static ShaderVariation VAR_Vertical;
		static ShaderVariation VAR_Horizontal;
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
		 * @param[in]	gbuffer			GBuffer textures.
		 * @param[in]	settings		Parameters used for controling the SSR effect.
		 */
		void execute(const RendererView& view, GBufferInput gbuffer, const ScreenSpaceReflectionsSettings& settings);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GBufferParams mGBufferParams;
	};

	BS_PARAM_BLOCK_BEGIN(SSRTraceParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gNDCToHiZUV)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHiZUVToScreenUV)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gHiZSize)
		BS_PARAM_BLOCK_ENTRY(int, gHiZNumMips)
		BS_PARAM_BLOCK_ENTRY(float, gIntensity)
		BS_PARAM_BLOCK_ENTRY(Vector2, gRoughnessScaleBias)
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
		 * @param[in]	gbuffer			GBuffer textures.
		 * @param[in]	sceneColor		Scene color texture.
		 * @param[in]	hiZ				Hierarchical Z buffer.
		 * @param[in]	settings		Parameters used for controling the SSR effect.
		 * @param[in]	destination		Render target to which to write the results to.
		 */
		void execute(const RendererView& view, GBufferInput gbuffer, const SPtr<Texture>& sceneColor, 
			const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings, 
			const SPtr<RenderTarget>& destination);

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
		GpuParamTexture mHiZTexture;
	};

	BS_PARAM_BLOCK_BEGIN(TemporalResolveParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(float, gSampleWeights, 9)
		BS_PARAM_BLOCK_ENTRY_ARRAY(float, gSampleWeightsLowpass, 9)
	BS_PARAM_BLOCK_END

	extern TemporalResolveParamDef gTemporalResolveParamDef;

	BS_PARAM_BLOCK_BEGIN(SSRResolveParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSceneDepthTexelSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gSceneColorTexelSize)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposure)
	BS_PARAM_BLOCK_END

	extern SSRResolveParamDef gSSRResolveParamDef;

	/** Shader used for combining SSR information from the previous frame, in order to yield better quality. */
	class SSRResolveMat : public RendererMaterial<SSRResolveMat>
	{
		RMAT_DEF("PPSSRResolve.bsl");

	public:
		SSRResolveMat();

		/** 
		 * Renders the effect with the provided parameters. 
		 * 
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	prevFrame		SSR data calculated previous frame.
		 * @param[in]	curFrame		SSR data calculated this frame.
		 * @param[in]	sceneDepth		Buffer containing scene depth.
		 * @param[in]	destination		Render target to which to write the results to.
		 */
		void execute(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, 
			const SPtr<Texture>& sceneDepth, const SPtr<RenderTarget>& destination);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param	eyeAdaptation	When true the shader will expect a texture containing an exposure value calculated by
		 *							the eye adaptation shader. Otherwise the manually provided exposure value is used
		 *							instead.
		 */
		SSRResolveMat* getVariation(bool eyeAdaptation);

	private:
		SPtr<GpuParamBlockBuffer> mSSRParamBuffer;
		SPtr<GpuParamBlockBuffer> mTemporalParamBuffer;

		GpuParamTexture mSceneColorTexture;
		GpuParamTexture mPrevColorTexture;
		GpuParamTexture mSceneDepthTexture;
		GpuParamTexture mEyeAdaptationTexture;

		static ShaderVariation VAR_EyeAdaptation;
		static ShaderVariation VAR_NoEyeAdaptation;
	};

	/** @} */
}}
