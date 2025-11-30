//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRenderSettings.h"
#include "Renderer/B3DGpuResourcePool.h"
#include "B3DRendererLight.h"

namespace b3d
{
	namespace render
	{
		struct RendererViewTargetInformation;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(DownsampleUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Vector2, gOffsets, 4)
		B3D_UNIFORM_BUFFER_END

		extern DownsampleUniformDefinition gDownsampleUniformDefinition;

		/** Shader that downsamples a texture to half its size. */
		class DownsampleMaterial : public RendererMaterial<DownsampleMaterial>
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
			DownsampleMaterial() = default;
			void Initialize() override;

			/** Renders the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& target);

			/** Returns the downsample material variation matching the provided parameters. */
			static DownsampleMaterial* GetVariation(u32 quality, bool msaa);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(EyeAdaptHistogramUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gPixelOffsetAndSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHistogramParams)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gThreadGroupCount)
		B3D_UNIFORM_BUFFER_END

		extern EyeAdaptHistogramUniformDefinition gEyeAdaptHistogramUniformDefinition;

		/** Shader that creates a luminance histogram used for eye adaptation. */
		class EyeAdaptHistogramMaterial : public RendererMaterial<EyeAdaptHistogramMaterial>
		{
			RMAT_DEF_CUSTOMIZED("PPEyeAdaptHistogram.bsl")

