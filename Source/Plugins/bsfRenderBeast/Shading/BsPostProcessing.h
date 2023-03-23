//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "Renderer/BsRenderSettings.h"
#include "Renderer/BsGpuResourcePool.h"
#include "BsRendererLight.h"

namespace bs
{
	namespace ct
	{
		struct RendererViewTargetData;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_PARAM_BLOCK_BEGIN(DownsampleParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector2, gOffsets, 4)
		B3D_PARAM_BLOCK_END

		extern DownsampleParamDef gDownsampleParamDef;

		/** Shader that downsamples a texture to half its size. */
		class DownsampleMat : public RendererMaterial<DownsampleMat>
		{
			RMAT_DEF("PPDownsample.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 quality, bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("QUALITY", quality),
					  ShaderVariationParameter("MSAA", MSAA) });

				return variation;
			};

		public:
			DownsampleMat() = default;
			void Initialize() override;

			/** Renders the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc(const SPtr<Texture>& target);

			/** Returns the downsample material variation matching the provided parameters. */
			static DownsampleMat* GetVariation(u32 quality, bool msaa);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(EyeAdaptHistogramParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector4I, gPixelOffsetAndSize)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gHistogramParams)
			B3D_PARAM_BLOCK_ENTRY(Vector2I, gThreadGroupCount)
		B3D_PARAM_BLOCK_END

		extern EyeAdaptHistogramParamDef gEyeAdaptHistogramParamDef;

		/** Shader that creates a luminance histogram used for eye adaptation. */
		class EyeAdaptHistogramMat : public RendererMaterial<EyeAdaptHistogramMat>
		{
			RMAT_DEF_CUSTOMIZED("PPEyeAdaptHistogram.bsl");

		public:
			EyeAdaptHistogramMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc(const SPtr<Texture>& target);

			/** Calculates the number of thread groups that need to execute to cover the provided texture. */
			static Vector2I GetThreadGroupCount(const SPtr<Texture>& target);

			/**
			 * Returns a vector containing scale and offset (in that order) that will be applied to luminance values
			 * to determine their position in the histogram.
			 */
			static Vector2 GetHistogramScaleOffset(const AutoExposureSettings& settings);

			static const u32 kThreadGroupSizeX = 8;
			static const u32 kThreadGroupSizeY = 8;

			static const u32 kHistogramNumTexels = (kThreadGroupSizeX * kThreadGroupSizeY) / 4;

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mSceneColor;
			GpuParameterStorageTexture mOutputTex;

			static const u32 kLoopCountX = 8;
			static const u32 kLoopCountY = 8;
		};

		B3D_PARAM_BLOCK_BEGIN(EyeAdaptHistogramReduceParamDef)
			B3D_PARAM_BLOCK_ENTRY(int, gThreadGroupCount)
		B3D_PARAM_BLOCK_END

		extern EyeAdaptHistogramReduceParamDef gEyeAdaptHistogramReduceParamDef;

		/** Shader that reduces the luminance histograms created by EyeAdaptHistogramMat into a single histogram. */
		class EyeAdaptHistogramReduceMat : public RendererMaterial<EyeAdaptHistogramReduceMat>
		{
			RMAT_DEF("PPEyeAdaptHistogramReduce.bsl");

		public:
			EyeAdaptHistogramReduceMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc();

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mHistogramTex;
			GpuParameterSampledTexture mEyeAdaptationTex;
		};

		B3D_PARAM_BLOCK_BEGIN(EyeAdaptationParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gEyeAdaptationParams, 3)
		B3D_PARAM_BLOCK_END

		extern EyeAdaptationParamDef gEyeAdaptationParamDef;

		/** Shader that computes the eye adaptation value based on scene luminance. */
		class EyeAdaptationMat : public RendererMaterial<EyeAdaptationMat>
		{
			RMAT_DEF_CUSTOMIZED("PPEyeAdaptation.bsl");

		public:
			EyeAdaptationMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& reducedHistogram, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc();

			/**
			 * Populates the provided paramater buffer with eye adaptation parameters. The parameter buffer is expected to be
			 * created with EyeAdaptationParamDef block definition.
			 */
			static void PopulateParams(const SPtr<GpuBuffer>& paramBuffer, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mReducedHistogramTex;
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
			EyeAdaptationBasicSetupMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& input, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc(const SPtr<Texture>& input);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTex;
		};

		B3D_PARAM_BLOCK_BEGIN(EyeAdaptationBasicParamsMatDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2I, gInputTexSize)
		B3D_PARAM_BLOCK_END

		extern EyeAdaptationBasicParamsMatDef gEyeAdaptationBasicParamsMatDef;

		/**
		 * Shader that computes eye adapatation value from a texture that has luminance encoded in its alpha channel (as done
		 * by EyeAdaptationBasicSetupMat). The result is a 1x1 texture containing the eye adaptation value.
		 */
		class EyeAdaptationBasicMat : public RendererMaterial<EyeAdaptationBasicMat>
		{
			RMAT_DEF("PPEyeAdaptationBasic.bsl");

		public:
			EyeAdaptationBasicMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, const SPtr<RenderTarget>& output, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc();

		private:
			SPtr<GpuBuffer> mEyeAdaptationParamsBuffer;
			SPtr<GpuBuffer> mParamsBuffer;
			GpuParameterSampledTexture mCurFrameTexParam;
			GpuParameterSampledTexture mPrevFrameTexParam;
		};

		B3D_PARAM_BLOCK_BEGIN(CreateTonemapLUTParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gTonemapParams, 2)
			B3D_PARAM_BLOCK_ENTRY(float, gGammaAdjustment)
			B3D_PARAM_BLOCK_ENTRY(int, gGammaCorrectionType)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gSaturation)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gContrast)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gGain)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gOffset)
		B3D_PARAM_BLOCK_END

		extern CreateTonemapLUTParamDef gCreateTonemapLUTParamDef;

		B3D_PARAM_BLOCK_BEGIN(WhiteBalanceParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gWhiteTemp)
			B3D_PARAM_BLOCK_ENTRY(float, gWhiteOffset)
		B3D_PARAM_BLOCK_END

		extern WhiteBalanceParamDef gWhiteBalanceParamDef;

		/** Shader that creates a 2D lookup texture that is used to apply tonemapping, color grading, white balancing and gamma correction. Uses a vertex/fragment pipeline. */
		class CreateTonemap2DLUTMat : public RendererMaterial<CreateTonemap2DLUTMat>
		{
			RMAT_DEF_CUSTOMIZED("PPCreateTonemap2DLUT.bsl")

		public:
			CreateTonemap2DLUTMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters, generating an unwrapped 2D LUT using the vertex & fragment shader pipeline. */
			void Execute(const SPtr<RenderTexture>& output, const RenderSettings& settings);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			POOLED_RENDER_TEXTURE_DESC GetOutputDesc() const;

			/** Populates a parameter block buffer of CreateTonemapLUTParamDef type using the provided settings. */
			static void PopulateTonemappingParameterBuffer(const RenderSettings& settings, const SPtr<GpuBuffer>& parameterBuffer);

			/** Populates a parameter block buffer of WhiteBalanceParamDef type using the provided settings. */
			static void PopulateWhiteBalanceParameterBuffer(const RenderSettings& settings, const SPtr<GpuBuffer>& parameterBuffer);

			/** Size of a single dimension in the color lookup table. */
			static const u32 kLutSize = 32;

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mWhiteBalanceParamBuffer;
		};

		/** Shader that creates a 3D lookup texture that is used to apply tonemapping, color grading, white balancing and gamma correction. Uses a compute pipeline. */
		class CreateTonemap3DLUTMat : public RendererMaterial<CreateTonemap3DLUTMat>
		{
			RMAT_DEF_CUSTOMIZED("PPCreateTonemap3DLUT.bsl");

		public:
			CreateTonemap3DLUTMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters, generating a 3D LUT using a compute shader. */
			void Execute(const SPtr<Texture>& output, const RenderSettings& settings);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			POOLED_RENDER_TEXTURE_DESC GetOutputDesc() const;

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mWhiteBalanceParamBuffer;

			GpuParameterStorageTexture mOutputTex;
		};

		B3D_PARAM_BLOCK_BEGIN(TonemappingParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gRawGamma)
			B3D_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gTexSize)
			B3D_PARAM_BLOCK_ENTRY(Color, gBloomTint)
			B3D_PARAM_BLOCK_ENTRY(int, gNumSamples)
		B3D_PARAM_BLOCK_END

		extern TonemappingParamDef gTonemappingParamDef;

		/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
		class TonemappingMat : public RendererMaterial<TonemappingMat>
		{
			RMAT_DEF_CUSTOMIZED("PPTonemapping.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("VOLUME_LUT", volumeLUT),
						ShaderVariationParameter("GAMMA_ONLY", gammaOnly),
						ShaderVariationParameter("AUTO_EXPOSURE", autoExposure),
						ShaderVariationParameter("MSAA", MSAA),
					});

				return variation;
			}

		public:
			TonemappingMat() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const SPtr<RenderTarget>& output, const RenderSettings& settings);

			/** Returns the material variation matching the provided parameters. */
			static TonemappingMat* GetVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA);

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mInputTex;
			GpuParameterSampledTexture mBloomTex;
			GpuParameterSampledTexture mColorLUT;
			GpuParameterSampledTexture mEyeAdaptationTex;
		};

		B3D_PARAM_BLOCK_BEGIN(BloomClipParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gThreshold)
			B3D_PARAM_BLOCK_ENTRY(float, gManualExposureScale)
		B3D_PARAM_BLOCK_END

		extern BloomClipParamDef gBloomClipParamDef;

		/** Shader that clips parts of the image that shouldn't be affected by bloom (parts that aren't bright enough). */
		class BloomClipMat : public RendererMaterial<BloomClipMat>
		{
			RMAT_DEF("PPBloomClip.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool AUTO_EXPOSURE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("AUTO_EXPOSURE", AUTO_EXPOSURE) });

				return variation;
			}

		public:
			BloomClipMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation, const RenderSettings& settings, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	autoExposure	If true the exposure value will need to be provided in a texture output from the
			 *								eye adaptation material. Otherwise manual exposure scale from render settings will
			 *								be used.
			 */
			static BloomClipMat* GetVariation(bool autoExposure);

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mInputTex;
			GpuParameterSampledTexture mEyeAdaptationTex;
		};

		B3D_PARAM_BLOCK_BEGIN(ScreenSpaceLensFlareParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gThreshold)
			B3D_PARAM_BLOCK_ENTRY(float, gGhostSpacing)
			B3D_PARAM_BLOCK_ENTRY(i32, gGhostCount)
			B3D_PARAM_BLOCK_ENTRY(float, gHaloRadius)
			B3D_PARAM_BLOCK_ENTRY(float, gHaloThickness)
			B3D_PARAM_BLOCK_ENTRY(float, gHaloThreshold)
			B3D_PARAM_BLOCK_ENTRY(float, gHaloAspectRatio)
			B3D_PARAM_BLOCK_ENTRY(float, gChromaticAberration)
		B3D_PARAM_BLOCK_END

		extern ScreenSpaceLensFlareParamDef gScreenSpaceLensFlareParamDef;

		/** Generates ghost features from bright areas of an image, required for screen space lens flare rendering. */
		class ScreenSpaceLensFlareMat : public RendererMaterial<ScreenSpaceLensFlareMat>
		{
			RMAT_DEF("PPScreenSpaceLensFlare.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 HALO_MODE, bool CHROMATIC_ABERRATION>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("HALO_MODE", HALO_MODE),
					  ShaderVariationParameter("CHROMATIC_ABERRATION", CHROMATIC_ABERRATION) });

				return variation;
			}

		public:
			ScreenSpaceLensFlareMat() = default;
			void Initialize() override;

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param[in]	input		Texture to process.
			 * @param[in]	settings	Settings used for customizing the effect.
			 * @param[in]	output		Render target to write the results to.
			 */
			void Execute(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings, const SPtr<RenderTarget>& output);

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
			static ScreenSpaceLensFlareMat* GetVariation(bool halo, bool haloAspect, bool chromaticAberration);

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mInputTex;
			GpuParameterSampledTexture mGradientTex;
		};

		B3D_PARAM_BLOCK_BEGIN(ChromaticAberrationParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInputSize)
			B3D_PARAM_BLOCK_ENTRY(float, gShiftAmount)
		B3D_PARAM_BLOCK_END

		extern ChromaticAberrationParamDef gChromaticAberrationParamDef;

		/** Renders a chromatic aberration effect by shifting RGB color channels. */
		class ChromaticAberrationMat : public RendererMaterial<ChromaticAberrationMat>
		{
			RMAT_DEF_CUSTOMIZED("PPChromaticAberration.bsl");

			static constexpr int kMaxSamples = 16;

			/** Helper method used for initializing variations of this material. */
			template <bool SIMPLE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SIMPLE", SIMPLE) });

				return variation;
			}

		public:
			ChromaticAberrationMat() = default;
			void Initialize() override;

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param[in]	input		Texture to process.
			 * @param[in]	settings	Settings used for customizing the effect.
			 * @param[in]	output		Render target to write the results to.
			 */
			void Execute(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	type		Type that determines how is the effect performed.
			 */
			static ChromaticAberrationMat* GetVariation(ChromaticAberrationType type);

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mInputTex;
			GpuParameterSampledTexture mFringeTex;
		};

		B3D_PARAM_BLOCK_BEGIN(FilmGrainParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gIntensity)
			B3D_PARAM_BLOCK_ENTRY(float, gTime)
		B3D_PARAM_BLOCK_END

		extern FilmGrainParamDef gFilmGrainParamDef;

		/** Renders a film grain effect using a noise function. */
		class FilmGrainMat : public RendererMaterial<FilmGrainMat>
		{
			RMAT_DEF("PPFilmGrain.bsl");

		public:
			FilmGrainMat() = default;
			void Initialize() override;

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param[in]	input		Texture to process.
			 * @param[in]	time		Time of the current frame, in seconds.
			 * @param[in]	settings	Settings used for customizing the effect.
			 * @param[in]	output		Render target to write the results to.
			 */
			void Execute(const SPtr<Texture>& input, float time, const FilmGrainSettings& settings, const SPtr<RenderTarget>& output);

		private:
			SPtr<GpuBuffer> mParamBuffer;

			GpuParameterSampledTexture mInputTex;
		};

		const int kMaxBlurSamples = 128;

		B3D_PARAM_BLOCK_BEGIN(GaussianBlurParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleOffsets, (kMaxBlurSamples + 1) / 2)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Vector4, gSampleWeights, kMaxBlurSamples)
			B3D_PARAM_BLOCK_ENTRY(int, gNumSamples)
		B3D_PARAM_BLOCK_END

		extern GaussianBlurParamDef gGaussianBlurParamDef;

		/** Shader that performs Gaussian blur filtering on the provided texture. */
		class GaussianBlurMat : public RendererMaterial<GaussianBlurMat>
		{
			RMAT_DEF_CUSTOMIZED("PPGaussianBlur.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool ADDITIVE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("ADDITIVE", ADDITIVE),
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

			GaussianBlurMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination, const Color& tint = Color::kWhite, const SPtr<Texture>& additive = nullptr);

			/**
			 * Populates the provided parameter buffer with parameters required for a shader including gaussian blur.
			 *
			 * @param[in]	buffer		Buffer to write the parameters to. Must be created using @p GaussianBlurParamDef.
			 * @param[in]	direction	Direction in which to perform the separable blur.
			 * @param[in]	source		Source texture that needs to be blurred.
			 * @param[in]	filterSize	Size of the blurring filter, in percent of the source texture. In range [0, 1].
			 * @param[in]	tint		Optional tint to apply all filtered pixels.
			 */
			static void PopulateBuffer(const SPtr<GpuBuffer>& buffer, Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint = Color::kWhite);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	additive	If true the returned variation will support and additional input texture that will be
			 *							added on top of the filtered output.
			 */
			static GaussianBlurMat* GetVariation(bool additive);

		private:
			/** Calculates weights and offsets for the standard distribution of the specified filter size. */
			static u32 CalcStdDistribution(float filterRadius, std::array<float, kMaxBlurSamples>& weights, std::array<float, kMaxBlurSamples>& offsets);

			/** Calculates the radius of the blur kernel depending on the source texture size and provided scale. */
			static float CalcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir);

			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
			GpuParameterSampledTexture mAdditiveTexture;
			bool mIsAdditive = false;
		};

		B3D_PARAM_BLOCK_BEGIN(GaussianDOFParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gNearBlurPlane)
			B3D_PARAM_BLOCK_ENTRY(float, gFarBlurPlane)
			B3D_PARAM_BLOCK_ENTRY(float, gInvNearBlurRange)
			B3D_PARAM_BLOCK_ENTRY(float, gInvFarBlurRange)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gHalfPixelOffset)
		B3D_PARAM_BLOCK_END

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
			template <bool near, bool far>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("NEAR", near),
					  ShaderVariationParameter("FAR", far) });

				return variation;
			}

		public:
			GaussianDOFSeparateMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	color		Input color texture to process.
			 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
			 * @param[in]	view		View through which the depth of field effect is viewed.
			 * @param[in]	settings	Settings used to control depth of field rendering.
			 */
			void Execute(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Returns the texture generated after the shader was executed. Only valid to call this in-between calls to
			 * execute() & release(), with @p idx value 0 or 1.
			 */
			SPtr<PooledRenderTexture> GetOutput(u32 idx);

			/**
			 * Releases the interally allocated output render textures. Must be called after each call to execute(), when the
			 * caller is done using the textures.
			 */
			void Release();

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	near	If true, near plane pixels are output to the first render target.
			 * @param	far		If true, far plane pixels are output to the first render target. If @p near is also enabled, the
			 *					pixels are output to the second render target instead.
			 */
			static GaussianDOFSeparateMat* GetVariation(bool near, bool far);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mColorTexture;
			GpuParameterSampledTexture mDepthTexture;

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
			template <bool near, bool far>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("NEAR", near),
						ShaderVariationParameter("FAR", far),
					});

				return variation;
			}

		public:
			GaussianDOFCombineMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, const SPtr<Texture>& depth, const SPtr<RenderTarget>& output, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	near	If true, near plane pixels are read from the near plane texture, otherwise near plane is assumed
			 *					not to exist.
			 * @param	far		If true, far plane pixels are read from the far plane texture, otherwise far plane is assumed not
			 *					to exist.
			 */
			static GaussianDOFCombineMat* GetVariation(bool near, bool far);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mFocusedTexture;
			GpuParameterSampledTexture mNearTexture;
			GpuParameterSampledTexture mFarTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(DepthOfFieldCommonParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gFocalPlaneDistance)
			B3D_PARAM_BLOCK_ENTRY(float, gApertureSize)
			B3D_PARAM_BLOCK_ENTRY(float, gFocalLength)
			B3D_PARAM_BLOCK_ENTRY(float, gInFocusRange)
			B3D_PARAM_BLOCK_ENTRY(float, gSensorSize)
			B3D_PARAM_BLOCK_ENTRY(float, gImageSize)
			B3D_PARAM_BLOCK_ENTRY(float, gMaxBokehSize)
			B3D_PARAM_BLOCK_ENTRY(float, gNearTransitionRegion)
			B3D_PARAM_BLOCK_ENTRY(float, gFarTransitionRegion)
		B3D_PARAM_BLOCK_END

		B3D_PARAM_BLOCK_BEGIN(BokehDOFPrepareParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInvInputSize)
		B3D_PARAM_BLOCK_END

		/**
		 * Shader does a 2x texture downsample while accounting for different depth of field layers and encoding depth into
		 * the output.
		 */
		class BokehDOFPrepareMat : public RendererMaterial<BokehDOFPrepareMat>
		{
			RMAT_DEF("PPBokehDOFPrepare.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", MSAA ? 2 : 1) });

				return variation;
			}

		public:
			BokehDOFPrepareMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	input		Input texture to downsample.
			 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
			 * @param[in]	view		View through which the depth of field effect is viewed.
			 * @param[in]	settings	Settings used to control depth of field rendering.
			 * @param[in]	output		Texture to output the results to.
			 */
			void Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc(const SPtr<Texture>& target);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFPrepareMat* GetVariation(bool msaa);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mCommonParamBuffer;
			GpuParameterSampledTexture mInputTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(BokehDOFParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2I, gTileCount)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInvInputSize)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInvOutputSize)
			B3D_PARAM_BLOCK_ENTRY(float, gAdaptiveThresholdColor)
			B3D_PARAM_BLOCK_ENTRY(float, gAdaptiveThresholdCOC)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gBokehSize)
			B3D_PARAM_BLOCK_ENTRY(int, gLayerPixelOffset)
			B3D_PARAM_BLOCK_ENTRY(float, gInvDepthRange)
		B3D_PARAM_BLOCK_END

		/**
		 * Shader that renders the Bokeh DOF sprites and generates the blurred depth of field images. Separate images
		 * are generated for the near-field and in-focus + far-field.
		 */
		class BokehDOFMat : public RendererMaterial<BokehDOFMat>
		{
			RMAT_DEF_CUSTOMIZED("PPBokehDOF.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool DEPTH_OCCLUSION>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("DEPTH_OCCLUSION", DEPTH_OCCLUSION) });

				return variation;
			}

		public:
			static constexpr u32 kNearFarPadding = 128;
			static constexpr u32 kQuadsPerTile = 8;

			BokehDOFMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	input		Input texture as generated by BokehDOFPrepare material..
			 * @param[in]	view		View through which the depth of field effect is viewed.
			 * @param[in]	settings	Settings used to control depth of field rendering.
			 * @param[in]	output		Texture to output the results to.
			 */
			void Execute(const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static POOLED_RENDER_TEXTURE_DESC GetOutputDesc(const SPtr<Texture>& target);

			/** Populates the common depth of field parameter buffers with values from the provided settings object. */
			static void PopulateDofCommonParams(const SPtr<GpuBuffer>& buffer, const DepthOfFieldSettings& settings, const RendererView& view);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFMat* GetVariation(bool depthOcclusion);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mCommonParamBuffer;
			GpuParameterSampledTexture mInputTextureVS;
			GpuParameterSampledTexture mInputTextureFS;
			GpuParameterSampledTexture mBokehTexture;
			GpuParameterSampledTexture mDepthTexture;

			SPtr<VertexDescription> mTileVertexDescription;
			SPtr<GpuBuffer> mTileIndexBuffer;
			SPtr<GpuBuffer> mTileVertexBuffer;
		};

		B3D_PARAM_BLOCK_BEGIN(BokehDOFCombineParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gLayerAndScaleOffset)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gFocusedImageSize)
		B3D_PARAM_BLOCK_END

		/** Shader that combines the unfocused texture's near and far layers, together with the focused version. */
		class BokehDOFCombineMat : public RendererMaterial<BokehDOFCombineMat>
		{
			RMAT_DEF("PPBokehDOFCombine.bsl");

			/** Helper method used for initializing variations of this material. */
			template <MSAAMode MSAA_MODE>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_MODE", (i32)MSAA_MODE) });

				return variation;
			}

		public:
			BokehDOFCombineMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFCombineMat* GetVariation(MSAAMode msaaMode);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mCommonParamBuffer;
			GpuParameterSampledTexture mUnfocusedTexture;
			GpuParameterSampledTexture mFocusedTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(MotionBlurParamDef)
			B3D_PARAM_BLOCK_ENTRY(u32, gHalfNumSamples)
		B3D_PARAM_BLOCK_END

		/** Shader that blurs the scene depending on camera and/or object movement. */
		class MotionBlurMat : public RendererMaterial<MotionBlurMat>
		{
			RMAT_DEF("PPMotionBlur.bsl");

		public:
			MotionBlurMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	input		Input texture to blur.
			 * @param[in]	depth		Input depth buffer texture that will be used for determining pixel depth.
			 * @param[in]	view		View through which the depth of field effect is viewed.
			 * @param[in]	settings	Settings used to control the motion blur effect.
			 * @param[in]	output		Texture to output the results to.
			 */
			void Execute(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings, const SPtr<RenderTarget>& output);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(BuildHiZFParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gHalfPixelOffset)
			B3D_PARAM_BLOCK_ENTRY(int, gMipLevel)
		B3D_PARAM_BLOCK_END

		extern BuildHiZFParamDef gBuildHiZFParamDef;

		/** Shader that calculates a single level of the hierarchical Z mipmap chain. */
		class BuildHiZMat : public RendererMaterial<BuildHiZMat>
		{
			RMAT_DEF("PPBuildHiZ.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool noTextureViews>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("NO_TEXTURE_VIEWS", noTextureViews),
					});

				return variation;
			}

		public:
			BuildHiZMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& source, u32 srcMip, const Rect2& srcRect, const Rect2& dstRect, const SPtr<RenderTexture>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	noTextureViews		Specify as true if the current render backend doesn't support texture views, in
			 *								which case the implementation falls back on using a simpler version of the shader.
			 */
			static BuildHiZMat* GetVariation(bool noTextureViews);

		private:
			GpuParameterSampledTexture mInputTexture;
			SPtr<GpuBuffer> mParamBuffer;
			bool mNoTextureViews = false;
		};

		B3D_PARAM_BLOCK_BEGIN(FXAAParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gInvTexSize)
		B3D_PARAM_BLOCK_END

		extern FXAAParamDef gFXAAParamDef;

		/** Shader that performs Fast Approximate anti-aliasing. */
		class FXAAMat : public RendererMaterial<FXAAMat>
		{
			RMAT_DEF("PPFXAA.bsl");

		public:
			FXAAMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	source		Input texture to apply FXAA to.
			 * @param[in]	destination	Output target to which to write the antialiased image to.
			 */
			void Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(SSAOParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gSampleRadius)
			B3D_PARAM_BLOCK_ENTRY(float, gWorldSpaceRadiusMask)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gTanHalfFOV)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gRandomTileScale)
			B3D_PARAM_BLOCK_ENTRY(float, gCotHalfFOV)
			B3D_PARAM_BLOCK_ENTRY(float, gBias)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gDownsampledPixelSize)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gFadeMultiplyAdd)
			B3D_PARAM_BLOCK_ENTRY(float, gPower)
			B3D_PARAM_BLOCK_ENTRY(float, gIntensity)
		B3D_PARAM_BLOCK_END

		extern SSAOParamDef gSSAOParamDef;

		/** Textures used as input when calculating SSAO. */
		struct SSAOTextureInputs
		{
			/** Full resolution scene depth. Only used by final SSAO pass. */
			SPtr<Texture> SceneDepth;

			/** Full resolution buffer containing scene normals. Only used by final SSAO pass. */
			SPtr<Texture> SceneNormals;

			/** Precalculated texture containing downsampled normals/depth, to be used for AO input. */
			SPtr<Texture> AoSetup;

			/** Texture containing AO from the previous pass. Only used if upsampling is enabled. */
			SPtr<Texture> AoDownsampled;

			/** Tileable texture containing random rotations that will be applied to AO samples. */
			SPtr<Texture> RandomRotations;
		};

		/** Shader that computes ambient occlusion using screen based methods. */
		class SSAOMat : public RendererMaterial<SSAOMat>
		{
			RMAT_DEF("PPSSAO.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool upsample, bool finalPass, int quality>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MIX_WITH_UPSAMPLED", upsample),
					  ShaderVariationParameter("FINAL_AO", finalPass),
					  ShaderVariationParameter("QUALITY", quality) });

				return variation;
			}

		public:
			SSAOMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	view			Information about the view we're rendering from.
			 * @param[in]	textures		Set of textures to be used as input. Which textures are used depends on the
			 *								template parameters of this class.
			 * @param[in]	destination		Output texture to which to write the ambient occlusion data to.
			 * @param[in]	settings		Settings used to control the ambient occlusion effect.
			 */
			void Execute(const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings);

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
			static SSAOMat* GetVariation(bool upsample, bool finalPass, int quality);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mDepthTexture;
			GpuParameterSampledTexture mNormalsTexture;
			GpuParameterSampledTexture mDownsampledAOTexture;
			GpuParameterSampledTexture mSetupAOTexture;
			GpuParameterSampledTexture mRandomTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(SSAODownsampleParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gPixelSize)
			B3D_PARAM_BLOCK_ENTRY(float, gInvDepthThreshold)
		B3D_PARAM_BLOCK_END

		extern SSAODownsampleParamDef gSSAODownsampleParamDef;

		/**
		 * Shader that downsamples the depth & normal buffer and stores their results in a common texture, to be consumed
		 * by SSAOMat.
		 */
		class SSAODownsampleMat : public RendererMaterial<SSAODownsampleMat>
		{
			RMAT_DEF("PPSSAODownsample.bsl");

		public:
			SSAODownsampleMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	view			Information about the view we're rendering from.
			 * @param[in]	sceneDepth		Input texture containing scene depth.
			 * @param[in]	sceneNormals	Input texture containing scene world space normals.
			 * @param[in]	destination		Output texture to which to write the downsampled data to.
			 * @param[in]	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
			 */
			void Execute(const RendererView& view, const SPtr<Texture>& sceneDepth, const SPtr<Texture>& sceneNormals, const SPtr<RenderTexture>& destination, float depthRange);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mDepthTexture;
			GpuParameterSampledTexture mNormalsTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(SSAOBlurParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gPixelSize)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gPixelOffset)
			B3D_PARAM_BLOCK_ENTRY(float, gInvDepthThreshold)
		B3D_PARAM_BLOCK_END

		extern SSAOBlurParamDef gSSAOBlurParamDef;

		/**
		 * Shaders that blurs the ambient occlusion output, in order to hide the noise caused by the randomization texture.
		 */
		class SSAOBlurMat : public RendererMaterial<SSAOBlurMat>
		{
			RMAT_DEF("PPSSAOBlur.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool horizontal>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("DIR_HORZ", horizontal) });

				return variation;
			}

		public:
			SSAOBlurMat() = default;
			void Initialize() override;

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param[in]	view			Information about the view we're rendering from.
			 * @param[in]	ao				Input texture containing ambient occlusion data to be blurred.
			 * @param[in]	sceneDepth		Input texture containing scene depth.
			 * @param[in]	destination		Output texture to which to write the blurred data to.
			 * @param[in]	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
			 */
			void Execute(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& sceneDepth, const SPtr<RenderTexture>& destination, float depthRange);

			/** Returns the material variation matching the provided parameters. */
			static SSAOBlurMat* GetVariation(bool horizontal);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mAOTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(SSRStencilParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gRoughnessScaleBias)
		B3D_PARAM_BLOCK_END

		extern SSRStencilParamDef gSSRStencilParamDef;

		/** Shader used for marking which parts of the screen require screen space reflections. */
		class SSRStencilMat : public RendererMaterial<SSRStencilMat>
		{
			RMAT_DEF("PPSSRStencil.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", msaa ? 2 : 1),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			SSRStencilMat() = default;
			void Initialize() override;

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param[in]	view			Information about the view we're rendering from.
			 * @param[in]	gbuffer			GBuffer textures.
			 * @param[in]	settings		Parameters used for controling the SSR effect.
			 */
			void Execute(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static SSRStencilMat* GetVariation(bool msaa, bool singleSampleMSAA);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GBufferParams mGBufferParams;
		};

		B3D_PARAM_BLOCK_BEGIN(SSRTraceParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gNDCToHiZUV)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gHiZUVToScreenUV)
			B3D_PARAM_BLOCK_ENTRY(Vector2I, gHiZSize)
			B3D_PARAM_BLOCK_ENTRY(int, gHiZNumMips)
			B3D_PARAM_BLOCK_ENTRY(float, gIntensity)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gRoughnessScaleBias)
			B3D_PARAM_BLOCK_ENTRY(int, gTemporalJitter)
		B3D_PARAM_BLOCK_END

		extern SSRTraceParamDef gSSRTraceParamDef;

		/** Shader used for tracing rays for screen space reflections. */
		class SSRTraceMat : public RendererMaterial<SSRTraceMat>
		{
			RMAT_DEF("PPSSRTrace.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 quality, bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", msaa ? 2 : 1),
					  ShaderVariationParameter("QUALITY", quality),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			SSRTraceMat() = default;
			void Initialize() override;

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
			void Execute(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings, const SPtr<RenderTarget>& destination);

			/**
			 * Calculates a scale & bias that is used for transforming roughness into a fade out value. Anything that is below
			 * @p maxRoughness will have the fade value of 1. Values above @p maxRoughness is slowly fade out over a range that
			 * is 1/2 the length of @p maxRoughness.
			 */
			static Vector2 CalcRoughnessFadeScaleBias(float maxRoughness);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	quality				Determines how many rays to trace. In range [0, 4].
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static SSRTraceMat* GetVariation(u32 quality, bool msaa, bool singleSampleMSAA = false);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GBufferParams mGBufferParams;
			GpuParameterSampledTexture mSceneColorTexture;
			GpuParameterSampledTexture mHiZTexture;
		};

		B3D_PARAM_BLOCK_BEGIN(TemporalResolveParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(float, gSampleWeights, 9)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(float, gSampleWeightsLowpass, 9)
		B3D_PARAM_BLOCK_END

		extern TemporalResolveParamDef gTemporalResolveParamDef;

		B3D_PARAM_BLOCK_BEGIN(TemporalFilteringParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gSceneDepthTexelSize)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gSceneColorTexelSize)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gVelocityTexelSize)
			B3D_PARAM_BLOCK_ENTRY(float, gManualExposure)
		B3D_PARAM_BLOCK_END

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
			template <TemporalFilteringType TYPE, bool PER_PIXEL_VELOCITY, bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{
						ShaderVariationParameter("TYPE", (int)TYPE),
						ShaderVariationParameter("PER_PIXEL_VELOCITY", (int)PER_PIXEL_VELOCITY),
						ShaderVariationParameter("MSAA", MSAA),
					});

				return variation;
			}

		public:
			TemporalFilteringMat() = default;
			void Initialize() override;

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
			void Execute(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter, float exposure, const SPtr<RenderTarget>& destination);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	type		Type of filter to use.
			 * @param[in]	velocity	True if the filter will have access to a buffer containing per-pixel velocities.
			 * @param[in]	msaa		True if the shader will operate on a multisampled surface. Note that previous
			 *							and current frame color textures must be non-MSAA, regardless of this parameter.
			 * @return					Requested variation of the material.
			 */
			static TemporalFilteringMat* GetVariation(TemporalFilteringType type, bool velocity, bool msaa);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			SPtr<GpuBuffer> mTemporalParamBuffer;

			GpuParameterSampledTexture mSceneColorTexture;
			GpuParameterSampledTexture mPrevColorTexture;
			GpuParameterSampledTexture mSceneDepthTexture;
			GpuParameterSampledTexture mVelocityTexture;

			bool mHasVelocityTexture = false;
		};

		B3D_PARAM_BLOCK_BEGIN(EncodeDepthParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gNear)
			B3D_PARAM_BLOCK_ENTRY(float, gFar)
		B3D_PARAM_BLOCK_END

		extern EncodeDepthParamDef gEncodeDepthParamDef;

		/**
		 * Shader that encodes depth from a specified range into [0, 1] range, and writes the result in the alpha channel
		 * of the output texture.
		 */
		class EncodeDepthMat : public RendererMaterial<EncodeDepthMat>
		{
			RMAT_DEF("PPEncodeDepth.bsl");

		public:
			EncodeDepthMat() = default;
			void Initialize() override;

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
			void Execute(const SPtr<Texture>& depth, float near, float far, const SPtr<RenderTarget>& output);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		/**
		 * Shader that outputs a texture that determines which pixels require per-sample evaluation. Only relevant when
		 * rendering with MSAA enabled.
		 */
		class MSAACoverageMat : public RendererMaterial<MSAACoverageMat>
		{
			RMAT_DEF("MSAACoverage.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 msaa>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA_COUNT", msaa) });

				return variation;
			}

		public:
			MSAACoverageMat() = default;
			void Initialize() override;

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param[in]	view			Information about the view we're rendering from.
			 * @param[in]	gbuffer			GBuffer textures.
			 */
			void Execute(const RendererView& view, GBufferTextures gbuffer);

			/** Returns the material variation matching the provided parameters. */
			static MSAACoverageMat* GetVariation(u32 msaaCount);

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
			MSAACoverageStencilMat() = default;
			void Initialize() override;

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param[in]	view		Information about the view we're rendering from.
			 * @param[in]	coverage	Coverage texture as output by MSAACoverageMat.
			 */
			void Execute(const RendererView& view, const SPtr<Texture>& coverage);

		private:
			GpuParameterSampledTexture mCoverageTexParam;
		};

		/** @} */
	} // namespace ct
} // namespace bs
