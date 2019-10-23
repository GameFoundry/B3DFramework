//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRenderSettings.h"
#include "Renderer/BsGpuResourcePool.h"
#include "BsRendererLight.h"

namespace bs { namespace ct
{
	struct RendererViewTargetData;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(DownsampleParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector2, gOffsets, 4)
	BS_PARAM_BLOCK_END

	extern DownsampleParamDef gDownsampleParamDef;

	/** Shader that downsamples a texture to half its size. */
	class DownsampleMat : public RendererMaterial<DownsampleMat>
	{
		RMAT_DEF("PPDownsample.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 quality, bool MSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("QUALITY", quality),
				ShaderVariation::Param("MSAA", MSAA)
			});

			return variation;
		};

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
		RMAT_DEF_CUSTOMIZED("PPEyeAdaptHistogram.bsl");

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
		RMAT_DEF_CUSTOMIZED("PPEyeAdaptation.bsl");

	public:
		EyeAdaptationMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, float frameDelta,
			const AutoExposureSettings& settings, float exposureScale);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();

		/**
		 * Populates the provided paramater buffer with eye adaptation parameters. The parameter buffer is expected to be
		 * created with EyeAdaptationParamDef block definition.
		 */
		static void populateParams(const SPtr<GpuParamBlockBuffer>& paramBuffer, float frameDelta,
			const AutoExposureSettings& settings, float exposureScale);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mReducedHistogramTex;
	};

	/**
	 * Shader that computes luminance of all the pixels in the provided texture, and stores them in log2 format, scaled
	 * to [0, 1] range (according to eye adapatation parameters) and stores those values in the alpha channel of the
	 * output texture. Color channel is just a copy of the input texture. Resulting texture is intended to be provided
	 * to the downsampling shader in order to calculate the average luminance, used for non-histogram eye adaptation
	 * calculation (when compute shader is not available).
	 */
	class EyeAdaptationBasicSetupMat : public RendererMaterial<EyeAdaptationBasicSetupMat>
	{
		RMAT_DEF("PPEyeAdaptationBasicSetup.bsl");

	public:
		EyeAdaptationBasicSetupMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output, float frameDelta,
			const AutoExposureSettings& settings, float exposureScale);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& input);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTex;
	};

	BS_PARAM_BLOCK_BEGIN(EyeAdaptationBasicParamsMatDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gInputTexSize)
	BS_PARAM_BLOCK_END

	extern EyeAdaptationBasicParamsMatDef gEyeAdaptationBasicParamsMatDef;

	/**
	 * Shader that computes eye adapatation value from a texture that has luminance encoded in its alpha channel (as done
	 * by EyeAdaptationBasicSetupMat). The result is a 1x1 texture containing the eye adaptation value.
	 */
	class EyeAdaptationBasicMat : public RendererMaterial<EyeAdaptationBasicMat>
	{
		RMAT_DEF("PPEyeAdaptationBasic.bsl");

	public:
		EyeAdaptationBasicMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output,
			float frameDelta, const AutoExposureSettings& settings, float exposureScale);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();
	private:
		SPtr<GpuParamBlockBuffer> mEyeAdaptationParamsBuffer;
		SPtr<GpuParamBlockBuffer> mParamsBuffer;
		GpuParamTexture mCurFrameTexParam;
		GpuParamTexture mPrevFrameTexParam;
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
		RMAT_DEF_CUSTOMIZED("PPCreateTonemapLUT.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool is3D>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("VOLUME_LUT", is3D),
			});

			return variation;
		};

	public:
		CreateTonemapLUTMat();

		/**
		 * Executes the post-process effect with the provided parameters, generating a 3D LUT using a compute shader.
		 * Should only be called on the appropriate variation (3D one).
		 */
		void execute3D(const SPtr<Texture>& output, const RenderSettings& settings);

		/**
		 * Executes the post-process effect with the provided parameters, generating an unwrapped 2D LUT without the use
		 * of a compute shader. Should only be called on the appropriate variation (non-3D one).
		 */
		void execute2D(const SPtr<RenderTexture>& output, const RenderSettings& settings);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		POOLED_RENDER_TEXTURE_DESC getOutputDesc() const;

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	is3D	If true the material will generate a 3D LUT using a compute shader. Otherwise it will
		 *						generate an unwrapped 2D LUT withou the use of a compute shader. Depending on this parameter
		 *						you should call either execute3D() or execute2D() methods to render the material.
		 */
		static CreateTonemapLUTMat* getVariation(bool is3D);

		/** Size of the 3D color lookup table. */
		static const UINT32 LUT_SIZE = 32;
	private:
		/** Populates the parameter block buffers using the provided settings. */
		void populateParamBuffers(const RenderSettings& settings);

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mWhiteBalanceParamBuffer;

		GpuParamLoadStoreTexture mOutputTex;
		bool mIs3D;
	};

	BS_PARAM_BLOCK_BEGIN(TonemappingParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gRawGamma)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
		BS_PARAM_BLOCK_ENTRY(Vector2, gTexSize)
		BS_PARAM_BLOCK_ENTRY(Color, gBloomTint)
		BS_PARAM_BLOCK_ENTRY(int, gNumSamples)
	BS_PARAM_BLOCK_END

	extern TonemappingParamDef gTonemappingParamDef;

	/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
	class TonemappingMat : public RendererMaterial<TonemappingMat>
	{
		RMAT_DEF_CUSTOMIZED("PPTonemapping.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("VOLUME_LUT", volumeLUT),
				ShaderVariation::Param("GAMMA_ONLY", gammaOnly),
				ShaderVariation::Param("AUTO_EXPOSURE", autoExposure),
				ShaderVariation::Param("MSAA", MSAA),
			});

			return variation;
		}
	public:
		TonemappingMat();

		/** Executes the post-process effect with the provided parameters. */
		void execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom,
			const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output, const RenderSettings& settings);

		/** Returns the material variation matching the provided parameters. */
		static TonemappingMat* getVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mBloomTex;
		GpuParamTexture mColorLUT;
		GpuParamTexture mEyeAdaptationTex;
	};	
	
	BS_PARAM_BLOCK_BEGIN(BloomClipParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gThreshold)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
	BS_PARAM_BLOCK_END

	extern BloomClipParamDef gBloomClipParamDef;

	/** Shader that clips parts of the image that shouldn't be affected by bloom (parts that aren't bright enough). */
	class BloomClipMat : public RendererMaterial<BloomClipMat>
	{
		RMAT_DEF("PPBloomClip.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool AUTO_EXPOSURE>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("AUTO_EXPOSURE", AUTO_EXPOSURE)
			});

			return variation;
		}
	public:
		BloomClipMat();

		/**
		 * Executes the post-process effect with the provided parameters and writes the results to the currently bound
		 * render target.
		 *
		 * @param[in]	input			Texture to process.
		 * @param[in]	threshold		Threshold below which values will be ignored for purposes of bloom.
		 * @param[in]	eyeAdaptation	Texture containing eye adaptation exposure value. Only needed if using the
		 *								AUTO_EXPOSURE variation of this material.
		 * @param[in]	settings		Render settings for the current view.
		 * @param[in]	output			Render target to write the results to.
		 */
		void execute(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation,
			const RenderSettings& settings, const SPtr<RenderTarget>& output);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	autoExposure	If true the exposure value will need to be provided in a texture output from the
		 *								eye adaptation material. Otherwise manual exposure scale from render settings will
		 *								be used.
		 */
		static BloomClipMat* getVariation(bool autoExposure);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mEyeAdaptationTex;
	};

	BS_PARAM_BLOCK_BEGIN(ScreenSpaceLensFlareParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gThreshold)
		BS_PARAM_BLOCK_ENTRY(float, gGhostSpacing)
		BS_PARAM_BLOCK_ENTRY(INT32, gGhostCount)
		BS_PARAM_BLOCK_ENTRY(float, gHaloRadius)
		BS_PARAM_BLOCK_ENTRY(float, gHaloThickness)
		BS_PARAM_BLOCK_ENTRY(float, gHaloThreshold)
		BS_PARAM_BLOCK_ENTRY(float, gHaloAspectRatio)
		BS_PARAM_BLOCK_ENTRY(float, gChromaticAberration)
	BS_PARAM_BLOCK_END

	extern ScreenSpaceLensFlareParamDef gScreenSpaceLensFlareParamDef;

	/** Generates ghost features from bright areas of an image, required for screen space lens flare rendering. */
	class ScreenSpaceLensFlareMat : public RendererMaterial<ScreenSpaceLensFlareMat>
	{
		RMAT_DEF("PPScreenSpaceLensFlare.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 HALO_MODE, bool CHROMATIC_ABERRATION>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("HALO_MODE", HALO_MODE),
				ShaderVariation::Param("CHROMATIC_ABERRATION", CHROMATIC_ABERRATION)
			});

			return variation;
		}
	public:
		ScreenSpaceLensFlareMat();

		/**
		 * Executes the post-process effect with the provided parameters and writes the results to the provided
		 * render target.
		 *
		 * @param[in]	input		Texture to process.
		 * @param[in]	settings	Settings used for customizing the effect.
		 * @param[in]	output		Render target to write the results to.
		 */
		void execute(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings,
			const SPtr<RenderTarget>& output);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	halo					If true the effect will render a halo as well as a normal lens flare.
		 * @param[in]	haloAspect				If true, user can set a custom aspect ratio for the rendered halo.
		 *										Only relevant if @p halo is turned on.
		 * @param[in]	chromaticAberration		If true, lens flare and halo (if enabled) features will be rendered by
		 *										splitting the red, green and blue channels according to a user-provided
		 *										offset.
		 */
		static ScreenSpaceLensFlareMat* getVariation(bool halo, bool haloAspect, bool chromaticAberration);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mGradientTex;
	};

	BS_PARAM_BLOCK_BEGIN(ChromaticAberrationParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInputSize)
		BS_PARAM_BLOCK_ENTRY(float, gShiftAmount)
	BS_PARAM_BLOCK_END

	extern ChromaticAberrationParamDef gChromaticAberrationParamDef;

	/** Renders a chromatic aberration effect by shifting RGB color channels. */
	class ChromaticAberrationMat : public RendererMaterial<ChromaticAberrationMat>
	{
		RMAT_DEF_CUSTOMIZED("PPChromaticAberration.bsl");

		static constexpr int MAX_SAMPLES = 16;

		/** Helper method used for initializing variations of this material. */
		template<bool SIMPLE>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
				{
					ShaderVariation::Param("SIMPLE", SIMPLE)
				});

			return variation;
		}
	public:
		ChromaticAberrationMat();

		/**
		 * Executes the post-process effect with the provided parameters and writes the results to the provided
		 * render target.
		 *
		 * @param[in]	input		Texture to process.
		 * @param[in]	settings	Settings used for customizing the effect.
		 * @param[in]	output		Render target to write the results to.
		 */
		void execute(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings,
			const SPtr<RenderTarget>& output);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	type		Type that determines how is the effect performed.
		 */
		static ChromaticAberrationMat* getVariation(ChromaticAberrationType type);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
		GpuParamTexture mFringeTex;
	};

	BS_PARAM_BLOCK_BEGIN(FilmGrainParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gIntensity)
		BS_PARAM_BLOCK_ENTRY(float, gTime)
	BS_PARAM_BLOCK_END

	extern FilmGrainParamDef gFilmGrainParamDef;

	/** Renders a film grain effect using a noise function. */
	class FilmGrainMat : public RendererMaterial<FilmGrainMat>
	{
		RMAT_DEF("PPFilmGrain.bsl");

	public:
		FilmGrainMat();

		/**
		 * Executes the post-process effect with the provided parameters and writes the results to the provided
		 * render target.
		 *
		 * @param[in]	input		Texture to process.
		 * @param[in]	time		Time of the current frame, in seconds.
		 * @param[in]	settings	Settings used for customizing the effect.
		 * @param[in]	output		Render target to write the results to.
		 */
		void execute(const SPtr<Texture>& input, float time, const FilmGrainSettings& settings,
			const SPtr<RenderTarget>& output);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;

		GpuParamTexture mInputTex;
	};

	const int MAX_BLUR_SAMPLES = 128;

	BS_PARAM_BLOCK_BEGIN(GaussianBlurParamDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleOffsets, (MAX_BLUR_SAMPLES + 1) / 2)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleWeights, MAX_BLUR_SAMPLES)
		BS_PARAM_BLOCK_ENTRY(int, gNumSamples)
	BS_PARAM_BLOCK_END

	extern GaussianBlurParamDef gGaussianBlurParamDef;

	/** Shader that performs Gaussian blur filtering on the provided texture. */
	class GaussianBlurMat : public RendererMaterial<GaussianBlurMat>
	{
		RMAT_DEF_CUSTOMIZED("PPGaussianBlur.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool ADDITIVE>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("ADDITIVE", ADDITIVE),
			});

			return variation;
		}
	public:
		// Direction of the Gaussian filter pass
		enum Direction
		{
			DirVertical,
			DirHorizontal
		};

		GaussianBlurMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	source		Input texture to blur.
		 * @param[in]	filterSize	Size of the blurring filter, in percent of the source texture. In range [0, 1].
		 * @param[in]	destination	Output texture to which to write the blurred image to.
		 * @param[in]	tint		Optional tint to apply all filtered pixels.
		 * @param[in]	additive	Optional texture whose values to add to the destination texture (won't be included
		 *							in filtering). Only used if using the variation of this shader that supports additive
		 *							input.
		 */
		void execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination,
			const Color& tint = Color::White, const SPtr<Texture>& additive = nullptr);

		/**
		 * Populates the provided parameter buffer with parameters required for a shader including gaussian blur.
		 *
		 * @param[in]	buffer		Buffer to write the parameters to. Must be created using @p GaussianBlurParamDef.
		 * @param[in]	direction	Direction in which to perform the separable blur.
		 * @param[in]	source		Source texture that needs to be blurred.
		 * @param[in]	filterSize	Size of the blurring filter, in percent of the source texture. In range [0, 1].
		 * @param[in]	tint		Optional tint to apply all filtered pixels.
		 */
		static void populateBuffer(const SPtr<GpuParamBlockBuffer>& buffer, Direction direction,
			const SPtr<Texture>& source, float filterSize, const Color& tint = Color::White);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	additive	If true the returned variation will support and additional input texture that will be
		 *							added on top of the filtered output.
		 */
		static GaussianBlurMat* getVariation(bool additive);
	private:
		/** Calculates weights and offsets for the standard distribution of the specified filter size. */
		static UINT32 calcStdDistribution(float filterRadius, std::array<float, MAX_BLUR_SAMPLES>& weights,
			std::array<float, MAX_BLUR_SAMPLES>& offsets);

		/** Calculates the radius of the blur kernel depending on the source texture size and provided scale. */
		static float calcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir);

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
		GpuParamTexture mAdditiveTexture;
		bool mIsAdditive = false;
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

		/** Helper method used for initializing variations of this material. */
		template<bool near, bool far>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("NEAR", near),
				ShaderVariation::Param("FAR", far)
			});

			return variation;
		}
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
	};

	/**
	 * Shader that combines pixels for near unfocused, focused and far unfocused planes, as calculated by
	 * GaussianDOFSeparateMat. Outputs final depth-of-field filtered image.
	 */
	class GaussianDOFCombineMat : public RendererMaterial<GaussianDOFCombineMat>
	{
		RMAT_DEF("PPGaussianDOFCombine.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool near, bool far>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("NEAR", near),
				ShaderVariation::Param("FAR", far),
			});

			return variation;
		}
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
	};

	BS_PARAM_BLOCK_BEGIN(DepthOfFieldCommonParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gFocalPlaneDistance)
		BS_PARAM_BLOCK_ENTRY(float, gApertureSize)
		BS_PARAM_BLOCK_ENTRY(float, gFocalLength)
		BS_PARAM_BLOCK_ENTRY(float, gInFocusRange)
		BS_PARAM_BLOCK_ENTRY(float, gSensorSize)
		BS_PARAM_BLOCK_ENTRY(float, gImageSize)
		BS_PARAM_BLOCK_ENTRY(float, gMaxBokehSize)
		BS_PARAM_BLOCK_ENTRY(float, gNearTransitionRegion)
		BS_PARAM_BLOCK_ENTRY(float, gFarTransitionRegion)
	BS_PARAM_BLOCK_END

	BS_PARAM_BLOCK_BEGIN(BokehDOFPrepareParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvInputSize)
	BS_PARAM_BLOCK_END

	/**
	 * Shader does a 2x texture downsample while accounting for different depth of field layers and encoding depth into
	 * the output.
	 */
	class BokehDOFPrepareMat : public RendererMaterial<BokehDOFPrepareMat>
	{
		RMAT_DEF("PPBokehDOFPrepare.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool MSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
				{
					ShaderVariation::Param("MSAA_COUNT", MSAA ? 2 : 1)
				});

			return variation;
		}
	public:
		BokehDOFPrepareMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	input		Input texture to downsample.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering.
		 * @param[in]	output		Texture to output the results to.
		 */
		void execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
			const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& target);

		/** Returns the material variation matching the provided parameters. */
		static BokehDOFPrepareMat* getVariation(bool msaa);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mCommonParamBuffer;
		GpuParamTexture mInputTexture;
		GpuParamTexture mDepthTexture;
	};

	BS_PARAM_BLOCK_BEGIN(BokehDOFParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gTileCount)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvInputSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gInvOutputSize)
		BS_PARAM_BLOCK_ENTRY(float, gAdaptiveThresholdColor)
		BS_PARAM_BLOCK_ENTRY(float, gAdaptiveThresholdCOC)
		BS_PARAM_BLOCK_ENTRY(Vector2, gBokehSize)
		BS_PARAM_BLOCK_ENTRY(int, gLayerPixelOffset)
		BS_PARAM_BLOCK_ENTRY(float, gInvDepthRange)
	BS_PARAM_BLOCK_END

	/** 
	 * Shader that renders the Bokeh DOF sprites and generates the blurred depth of field images. Separate images
	 * are generated for the near-field and in-focus + far-field.
	 */
	class BokehDOFMat : public RendererMaterial<BokehDOFMat>
	{
		RMAT_DEF_CUSTOMIZED("PPBokehDOF.bsl");
		
		/** Helper method used for initializing variations of this material. */
		template<bool DEPTH_OCCLUSION>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("DEPTH_OCCLUSION", DEPTH_OCCLUSION)
			});

			return variation;
		}
	public:
		static constexpr UINT32 NEAR_FAR_PADDING = 128;
		static constexpr UINT32 QUADS_PER_TILE = 8;

		BokehDOFMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	input		Input texture as generated by BokehDOFPrepare material..
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering.
		 * @param[in]	output		Texture to output the results to.
		 */
		void execute(const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings,
			const SPtr<RenderTarget>& output);

		/** Returns the texture descriptor that can be used for initializing the output render target. */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& target);

		/** Populates the common depth of field parameter buffers with values from the provided settings object. */
		static void populateDOFCommonParams(const SPtr<GpuParamBlockBuffer>& buffer, const DepthOfFieldSettings& settings,
			const RendererView& view);

		/** Returns the material variation matching the provided parameters. */
		static BokehDOFMat* getVariation(bool depthOcclusion);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mCommonParamBuffer;
		GpuParamTexture mInputTextureVS;
		GpuParamTexture mInputTextureFS;
		GpuParamTexture mBokehTexture;
		GpuParamTexture mDepthTexture;

		SPtr<VertexDeclaration> mTileVertexDecl;
		SPtr<IndexBuffer> mTileIndexBuffer;
		SPtr<VertexBuffer> mTileVertexBuffer;
	};

	BS_PARAM_BLOCK_BEGIN(BokehDOFCombineParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gLayerAndScaleOffset)
		BS_PARAM_BLOCK_ENTRY(Vector2, gFocusedImageSize)
	BS_PARAM_BLOCK_END

	/** Shader that combines the unfocused texture's near and far layers, together with the focused version. */
	class BokehDOFCombineMat : public RendererMaterial<BokehDOFCombineMat>
	{
		RMAT_DEF("PPBokehDOFCombine.bsl");

		/** Helper method used for initializing variations of this material. */
		template<MSAAMode MSAA_MODE>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("MSAA_MODE", (INT32)MSAA_MODE)
			});

			return variation;
		}
	public:
		BokehDOFCombineMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	unfocused	Unfocused and half-resolution texture as generated by the BokehDOF material, containing 
		 *							the near and far unfocused layers.
		 * @param[in]	focused		Focused full resolution scene color.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control depth of field rendering.
		 * @param[in]	output		Texture to output the results to.
		 */
		void execute(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth,
			const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);


		/** Returns the material variation matching the provided parameters. */
		static BokehDOFCombineMat* getVariation(MSAAMode msaaMode);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mCommonParamBuffer;
		GpuParamTexture mUnfocusedTexture;
		GpuParamTexture mFocusedTexture;
		GpuParamTexture mDepthTexture;
	};

	BS_PARAM_BLOCK_BEGIN(MotionBlurParamDef)
		BS_PARAM_BLOCK_ENTRY(UINT32, gHalfNumSamples)
	BS_PARAM_BLOCK_END

	/** Shader that blurs the scene depending on camera and/or object movement. */
	class MotionBlurMat : public RendererMaterial<MotionBlurMat>
	{
		RMAT_DEF("PPMotionBlur.bsl");

	public:
		MotionBlurMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	input		Input texture to blur.
		 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
		 * @param[in]	view		View through which the depth of field effect is viewed.
		 * @param[in]	settings	Settings used to control the motion blur effect.
		 * @param[in]	output		Texture to output the results to.
		 */
		void execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view,
			const MotionBlurSettings& settings, const SPtr<RenderTarget>& output);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
		GpuParamTexture mDepthTexture;
	};

	BS_PARAM_BLOCK_BEGIN(BuildHiZFParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHalfPixelOffset)
		BS_PARAM_BLOCK_ENTRY(int, gMipLevel)
	BS_PARAM_BLOCK_END

	extern BuildHiZFParamDef gBuildHiZFParamDef;

	/** Shader that calculates a single level of the hierarchical Z mipmap chain. */
	class BuildHiZMat : public RendererMaterial<BuildHiZMat>
	{
		RMAT_DEF("PPBuildHiZ.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool noTextureViews>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("NO_TEXTURE_VIEWS", noTextureViews),
			});

			return variation;
		}
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

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param	noTextureViews		Specify as true if the current render backend doesn't support texture views, in
		 *								which case the implementation falls back on using a simpler version of the shader.
		 */
		static BuildHiZMat* getVariation(bool noTextureViews);
	private:
		GpuParamTexture mInputTexture;
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		bool mNoTextureViews = false;
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

		/** Helper method used for initializing variations of this material. */
		template<bool upsample, bool finalPass, int quality>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("MIX_WITH_UPSAMPLED", upsample),
				ShaderVariation::Param("FINAL_AO", finalPass),
				ShaderVariation::Param("QUALITY", quality)
			});

			return variation;
		}
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

		/** Helper method used for initializing variations of this material. */
		template<bool horizontal>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("DIR_HORZ", horizontal)
			});

			return variation;
		}
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
	};

	BS_PARAM_BLOCK_BEGIN(SSRStencilParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2, gRoughnessScaleBias)
	BS_PARAM_BLOCK_END

	extern SSRStencilParamDef gSSRStencilParamDef;

	/** Shader used for marking which parts of the screen require screen space reflections. */
	class SSRStencilMat : public RendererMaterial<SSRStencilMat>
	{
		RMAT_DEF("PPSSRStencil.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("MSAA_COUNT", msaa ? 2 : 1),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		SSRStencilMat();

		/**
		 * Renders the effect with the provided parameters, using the currently bound render target.
		 *
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	gbuffer			GBuffer textures.
		 * @param[in]	settings		Parameters used for controling the SSR effect.
		 */
		void execute(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static SSRStencilMat* getVariation(bool msaa, bool singleSampleMSAA);
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
		BS_PARAM_BLOCK_ENTRY(int, gTemporalJitter)
	BS_PARAM_BLOCK_END

	extern SSRTraceParamDef gSSRTraceParamDef;

	/** Shader used for tracing rays for screen space reflections. */
	class SSRTraceMat : public RendererMaterial<SSRTraceMat>
	{
		RMAT_DEF("PPSSRTrace.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 quality, bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("MSAA_COUNT", msaa ? 2 : 1),
				ShaderVariation::Param("QUALITY", quality),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
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
		void execute(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor,
			const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings,
			const SPtr<RenderTarget>& destination);

		/**
		 * Calculates a scale & bias that is used for transforming roughness into a fade out value. Anything that is below
		 * @p maxRoughness will have the fade value of 1. Values above @p maxRoughness is slowly fade out over a range that
		 * is 1/2 the length of @p maxRoughness.
		 */
		static Vector2 calcRoughnessFadeScaleBias(float maxRoughness);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	quality				Determines how many rays to trace. In range [0, 4].
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static SSRTraceMat* getVariation(UINT32 quality, bool msaa, bool singleSampleMSAA = false);
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

	BS_PARAM_BLOCK_BEGIN(TemporalFilteringParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gSceneDepthTexelSize)
		BS_PARAM_BLOCK_ENTRY(Vector4, gSceneColorTexelSize)
		BS_PARAM_BLOCK_ENTRY(Vector4, gVelocityTexelSize)
		BS_PARAM_BLOCK_ENTRY(float, gManualExposure)
	BS_PARAM_BLOCK_END

	extern TemporalFilteringParamDef gTemporalFilteringParamDef;

	/** Supported filter types by TemporalFilteringMat. */
	enum class TemporalFilteringType
	{
		/** Temporal filter used for full screen anti-aliasing. */
		FullScreenAA,

		/** Temporal filter used for accumulating SSR samples over multiple frames. */
		SSR
	};

	/** Shader used for combining multiple frames of information using a temporal filter, in order to yield better quality. */
	class TemporalFilteringMat : public RendererMaterial<TemporalFilteringMat>
	{
		RMAT_DEF("TemporalFiltering.bsl");

		/** Helper method used for initializing variations of this material. */
		template<TemporalFilteringType TYPE, bool PER_PIXEL_VELOCITY, bool MSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("TYPE", (int)TYPE),
				ShaderVariation::Param("PER_PIXEL_VELOCITY", (int)PER_PIXEL_VELOCITY),
				ShaderVariation::Param("MSAA", MSAA),
			});

			return variation;
		}
	public:
		TemporalFilteringMat();

		/**
		 * Renders the effect with the provided parameters.
		 *
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	prevFrame		Frame data calculated previous frame.
		 * @param[in]	curFrame		Frame data calculated this frame.
		 * @param[in]	velocity		Optional texture containing per-pixel velocity;
		 * @param[in]	sceneDepth		Buffer containing scene depth.
		 * @param[in]	jitter			Sub-pixel jitter applied to the projection matrix.
		 * @param[in]	exposure		Exposure to use when transforming from HDR to LDR image.
		 * @param[in]	destination		Render target to which to write the results to.
		 */
		void execute(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame,
			const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter,
			float exposure, const SPtr<RenderTarget>& destination);

		/**
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param[in]	type		Type of filter to use.
		 * @param[in]	velocity	True if the filter will have access to a buffer containing per-pixel velocities.
		 * @param[in]	msaa		True if the shader will operate on a multisampled surface. Note that previous
		 *							and current frame color textures must be non-MSAA, regardless of this parameter.
		 * @return					Requested variation of the material.
		 */
		static TemporalFilteringMat* getVariation(TemporalFilteringType type, bool velocity, bool msaa);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mTemporalParamBuffer;

		GpuParamTexture mSceneColorTexture;
		GpuParamTexture mPrevColorTexture;
		GpuParamTexture mSceneDepthTexture;
		GpuParamTexture mVelocityTexture;

		bool mHasVelocityTexture = false;
	};

	BS_PARAM_BLOCK_BEGIN(EncodeDepthParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gNear)
		BS_PARAM_BLOCK_ENTRY(float, gFar)
	BS_PARAM_BLOCK_END

	extern EncodeDepthParamDef gEncodeDepthParamDef;

	/**
	 * Shader that encodes depth from a specified range into [0, 1] range, and writes the result in the alpha channel
	 * of the output texture.
	 */
	class EncodeDepthMat : public RendererMaterial<EncodeDepthMat>
	{
		RMAT_DEF("PPEncodeDepth.bsl");

	public:
		EncodeDepthMat();

		/**
		 * Renders the post-process effect with the provided parameters.
		 *
		 * @param[in]	depth		Resolved (non-MSAA) depth texture to encode.
		 * @param[in]	near		Near range (in view space) to start encoding the depth. Any depth lower than this will
		 *							be encoded to 1.
		 * @param[in]	far			Far range (in view space) to end encoding the depth. Any depth higher than this will
		 *							be encoded to 0.
		 * @param[in]	output		Output texture to write the results in. Results will be written in the alpha channel.
		 */
		void execute(const SPtr<Texture>& depth, float near, float far, const SPtr<RenderTarget>& output);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	/**
	 * Shader that outputs a texture that determines which pixels require per-sample evaluation. Only relevant when
	 * rendering with MSAA enabled.
	 */
	class MSAACoverageMat : public RendererMaterial<MSAACoverageMat>
	{
		RMAT_DEF("MSAACoverage.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 msaa>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation(
			{
				ShaderVariation::Param("MSAA_COUNT", msaa)
			});

			return variation;
		}
	public:
		MSAACoverageMat();

		/**
		 * Renders the effect with the provided parameters, using the currently bound render target.
		 *
		 * @param[in]	view			Information about the view we're rendering from.
		 * @param[in]	gbuffer			GBuffer textures.
		 */
		void execute(const RendererView& view, GBufferTextures gbuffer);

		/** Returns the material variation matching the provided parameters. */
		static MSAACoverageMat* getVariation(UINT32 msaaCount);
	private:
		GBufferParams mGBufferParams;
	};

	/**
	 * Converts the coverage texture output by MSAACoverageMat and writes its information in the highest bit of the
	 * currently bound stencil buffer. This allows coverage information to be used by normal (non-compute) rendering
	 * shaders.
	 */
	class MSAACoverageStencilMat : public RendererMaterial<MSAACoverageStencilMat>
	{
		RMAT_DEF("MSAACoverageStencil.bsl");

	public:
		MSAACoverageStencilMat();

		/**
		 * Renders the effect with the provided parameters, using the currently bound render target.
		 *
		 * @param[in]	view		Information about the view we're rendering from.
		 * @param[in]	coverage	Coverage texture as output by MSAACoverageMat.
		 */
		void execute(const RendererView& view, const SPtr<Texture>& coverage);
	private:
		GpuParamTexture mCoverageTexParam;
	};

	/** @} */
}}