		public:
			EyeAdaptHistogramMaterial() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<Texture>& output, const AutoExposureSettings& settings);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& target);

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
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mSceneColorParameter;
			GpuParameterStorageTexture mOutputTextureParameter;

			static const u32 kLoopCountX = 8;
			static const u32 kLoopCountY = 8;
		};

		B3D_UNIFORM_BUFFER_BEGIN(EyeAdaptHistogramReduceUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gThreadGroupCount)
		B3D_UNIFORM_BUFFER_END

		extern EyeAdaptHistogramReduceUniformDefinition gEyeAdaptHistogramReduceUniformDefinition;

		/** Shader that reduces the luminance histograms created by EyeAdaptHistogramMat into a single histogram. */
		class EyeAdaptHistogramReduceMaterial : public RendererMaterial<EyeAdaptHistogramReduceMaterial>
		{
			RMAT_DEF("PPEyeAdaptHistogramReduce.bsl");

		public:
			EyeAdaptHistogramReduceMaterial() = default;
			void Initialize() override;

			/** Prepares GPU parameters for rendering. Must be called before Execute(). */
			void Prepare(const SPtr<Texture>& sceneColor, const SPtr<Texture>& histogram, const SPtr<Texture>& prevFrame);

			/** Executes the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc();

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mHistogramTextureParameter;
			GpuParameterSampledTexture mEyeAdaptationTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(EyeAdaptationUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Vector4, gEyeAdaptationParams, 3)
		B3D_UNIFORM_BUFFER_END

		extern EyeAdaptationUniformDefinition gEyeAdaptationUniformDefinition;

		/** Shader that computes the eye adaptation value based on scene luminance. */
		class EyeAdaptationMaterial : public RendererMaterial<EyeAdaptationMaterial>
		{
			RMAT_DEF_CUSTOMIZED("PPEyeAdaptation.bsl");

		public:
			EyeAdaptationMaterial() = default;
			void Initialize() override;

			/** Prepares GPU parameters before rendering. */
			void Prepare(const SPtr<Texture>& reducedHistogram, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Executes the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc();

			/**
			 * Populates the provided uniform buffer with eye adaptation parameters. The uniform buffer is expected to be
			 * created with EyeAdaptationUniformDefinition block definition.
			 */
			static void PopulateUniformBuffer(const GpuBufferSuballocation& uniformBuffer, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mReducedHistogramTex;
		};

		/**
		 * Shader that computes luminance of all the pixels in the provided texture, and stores them in log2 format, scaled
		 * to [0, 1] range (according to eye adapatation parameters) and stores those values in the alpha channel of the
		 * output texture. Color channel is just a copy of the input texture. Resulting texture is intended to be provided
		 * to the downsampling shader in order to calculate the average luminance, used for non-histogram eye adaptation
		 * calculation (when compute shader is not available).
		 */
		class EyeAdaptationBasicSetupMaterial : public RendererMaterial<EyeAdaptationBasicSetupMaterial>
		{
			RMAT_DEF("PPEyeAdaptationBasicSetup.bsl");

		public:
			EyeAdaptationBasicSetupMaterial() = default;
			void Initialize() override;

			/** Prepares GPU parameters before rendering. */
			void Prepare(const SPtr<Texture>& input, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Executes the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& input);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(EyeAdaptationBasicUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gInputTexSize)
		B3D_UNIFORM_BUFFER_END

		extern EyeAdaptationBasicUniformDefinition gEyeAdaptationBasicUniformDefinition;

		/**
		 * Shader that computes eye adapatation value from a texture that has luminance encoded in its alpha channel (as done
		 * by EyeAdaptationBasicSetupMat). The result is a 1x1 texture containing the eye adaptation value.
		 */
		class EyeAdaptationBasicMaterial : public RendererMaterial<EyeAdaptationBasicMaterial>
		{
			RMAT_DEF("PPEyeAdaptationBasic.bsl");

		public:
			EyeAdaptationBasicMaterial() = default;
			void Initialize() override;

			/** Prepares GPU parameters before rendering. */
			void Prepare(const SPtr<Texture>& curFrame, const SPtr<Texture>& prevFrame, float frameDelta, const AutoExposureSettings& settings, float exposureScale);

			/** Executes the post-process effect with the provided parameters. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc();

		private:
			GpuParameterUniformBuffer mEyeAdaptationUniformBufferParameter;
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mCurrentFrameTextureParameter;
			GpuParameterSampledTexture mPreviousFrameTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(CreateTonemapLUTUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Vector4, gTonemapParams, 2)
			B3D_UNIFORM_BUFFER_MEMBER(float, gGammaAdjustment)
			B3D_UNIFORM_BUFFER_MEMBER(int, gGammaCorrectionType)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gSaturation)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gContrast)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gGain)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gOffset)
		B3D_UNIFORM_BUFFER_END

		extern CreateTonemapLUTUniformDefinition gCreateTonemapLUTUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(WhiteBalanceUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gWhiteTemp)
			B3D_UNIFORM_BUFFER_MEMBER(float, gWhiteOffset)
		B3D_UNIFORM_BUFFER_END

		extern WhiteBalanceUniformDefinition gWhiteBalanceUniformDefinition;

		/** Shader that creates a 2D lookup texture that is used to apply tonemapping, color grading, white balancing and gamma correction. Uses a vertex/fragment pipeline. */
		class CreateTonemap2DLUTMaterial : public RendererMaterial<CreateTonemap2DLUTMaterial>
		{
			RMAT_DEF_CUSTOMIZED("PPCreateTonemap2DLUT.bsl")

		public:
			CreateTonemap2DLUTMaterial() = default;
			void Initialize() override;

			/** Prepares GPU parameters for rendering. Must be called before Execute(). */
			void Prepare(const RenderSettings& settings);

			/** Executes the post-process effect with the provided parameters, generating an unwrapped 2D LUT using the vertex & fragment shader pipeline. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			PooledRenderTextureCreateInformation GetOutputDesc() const;

			/** Populates a uniform buffer of CreateTonemapLUTUniformDefinition type using the provided settings. */
			static void PopulateTonemappingUniformBuffer(const RenderSettings& settings, const GpuBufferSuballocation& uniformBuffer);

			/** Populates a uniform buffer of WhiteBalanceUniformDefinition type using the provided settings. */
			static void PopulateWhiteBalanceUniformBuffer(const RenderSettings& settings, const GpuBufferSuballocation& uniformBuffer);

			/** Size of a single dimension in the color lookup table. */
			static const u32 kLutSize = 32;

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mWhiteBalanceUniformBufferParameter;
		};

		/** Shader that creates a 3D lookup texture that is used to apply tonemapping, color grading, white balancing and gamma correction. Uses a compute pipeline. */
		class CreateTonemap3DLUTMaterial : public RendererMaterial<CreateTonemap3DLUTMaterial>
		{
			RMAT_DEF_CUSTOMIZED("PPCreateTonemap3DLUT.bsl");

		public:
			CreateTonemap3DLUTMaterial() = default;
			void Initialize() override;

			/** Executes the post-process effect with the provided parameters, generating a 3D LUT using a compute shader. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& output, const RenderSettings& settings);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			PooledRenderTextureCreateInformation GetOutputDesc() const;

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mWhiteBalanceUniformBufferParameter;
			GpuParameterStorageTexture mOutputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(TonemappingUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gRawGamma)
			B3D_UNIFORM_BUFFER_MEMBER(float, gManualExposureScale)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gTexSize)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gBloomTint)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumSamples)
		B3D_UNIFORM_BUFFER_END

		extern TonemappingUniformDefinition gTonemappingUniformDefinition;

		/** Shader that applies tonemapping and converts a HDR image into a LDR image. */
		class TonemappingMaterial : public RendererMaterial<TonemappingMaterial>
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
			TonemappingMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	sceneColor		Input HDR scene color texture.
			 * @param	eyeAdaptation	Eye adaptation texture containing exposure value.
			 * @param	bloom			Optional bloom texture.
			 * @param	colorLUT		Color lookup table texture.
			 * @param	settings		Render settings for the current view.
			 */
			void Prepare(const SPtr<Texture>& sceneColor, const SPtr<Texture>& eyeAdaptation, const SPtr<Texture>& bloom, const SPtr<Texture>& colorLUT, const RenderSettings& settings);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the currently bound
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the material variation matching the provided parameters. */
			static TonemappingMaterial* GetVariation(bool volumeLUT, bool gammaOnly, bool autoExposure, bool MSAA);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mBloomTextureParameter;
			GpuParameterSampledTexture mColorLUTParameter;
			GpuParameterSampledTexture mEyeAdaptationTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(BloomClipUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gThreshold)
			B3D_UNIFORM_BUFFER_MEMBER(float, gManualExposureScale)
		B3D_UNIFORM_BUFFER_END

		extern BloomClipUniformDefinition gBloomClipUniformDefinition;

		/** Shader that clips parts of the image that shouldn't be affected by bloom (parts that aren't bright enough). */
		class BloomClipMaterial : public RendererMaterial<BloomClipMaterial>
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
			BloomClipMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	input			Texture to process.
			 * @param	threshold		Threshold below which values will be ignored for purposes of bloom.
			 * @param	eyeAdaptation	Texture containing eye adaptation exposure value. Only needed if using the
			 *								AUTO_EXPOSURE variation of this material.
			 * @param	settings		Render settings for the current view.
			 */
			void Prepare(const SPtr<Texture>& input, float threshold, const SPtr<Texture>& eyeAdaptation, const RenderSettings& settings);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the currently bound
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	autoExposure	If true the exposure value will need to be provided in a texture output from the
			 *								eye adaptation material. Otherwise manual exposure scale from render settings will
			 *								be used.
			 */
			static BloomClipMaterial* GetVariation(bool autoExposure);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mEyeAdaptationTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ScreenSpaceLensFlareUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gThreshold)
			B3D_UNIFORM_BUFFER_MEMBER(float, gGhostSpacing)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gGhostCount)
			B3D_UNIFORM_BUFFER_MEMBER(float, gHaloRadius)
			B3D_UNIFORM_BUFFER_MEMBER(float, gHaloThickness)
			B3D_UNIFORM_BUFFER_MEMBER(float, gHaloThreshold)
			B3D_UNIFORM_BUFFER_MEMBER(float, gHaloAspectRatio)
			B3D_UNIFORM_BUFFER_MEMBER(float, gChromaticAberration)
		B3D_UNIFORM_BUFFER_END

		extern ScreenSpaceLensFlareUniformDefinition gScreenSpaceLensFlareUniformDefinition;

		/** Generates ghost features from bright areas of an image, required for screen space lens flare rendering. */
		class ScreenSpaceLensFlareMaterial : public RendererMaterial<ScreenSpaceLensFlareMaterial>
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
			ScreenSpaceLensFlareMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	input			Texture to process.
			 * @param	settings		Settings used for customizing the effect.
			 */
			void Prepare(const SPtr<Texture>& input, const ScreenSpaceLensFlareSettings& settings);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	halo					If true the effect will render a halo as well as a normal lens flare.
			 * @param	haloAspect				If true, user can set a custom aspect ratio for the rendered halo.
			 *										Only relevant if @p halo is turned on.
			 * @param	chromaticAberration		If true, lens flare and halo (if enabled) features will be rendered by
			 *										splitting the red, green and blue channels according to a user-provided
			 *										offset.
			 */
			static ScreenSpaceLensFlareMaterial* GetVariation(bool halo, bool haloAspect, bool chromaticAberration);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mGradientTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ChromaticAberrationUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInputSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gShiftAmount)
		B3D_UNIFORM_BUFFER_END

		extern ChromaticAberrationUniformDefinition gChromaticAberrationUniformDefinition;

		/** Renders a chromatic aberration effect by shifting RGB color channels. */
		class ChromaticAberrationMaterial : public RendererMaterial<ChromaticAberrationMaterial>
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
			ChromaticAberrationMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	input			Texture to process.
			 * @param	settings		Settings used for customizing the effect.
			 */
			void Prepare(const SPtr<Texture>& input, const ChromaticAberrationSettings& settings);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	type		Type that determines how is the effect performed.
			 */
			static ChromaticAberrationMaterial* GetVariation(ChromaticAberrationType type);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mFringeTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(FilmGrainUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gIntensity)
			B3D_UNIFORM_BUFFER_MEMBER(float, gTime)
		B3D_UNIFORM_BUFFER_END

		extern FilmGrainUniformDefinition gFilmGrainUniformDefinition;

		/** Renders a film grain effect using a noise function. */
		class FilmGrainMaterial : public RendererMaterial<FilmGrainMaterial>
		{
			RMAT_DEF("PPFilmGrain.bsl");

		public:
			FilmGrainMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	input			Texture to process.
			 * @param	time			Time of the current frame, in seconds.
			 * @param	settings		Settings used for customizing the effect.
			 */
			void Prepare(const SPtr<Texture>& input, float time, const FilmGrainSettings& settings);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		const int kMaxBlurSamples = 128;

		B3D_UNIFORM_BUFFER_BEGIN(GaussianBlurUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Vector4, gSampleOffsets, (kMaxBlurSamples + 1) / 2)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(Vector4, gSampleWeights, kMaxBlurSamples)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumSamples)
		B3D_UNIFORM_BUFFER_END

		extern GaussianBlurUniformDefinition gGaussianBlurUniformDefinition;

		/** Shader that performs Gaussian blur filtering on the provided texture. */
		class GaussianBlurMaterial : public RendererMaterial<GaussianBlurMaterial>
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

			GaussianBlurMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	direction		Direction in which to perform the separable blur.
			 * @param	source			Input texture to blur.
			 * @param	filterSize		Size of the blurring filter, in percent of the source texture. In range [0, 1].
			 * @param	tint			Optional tint to apply all filtered pixels.
			 * @param	additive		Optional texture whose values to add to the destination texture (won't be included
			 *							in filtering). Only used if using the variation of this shader that supports additive
			 *							input.
			 */
			void PrepareDirection(Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint = Color::kWhite, const SPtr<Texture>& additive = nullptr);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Render target to write the results to.
			 */
			void ExecutePass(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/**
			 * Renders the post-process effect with the provided parameters. This is a convenience method that performs
			 * both horizontal and vertical blur passes.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	source			Input texture to blur.
			 * @param	filterSize		Size of the blurring filter, in percent of the source texture. In range [0, 1].
			 * @param	destination		Output texture to which to write the blurred image to.
			 * @param	tint			Optional tint to apply all filtered pixels.
			 * @param	additive		Optional texture whose values to add to the destination texture (won't be included
			 *							in filtering). Only used if using the variation of this shader that supports additive
			 *							input.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, float filterSize, const SPtr<RenderTexture>& destination, const Color& tint = Color::kWhite, const SPtr<Texture>& additive = nullptr);

			/**
			 * Populates the provided uniform buffer with parameters required for a shader including gaussian blur.
			 *
			 * @param[in]	uniformBuffer	Buffer to write the parameters to. Must be created using @p GaussianBlurUniformDefinition.
			 * @param[in]	direction		Direction in which to perform the separable blur.
			 * @param[in]	source			Source texture that needs to be blurred.
			 * @param[in]	filterSize		Size of the blurring filter, in percent of the source texture. In range [0, 1].
			 * @param[in]	tint			Optional tint to apply all filtered pixels.
			 */
			static void PopulateUniformBuffer(const GpuBufferSuballocation& uniformBuffer, Direction direction, const SPtr<Texture>& source, float filterSize, const Color& tint = Color::kWhite);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	additive	If true the returned variation will support and additional input texture that will be
			 *							added on top of the filtered output.
			 */
			static GaussianBlurMaterial* GetVariation(bool additive);

		private:
			/** Calculates weights and offsets for the standard distribution of the specified filter size. */
			static u32 CalcStdDistribution(float filterRadius, std::array<float, kMaxBlurSamples>& weights, std::array<float, kMaxBlurSamples>& offsets);

			/** Calculates the radius of the blur kernel depending on the source texture size and provided scale. */
			static float CalcKernelRadius(const SPtr<Texture>& source, float scale, Direction filterDir);

			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mAdditiveTextureParameter;
			bool mIsAdditive = false;
		};

		B3D_UNIFORM_BUFFER_BEGIN(GaussianDOFUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gNearBlurPlane)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFarBlurPlane)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInvNearBlurRange)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInvFarBlurRange)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHalfPixelOffset)
		B3D_UNIFORM_BUFFER_END

		extern GaussianDOFUniformDefinition gGaussianDOFUniformDefinition;

		/**
		 * Shader that masks pixels from the input color texture into one or two output textures. The masking is done by
		 * determining if the pixel falls into near or far unfocused plane, as determined by depth-of-field parameters. User
		 * can pick whether to output pixels just on the near plane, just on the far plane, or both.
		 *
		 */
		class GaussianDOFSeparateMaterial : public RendererMaterial<GaussianDOFSeparateMaterial>
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
			GaussianDOFSeparateMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	color			Input color texture to process.
			 * @param	depth			Input depth buffer texture that will be used for determining pixel depth.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control depth of field rendering.
			 */
			void Prepare(const SPtr<Texture>& color, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	color			Input color texture to process.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& color);

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
			static GaussianDOFSeparateMaterial* GetVariation(bool near, bool far);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mColorTextureParameter;
			GpuParameterSampledTexture mDepthTextureParameter;

			SPtr<PooledRenderTexture> mOutput0;
			SPtr<PooledRenderTexture> mOutput1;
		};

		/**
		 * Shader that combines pixels for near unfocused, focused and far unfocused planes, as calculated by
		 * GaussianDOFSeparateMat. Outputs final depth-of-field filtered image.
		 */
		class GaussianDOFCombineMaterial : public RendererMaterial<GaussianDOFCombineMaterial>
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
			GaussianDOFCombineMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	focused			Input texture containing focused (default) scene color.
			 * @param	near			Input texture containing filtered (blurred) values for the unfocused foreground area.
			 *							Can be null if no near plane needs to be blended.
			 * @param	far				Input texture containing filtered (blurred) values for the unfocused background area.
			 *							Can be null if no far plane needs to be blended.
			 * @param	depth			Input depth buffer texture that will be used for determining pixel depth.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control depth of field rendering.
			 */
			void Prepare(const SPtr<Texture>& focused, const SPtr<Texture>& near, const SPtr<Texture>& far, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Texture to output the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	near	If true, near plane pixels are read from the near plane texture, otherwise near plane is assumed
			 *					not to exist.
			 * @param	far		If true, far plane pixels are read from the far plane texture, otherwise far plane is assumed not
			 *					to exist.
			 */
			static GaussianDOFCombineMaterial* GetVariation(bool near, bool far);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mFocusedTextureParameter;
			GpuParameterSampledTexture mNearTextureParameter;
			GpuParameterSampledTexture mFarTextureParameter;
			GpuParameterSampledTexture mDepthTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(DepthOfFieldCommonUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFocalPlaneDistance)
			B3D_UNIFORM_BUFFER_MEMBER(float, gApertureSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFocalLength)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInFocusRange)
			B3D_UNIFORM_BUFFER_MEMBER(float, gSensorSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gImageSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gMaxBokehSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gNearTransitionRegion)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFarTransitionRegion)
		B3D_UNIFORM_BUFFER_END

		extern DepthOfFieldCommonUniformDefinition gDepthOfFieldCommonUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(BokehDOFPrepareUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvInputSize)
		B3D_UNIFORM_BUFFER_END

		extern BokehDOFPrepareUniformDefinition gBokehDOFPrepareUniformDefinition;

		/**
		 * Shader does a 2x texture downsample while accounting for different depth of field layers and encoding depth into
		 * the output.
		 */
		class BokehDOFPrepareMaterial : public RendererMaterial<BokehDOFPrepareMaterial>
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
			BokehDOFPrepareMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	input			Input texture to downsample.
			 * @param	depth			Input depth buffer texture that will be used for determining pixel depth.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control depth of field rendering.
			 */
			void Prepare(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Texture to output the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& target);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFPrepareMaterial* GetVariation(bool msaa);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mCommonUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterSampledTexture mDepthTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(BokehDOFUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gTileCount)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvInputSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvOutputSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gAdaptiveThresholdColor)
			B3D_UNIFORM_BUFFER_MEMBER(float, gAdaptiveThresholdCOC)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gBokehSize)
			B3D_UNIFORM_BUFFER_MEMBER(int, gLayerPixelOffset)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInvDepthRange)
		B3D_UNIFORM_BUFFER_END

		extern BokehDOFUniformDefinition gBokehDOFUniformDefinition;

		/**
		 * Shader that renders the Bokeh DOF sprites and generates the blurred depth of field images. Separate images
		 * are generated for the near-field and in-focus + far-field.
		 */
		class BokehDOFMaterial : public RendererMaterial<BokehDOFMaterial>
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

			BokehDOFMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	input			Input texture as generated by BokehDOFPrepare material.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control depth of field rendering.
			 * @param	output			Texture to output the results to (needed for size calculations).
			 */
			void Prepare(const SPtr<Texture>& input, const RendererView& view, const DepthOfFieldSettings& settings, const SPtr<RenderTarget>& output);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	input			Input texture as generated by BokehDOFPrepare material.
			 * @param	output			Texture to output the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& input, const SPtr<RenderTarget>& output);

			/** Returns the texture descriptor that can be used for initializing the output render target. */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& target);

			/** Populates the common depth of field uniform buffer with values from the provided settings object. */
			static void PopulateDofCommonParams(const GpuBufferSuballocation& uniformBuffer, const DepthOfFieldSettings& settings, const RendererView& view);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFMaterial* GetVariation(bool depthOcclusion);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mCommonUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureVSParameter;
			GpuParameterSampledTexture mInputTextureFSParameter;
			GpuParameterSampledTexture mBokehTextureParameter;
			GpuParameterSampledTexture mDepthTextureParameter;

			SPtr<VertexDescription> mTileVertexDescription;
			SPtr<GpuBuffer> mTileIndexBuffer;
			SPtr<GpuBuffer> mTileVertexBuffer;
		};

		B3D_UNIFORM_BUFFER_BEGIN(BokehDOFCombineUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gLayerAndScaleOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gFocusedImageSize)
		B3D_UNIFORM_BUFFER_END

		extern BokehDOFCombineUniformDefinition gBokehDOFCombineUniformDefinition;

		/** Shader that combines the unfocused texture's near and far layers, together with the focused version. */
		class BokehDOFCombineMaterial : public RendererMaterial<BokehDOFCombineMaterial>
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
			BokehDOFCombineMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	unfocused		Unfocused and half-resolution texture as generated by the BokehDOF material, containing
			 *							the near and far unfocused layers.
			 * @param	focused			Focused full resolution scene color.
			 * @param	depth			Input depth buffer texture that will be used for determining pixel depth.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control depth of field rendering.
			 */
			void Prepare(const SPtr<Texture>& unfocused, const SPtr<Texture>& focused, const SPtr<Texture>& depth, const RendererView& view, const DepthOfFieldSettings& settings);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Texture to output the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

			/** Returns the material variation matching the provided parameters. */
			static BokehDOFCombineMaterial* GetVariation(MSAAMode msaaMode);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mCommonUniformBufferParameter;
			GpuParameterSampledTexture mUnfocusedTextureParameter;
			GpuParameterSampledTexture mFocusedTextureParameter;
			GpuParameterSampledTexture mDepthTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(MotionBlurUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(u32, gHalfNumSamples)
		B3D_UNIFORM_BUFFER_END

		extern MotionBlurUniformDefinition gMotionBlurUniformDefinition;

		/** Shader that blurs the scene depending on camera and/or object movement. */
		class MotionBlurMaterial : public RendererMaterial<MotionBlurMaterial>
		{
			RMAT_DEF("PPMotionBlur.bsl");

		public:
			MotionBlurMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	input			Input texture to blur.
			 * @param	depth			Input depth buffer texture that will be used for determining pixel depth.
			 * @param	view			View through which the depth of field effect is viewed.
			 * @param	settings		Settings used to control the motion blur effect.
			 */
			void Prepare(const SPtr<Texture>& input, const SPtr<Texture>& depth, const RendererView& view, const MotionBlurSettings& settings);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Texture to output the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(BuildHiZUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHalfPixelOffset)
			B3D_UNIFORM_BUFFER_MEMBER(int, gMipLevel)
		B3D_UNIFORM_BUFFER_END

		extern BuildHiZUniformDefinition gBuildHiZUniformDefinition;

		/** Shader that calculates a single level of the hierarchical Z mipmap chain. */
		class BuildHiZMaterial : public RendererMaterial<BuildHiZMaterial>
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
			BuildHiZMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	source			Input depth texture to use as the source.
			 * @param	srcMip			Mip level to read from the @p source texture.
			 */
			void Prepare(const SPtr<Texture>& source, u32 srcMip);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Output target to which to write to results.
			 * @param	srcRect			Rectangle in normalized coordinates, describing from which portion of the source
			 *							texture to read the input.
			 * @param	dstRect			Destination rectangle to limit the writes to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& output, const Area2& srcRect, const Area2& dstRect);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	noTextureViews		Specify as true if the current render backend doesn't support texture views, in
			 *								which case the implementation falls back on using a simpler version of the shader.
			 */
			static BuildHiZMaterial* GetVariation(bool noTextureViews);

		private:
			GpuParameterSampledTexture mInputTexture;
			GpuParameterUniformBuffer mUniformBufferParameter;
			bool mNoTextureViews = false;
		};

		B3D_UNIFORM_BUFFER_BEGIN(FXAAUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gInvTexSize)
		B3D_UNIFORM_BUFFER_END

		extern FXAAUniformDefinition gFXAAUniformDefinition;

		/** Shader that performs Fast Approximate anti-aliasing. */
		class FXAAMaterial : public RendererMaterial<FXAAMaterial>
		{
			RMAT_DEF("PPFXAA.bsl");

		public:
			FXAAMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	source			Input texture to apply FXAA to.
			 */
			void Prepare(const SPtr<Texture>& source);

			/**
			 * Executes the post-process effect with the provided parameters and writes the results to the provided
			 * render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Output target to which to write the antialiased image to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(SSAOUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gSampleRadius)
			B3D_UNIFORM_BUFFER_MEMBER(float, gWorldSpaceRadiusMask)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gTanHalfFOV)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gRandomTileScale)
			B3D_UNIFORM_BUFFER_MEMBER(float, gCotHalfFOV)
			B3D_UNIFORM_BUFFER_MEMBER(float, gBias)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gDownsampledPixelSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gFadeMultiplyAdd)
			B3D_UNIFORM_BUFFER_MEMBER(float, gPower)
			B3D_UNIFORM_BUFFER_MEMBER(float, gIntensity)
		B3D_UNIFORM_BUFFER_END

		extern SSAOUniformDefinition gSSAOUniformDefinition;

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
		class SSAOMaterial : public RendererMaterial<SSAOMaterial>
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
			SSAOMaterial() = default;
			void Initialize() override;

			/**
			 * Updates GPU parameters. Must be called any time input parameters change.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	textures		Set of textures to be used as input. Which textures are used depends on the
			 *							template parameters of this class.
			 * @param	destination		Output texture to which to write the ambient occlusion data to.
			 * @param	settings		Settings used to control the ambient occlusion effect.
			 */
			void Prepare(const RendererView& view, const SSAOTextureInputs& textures, const SPtr<RenderTexture>& destination, const AmbientOcclusionSettings& settings);

			/**
			 * Renders the effect with the provided parameters, using the specified render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	destination		Output texture to which to write the ambient occlusion data to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination);

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
			static SSAOMaterial* GetVariation(bool upsample, bool finalPass, int quality);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mDepthTexture;
			GpuParameterSampledTexture mNormalsTexture;
			GpuParameterSampledTexture mDownsampledAOTexture;
			GpuParameterSampledTexture mSetupAOTexture;
			GpuParameterSampledTexture mRandomTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(SSAODownsampleUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gPixelSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInvDepthThreshold)
		B3D_UNIFORM_BUFFER_END

		extern SSAODownsampleUniformDefinition gSSAODownsampleUniformDefinition;

		/**
		 * Shader that downsamples the depth & normal buffer and stores their results in a common texture, to be consumed
		 * by SSAOMaterial.
		 */
		class SSAODownsampleMaterial : public RendererMaterial<SSAODownsampleMaterial>
		{
			RMAT_DEF("PPSSAODownsample.bsl");

		public:
			SSAODownsampleMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering. Must be called before Execute().
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	sceneDepth		Input texture containing scene depth.
			 * @param	sceneNormals	Input texture containing scene world space normals.
			 * @param	destination		Output texture to which to write the downsampled data to.
			 * @param	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
			 */
			void Prepare(const RendererView& view, const SPtr<Texture>& sceneDepth, const SPtr<Texture>& sceneNormals, const SPtr<RenderTexture>& destination, float depthRange);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	destination		Output texture to which to write the downsampled data to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mDepthTexture;
			GpuParameterSampledTexture mNormalsTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(SSAOBlurUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gPixelSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gPixelOffset)
			B3D_UNIFORM_BUFFER_MEMBER(float, gInvDepthThreshold)
		B3D_UNIFORM_BUFFER_END

		extern SSAOBlurUniformDefinition gSSAOBlurUniformDefinition;

		/**
		 * Shaders that blurs the ambient occlusion output, in order to hide the noise caused by the randomization texture.
		 */
		class SSAOBlurMaterial : public RendererMaterial<SSAOBlurMaterial>
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
			SSAOBlurMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	ao				Input texture containing ambient occlusion data to be blurred.
			 * @param	sceneDepth		Input texture containing scene depth.
			 * @param	depthRange		Valid depth range (in view space) within which nearby samples will be averaged.
			 */
			void Prepare(const RendererView& view, const SPtr<Texture>& ao, const SPtr<Texture>& sceneDepth, float depthRange);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	destination		Output texture to which to write the blurred data to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTexture>& destination);

			/** Returns the material variation matching the provided parameters. */
			static SSAOBlurMaterial* GetVariation(bool horizontal);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mAOTexture;
			GpuParameterSampledTexture mDepthTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(SSRStencilUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gRoughnessScaleBias)
		B3D_UNIFORM_BUFFER_END

		extern SSRStencilUniformDefinition gSSRStencilUniformDefinition;

		/** Shader used for marking which parts of the screen require screen space reflections. */
		class SSRStencilMaterial : public RendererMaterial<SSRStencilMaterial>
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
			SSRStencilMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	gbuffer			GBuffer textures.
			 * @param	settings		Parameters used for controlling the SSR effect.
			 */
			void Prepare(const RendererView& view, GBufferTextures gbuffer, const ScreenSpaceReflectionsSettings& settings);

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	view			Information about the view we're rendering from.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static SSRStencilMaterial* GetVariation(bool msaa, bool singleSampleMSAA);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GBufferParameterBinding mGBufferParams;
		};

		B3D_UNIFORM_BUFFER_BEGIN(SSRTraceUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gNDCToHiZUV)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHiZUVToScreenUV)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gHiZSize)
			B3D_UNIFORM_BUFFER_MEMBER(int, gHiZNumMips)
			B3D_UNIFORM_BUFFER_MEMBER(float, gIntensity)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gRoughnessScaleBias)
			B3D_UNIFORM_BUFFER_MEMBER(int, gTemporalJitter)
		B3D_UNIFORM_BUFFER_END

		extern SSRTraceUniformDefinition gSSRTraceUniformDefinition;

		/** Shader used for tracing rays for screen space reflections. */
		class SSRTraceMaterial : public RendererMaterial<SSRTraceMaterial>
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
			SSRTraceMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	gbuffer			GBuffer textures.
			 * @param	sceneColor		Scene color texture.
			 * @param	hiZ				Hierarchical Z buffer.
			 * @param	settings		Parameters used for controling the SSR effect.
			 */
			void Prepare(const RendererView& view, GBufferTextures gbuffer, const SPtr<Texture>& sceneColor, const SPtr<Texture>& hiZ, const ScreenSpaceReflectionsSettings& settings);

			/**
			 * Renders the effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	destination		Render target to which to write the results to.
			 * @param	view			Information about the view we're rendering from.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& destination, const RendererView& view);

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
			static SSRTraceMaterial* GetVariation(u32 quality, bool msaa, bool singleSampleMSAA = false);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GBufferParameterBinding mGBufferParams;
			GpuParameterSampledTexture mSceneColorTextureParameter;
			GpuParameterSampledTexture mHiZTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(TemporalResolveUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(float, gSampleWeights, 9)
			B3D_UNIFORM_BUFFER_MEMBER_ARRAY(float, gSampleWeightsLowpass, 9)
		B3D_UNIFORM_BUFFER_END

		extern TemporalResolveUniformDefinition gTemporalResolveUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(TemporalFilteringUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gSceneDepthTexelSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gSceneColorTexelSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gVelocityTexelSize)
			B3D_UNIFORM_BUFFER_MEMBER(float, gManualExposure)
		B3D_UNIFORM_BUFFER_END

		extern TemporalFilteringUniformDefinition gTemporalFilteringUniformDefinition;

		/** Supported filter types by TemporalFilteringMaterial. */
		enum class TemporalFilteringType
		{
			/** Temporal filter used for full screen anti-aliasing. */
			FullScreenAA,

			/** Temporal filter used for accumulating SSR samples over multiple frames. */
			SSR
		};

		/** Shader used for combining multiple frames of information using a temporal filter, in order to yield better quality. */
		class TemporalFilteringMaterial : public RendererMaterial<TemporalFilteringMaterial>
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
			TemporalFilteringMaterial() = default;
			void Initialize() override;

			/**
			 * Prepares GPU parameters for rendering.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	prevFrame		Frame data calculated previous frame.
			 * @param	curFrame		Frame data calculated this frame.
			 * @param	velocity		Optional texture containing per-pixel velocity;
			 * @param	sceneDepth		Buffer containing scene depth.
			 * @param	jitter			Sub-pixel jitter applied to the projection matrix.
			 * @param	exposure		Exposure to use when transforming from HDR to LDR image.
			 */
			void Prepare(const RendererView& view, const SPtr<Texture>& prevFrame, const SPtr<Texture>& curFrame, const SPtr<Texture>& velocity, const SPtr<Texture>& sceneDepth, const Vector2& jitter, float exposure);

			/**
			 * Renders the effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	view			Information about the view we're rendering from.
			 * @param	destination		Render target to which to write the results to.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const SPtr<RenderTarget>& destination);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	type		Type of filter to use.
			 * @param	velocity	True if the filter will have access to a buffer containing per-pixel velocities.
			 * @param	msaa		True if the shader will operate on a multisampled surface. Note that previous
			 *							and current frame color textures must be non-MSAA, regardless of this parameter.
			 * @return					Requested variation of the material.
			 */
			static TemporalFilteringMaterial* GetVariation(TemporalFilteringType type, bool velocity, bool msaa);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterUniformBuffer mTemporalUniformBufferParameter;

			GpuParameterSampledTexture mSceneColorTextureParameter;
			GpuParameterSampledTexture mPreviousColorTextureParameter;
			GpuParameterSampledTexture mSceneDepthTextureParameter;
			GpuParameterSampledTexture mVelocityTextureParameter;

			bool mHasVelocityTexture = false;
		};

		B3D_UNIFORM_BUFFER_BEGIN(EncodeDepthUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gNear)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFar)
		B3D_UNIFORM_BUFFER_END

		extern EncodeDepthUniformDefinition gEncodeDepthUniformDefinition;

		/**
		 * Shader that encodes depth from a specified range into [0, 1] range, and writes the result in the alpha channel
		 * of the output texture.
		 */
		class EncodeDepthMaterial : public RendererMaterial<EncodeDepthMaterial>
		{
			RMAT_DEF("PPEncodeDepth.bsl");

		public:
			EncodeDepthMaterial() = default;
			void Initialize() override;

			/**
			 * Updates GPU parameters. Must be called any time input parameters change.
			 *
			 * @param	depth			Resolved (non-MSAA) depth texture to encode.
			 * @param	near			Near range (in view space) to start encoding the depth. Any depth lower than this will
			 *							be encoded to 1.
			 * @param	far				Far range (in view space) to end encoding the depth. Any depth higher than this will
			 *							be encoded to 0.
			 */
			void Prepare(const SPtr<Texture>& depth, float near, float far);

			/**
			 * Renders the post-process effect with the provided parameters.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	output			Output texture to write the results in. Results will be written in the alpha channel.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<RenderTarget>& output);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		/**
		 * Shader that outputs a texture that determines which pixels require per-sample evaluation. Only relevant when
		 * rendering with MSAA enabled.
		 */
		class MSAACoverageMaterial : public RendererMaterial<MSAACoverageMaterial>
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
			MSAACoverageMaterial() = default;
			void Initialize() override;

			/**
			 * Updates GPU parameters. Must be called any time input parameters change.
			 *
			 * @param	view			Information about the view we're rendering from.
			 * @param	gbuffer			GBuffer textures.
			 */
			void Prepare(const RendererView& view, GBufferTextures gbuffer);

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	view			Information about the view we're rendering from.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view);

			/** Returns the material variation matching the provided parameters. */
			static MSAACoverageMaterial* GetVariation(u32 msaaCount);

		private:
			GBufferParameterBinding mGBufferParams;
		};

		/**
		 * Converts the coverage texture output by MSAACoverageMaterial and writes its information in the highest bit of the
		 * currently bound stencil buffer. This allows coverage information to be used by normal (non-compute) rendering
		 * shaders.
		 */
		class MSAACoverageStencilMaterial : public RendererMaterial<MSAACoverageStencilMaterial>
		{
			RMAT_DEF("MSAACoverageStencil.bsl");

		public:
			MSAACoverageStencilMaterial() = default;
			void Initialize() override;

			/**
			 * Updates GPU parameters. Must be called any time input parameters change.
			 *
			 * @param	coverage		Coverage texture as output by MSAACoverageMaterial.
			 */
			void Prepare(const SPtr<Texture>& coverage);

			/**
			 * Renders the effect with the provided parameters, using the currently bound render target.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	view			Information about the view we're rendering from.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const RendererView& view);

		private:
			GpuParameterSampledTexture mCoverageTextureParameter;
		};

		/** @} */
	} // namespace render
} // namespace b3d
