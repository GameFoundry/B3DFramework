//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DGpuResourcePool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(ReflectionCubeDownsampleUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gMipLevel)
		B3D_UNIFORM_BUFFER_END

		extern ReflectionCubeDownsampleUniformDefinition gReflectionCubeDownsampleUniformDefinition;

		/** Performs filtering on cubemap faces in order to prepare them for importance sampling. */
		class ReflectionCubeDownsampleMaterial : public RendererMaterial<ReflectionCubeDownsampleMaterial>
		{
			RMAT_DEF("ReflectionCubeDownsample.bsl")

		public:
			ReflectionCubeDownsampleMaterial() = default;
			void Initialize() override;

			/** Downsamples the provided texture face and outputs it to the provided target. */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 mip, const TShared<RenderTarget>& target);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ReflectionCubeImportanceSampleUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gMipLevel)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumMips)
			B3D_UNIFORM_BUFFER_MEMBER(float, gPrecomputedMipFactor)
		B3D_UNIFORM_BUFFER_END

		extern ReflectionCubeImportanceSampleUniformDefinition gReflectionCubeImportanceSampleUniformDefinition;

		/** Performs importance sampling on cubemap faces in order for make them suitable for specular evaluation. */
		class ReflectionCubeImportanceSampleMaterial : public RendererMaterial<ReflectionCubeImportanceSampleMaterial>
		{
			RMAT_DEF_CUSTOMIZED("ReflectionCubeImportanceSample.bsl")

		public:
			ReflectionCubeImportanceSampleMaterial() = default;
			void Initialize() override;

			/** Importance samples the provided texture face and outputs it to the provided target. */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 mip, const TShared<RenderTarget>& target);

		private:
			static const u32 kNumSamples;

			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		/** Vector representing spherical harmonic coefficients for 5 bands. */
		struct SHVector5
		{
			SHVector5()
				: Coeffs()
			{}

			float Coeffs[25];
		};

		/** Vector representing spherical coefficients for 5 bands, separate for red, green and blue components. */
		struct SHVector5RGB
		{
			SHVector5 R, G, B;
		};

		/** Vector representing spherical harmonic coefficients for 3 bands. */
		struct SHVector3
		{
			float Coeffs[9];
		};

		/** Vector representing spherical coefficients for 3 bands, separate for red, green and blue components. */
		struct SHVector3RGB
		{
			SHVector3 R, G, B;
		};

		/** Intermediate structure used for spherical coefficient calculation. Contains RGB coefficients and weight. */
		struct SHCoeffsAndWeight5
		{
			SHVector5RGB Coeffs;
			float Weight;
		};

		/** Intermediate structure used for spherical coefficient calculation. Contains RGB coefficients and weight. */
		struct SHCoeffsAndWeight3
		{
			SHVector3RGB Coeffs;
			float Weight;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceComputeSHUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gFaceSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gDispatchSize)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceComputeSHUniformDefinition gIrradianceComputeSHUniformDefinition;

		/** Computes spherical harmonic coefficients from a radiance cubemap. */
		class IrradianceComputeSHMaterial : public RendererMaterial<IrradianceComputeSHMaterial>
		{
			RMAT_DEF_CUSTOMIZED("IrradianceComputeSH.bsl")

			/** Helper method used for initializing variations of this material. */
			template <int shOrder>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SH_ORDER", shOrder) });

				return variation;
			}

		public:
			IrradianceComputeSHMaterial() = default;
			void Initialize() override;

			/**
			 * Computes spherical harmonic coefficients from a radiance texture and outputs a buffer containing a list of
			 * coefficient sets (one set of coefficients for each thread group). Coefficients must be reduced and normalized
			 * by IrradianceReduceSHMaterial before use. Output buffer should be created by calling createOutputBuffer().
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, const TShared<GpuBuffer>& output);

			/** Creates a buffer of adequate size to be used as output for this material. */
			TShared<GpuBuffer> CreateOutputBuffer(const TShared<Texture>& source, u32& numCoeffSets);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are
			 *					supported.
			 */
			static IrradianceComputeSHMaterial* GetVariation(int order = 5);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
			GpuParameterStorageBuffer mOutputBufferParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceReduceSHUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gOutputIdx)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumEntries)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceReduceSHUniformDefinition gIrradianceReduceSHUniformDefinition;

		/**
		 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMaterial and outputs a single
		 * set of normalized coefficients.
		 */
		class IrradianceReduceSHMaterial : public RendererMaterial<IrradianceReduceSHMaterial>
		{
			RMAT_DEF("IrradianceReduceSH.bsl")

			/** Helper method used for initializing variations of this material. */
			template <int shOrder>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SH_ORDER", shOrder) });

				return variation;
			}

		public:
			IrradianceReduceSHMaterial() = default;
			void Initialize() override;

			/**
			 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMaterial and outputs a
			 * single set of normalized coefficients. Output texture should be created by calling createOutputTexture(). The
			 * value will be recorded at the @p outputIdx position in the texture.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<GpuBuffer>& source, u32 numCoeffSets, const TShared<Texture>& output, u32 outputIdx);

			/** Creates a texture of adequate size to be used as output for this material. */
			TShared<Texture> CreateOutputTexture(u32 numCoeffSets);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are
			 *					supported.
			 */
			static IrradianceReduceSHMaterial* GetVariation(int order = 5);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterStorageBuffer mInputBufferParameter;
			GpuParameterStorageTexture mOutputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceComputeSHFragUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gFaceSize)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCoeffEntryIdx)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCoeffComponentIdx)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceComputeSHFragUniformDefinition gIrradianceComputeSHFragUniformDefinition;

		/**
		 * Computes spherical harmonic coefficients from a radiance cubemap. This is an alternative to IrradianceComputeSHMaterial
		 * that does not require compute shader support.
		 */
		class IrradianceComputeSHFragMaterial : public RendererMaterial<IrradianceComputeSHFragMaterial>
		{
			RMAT_DEF("IrradianceComputeSHFrag.bsl")

		public:
			IrradianceComputeSHFragMaterial() = default;
			void Initialize() override;

			/**
			 * Computes spherical harmonic coefficients from a face of an input cube radiance texture and outputs them to the
			 * specified face of the output cube texture. Only a single coefficient is output per execution. The output texture
			 * will contain the coefficients for red, green and blue channels in the corresponding texture channels, and
			 * per-texel weight in the alpha channel. Output coefficients must be summed up and normalized before use (using
			 * IrradianceAccumulateCubeSH).
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 coefficientIdx, const TShared<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. Note that the
			 * output texture is a cubemap but the execute() method expects a render target that is a single face of a
			 * cubemap.
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc(const TShared<Texture>& source);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceAccumulateSHUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeMip)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHalfPixel)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceAccumulateSHUniformDefinition gIrradianceAccumulateSHUniformDefinition;

		/**
		 * Downsamples a cubemap face containing SH coefficient and weight values as output by IrradianceComputeSHFragMaterial. Each
		 * downsample sums up 2x2 pixel area coefficients/weights from the previous mip level.
		 */
		class IrradianceAccumulateSHMaterial : public RendererMaterial<IrradianceAccumulateSHMaterial>
		{
			RMAT_DEF("IrradianceAccumulateSH.bsl")

		public:
			IrradianceAccumulateSHMaterial() = default;
			void Initialize() override;

			/**
			 * Downsamples the provided face and mip level of the source texture and outputs the downsampled (i.e summed up)
			 * values in the resulting output texture.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 face, u32 sourceMip, const TShared<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. Note the output
			 * is a cubemap.
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc(const TShared<Texture>& source);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		/**
		 * Accumulates SH coefficient values from all six faces of a cubemap and normalizes them. The cubemap is expected to be
		 * 1x1 in size (previously downsampled by IrradianceAccumulateSHMaterial). After this shader is ran for all SH coefficients
		 * the output texture will contain final valid set of SH coefficients.
		 */
		class IrradianceAccumulateCubeSHMaterial : public RendererMaterial<IrradianceAccumulateCubeSHMaterial>
		{
			RMAT_DEF("IrradianceAccumulateCubeSH.bsl")

		public:
			IrradianceAccumulateCubeSHMaterial() = default;
			void Initialize() override;

			/**
			 * Sums up all faces of the input cube texture and writes the value to the corresponding index in the output
			 * texture. The source mip should point to a mip level with size 1x1.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& source, u32 sourceMip, const Vector2I& outputOffset, u32 coefficientIdx, const TShared<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. The render target
			 * will be able to hold all required SH coefficients (even though execute() outputs just one coefficient at a time).
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc();

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceProjectSHUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceProjectSHUniformDefinition gIrradianceProjectSHUniformDefinition;

		/**
		 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMaterial and projects them onto faces of
		 * a cubemap.
		 */
		class IrradianceProjectSHMaterial : public RendererMaterial<IrradianceProjectSHMaterial>
		{
			RMAT_DEF("IrradianceProjectSH.bsl")

		public:
			IrradianceProjectSHMaterial() = default;
			void Initialize() override;

			/**
			 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMaterial and projects them onto faces of
			 * a cubemap.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& shCoeffs, u32 face, const TShared<RenderTarget>& target);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		/** Render beast implementation of IBLUtility. */
		class RenderBeastIBLUtility : public IBLUtility
		{
		public:
			void FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& scratch) const override;
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output) const override;
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output, u32 outputIdx) const override;
			void ScaleCubemap(GpuCommandBuffer& commandBuffer, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip) const override;

		private:
			/**
			 * Downsamples a cubemap using hardware bilinear filtering.
			 *
			 * @param[in]	src		Cubemap to downsample.
			 * @param[in]   srcMip	Determines which mip level of the source texture to downsample.
			 * @param[in]   dst		Desination texture to output the scaled data to. Must be usable as a render target.
			 * @param[in]   dstMip	Determines which mip level of the destination texture to scale.
			 */
			static void DownsampleCubemap(GpuCommandBuffer& commandBuffer, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip);

			/**
			 * Generates irradiance SH coefficients from the input cubemap and writes them to a 1D texture. Does not make
			 * use of the compute shader.
			 */
			static void FilterCubemapForIrradianceNonCompute(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, u32 outputIdx, const TShared<RenderTexture>& output);
		};

		/** @} */
	} // namespace render
} // namespace b3d
