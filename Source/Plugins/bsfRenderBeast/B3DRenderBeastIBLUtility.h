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
		};

		/** @} */
	} // namespace render
} // namespace b3d
