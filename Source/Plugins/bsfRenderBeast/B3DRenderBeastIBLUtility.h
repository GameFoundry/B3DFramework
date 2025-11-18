//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "Renderer/B3DGpuResourcePool.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(ReflectionCubeDownsampleParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gMipLevel)
		B3D_UNIFORM_BUFFER_END

		extern ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

		/** Performs filtering on cubemap faces in order to prepare them for importance sampling. */
		class ReflectionCubeDownsampleMat : public RendererMaterial<ReflectionCubeDownsampleMat>
		{
			RMAT_DEF("ReflectionCubeDownsample.bsl")

		public:
			ReflectionCubeDownsampleMat() = default;
			void Initialize() override;

			/** Downsamples the provided texture face and outputs it to the provided target. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(ReflectionCubeImportanceSampleParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gMipLevel)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumMips)
			B3D_UNIFORM_BUFFER_MEMBER(float, gPrecomputedMipFactor)
		B3D_UNIFORM_BUFFER_END

		extern ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

		/** Performs importance sampling on cubemap faces in order for make them suitable for specular evaluation. */
		class ReflectionCubeImportanceSampleMat : public RendererMaterial<ReflectionCubeImportanceSampleMat>
		{
			RMAT_DEF_CUSTOMIZED("ReflectionCubeImportanceSample.bsl")

		public:
			ReflectionCubeImportanceSampleMat() = default;
			void Initialize() override;

			/** Importance samples the provided texture face and outputs it to the provided target. */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 mip, const SPtr<RenderTarget>& target);

		private:
			static const u32 kNumSamples;

			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
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

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceComputeSHParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gFaceSize)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gDispatchSize)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

		/** Computes spherical harmonic coefficients from a radiance cubemap. */
		class IrradianceComputeSHMat : public RendererMaterial<IrradianceComputeSHMat>
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
			IrradianceComputeSHMat() = default;
			void Initialize() override;

			/**
			 * Computes spherical harmonic coefficients from a radiance texture and outputs a buffer containing a list of
			 * coefficient sets (one set of coefficients for each thread group). Coefficients must be reduced and normalized
			 * by IrradianceReduceSHMat before use. Output buffer should be created by calling createOutputBuffer().
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, const SPtr<GpuBuffer>& output);

			/** Creates a buffer of adequate size to be used as output for this material. */
			SPtr<GpuBuffer> CreateOutputBuffer(const SPtr<Texture>& source, u32& numCoeffSets);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are
			 *					supported.
			 */
			static IrradianceComputeSHMat* GetVariation(int order = 5);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
			GpuParameterBuffer mOutputBuffer;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceReduceSHParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gOutputIdx)
			B3D_UNIFORM_BUFFER_MEMBER(int, gNumEntries)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

		/**
		 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMat and outputs a single
		 * set of normalized coefficients.
		 */
		class IrradianceReduceSHMat : public RendererMaterial<IrradianceReduceSHMat>
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
			IrradianceReduceSHMat() = default;
			void Initialize() override;

			/**
			 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMat and outputs a
			 * single set of normalized coefficients. Output texture should be created by calling createOutputTexture(). The
			 * value will be recorded at the @p outputIdx position in the texture.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& source, u32 numCoeffSets, const SPtr<Texture>& output, u32 outputIdx);

			/** Creates a texture of adequate size to be used as output for this material. */
			SPtr<Texture> CreateOutputTexture(u32 numCoeffSets);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are
			 *					supported.
			 */
			static IrradianceReduceSHMat* GetVariation(int order = 5);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterBuffer mInputBuffer;
			GpuParameterStorageTexture mOutputTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceComputeSHFragParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gFaceSize)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCoeffEntryIdx)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCoeffComponentIdx)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

		/**
		 * Computes spherical harmonic coefficients from a radiance cubemap. This is an alternative to IrradianceComputeSHMat
		 * that does not require compute shader support.
		 */
		class IrradianceComputeSHFragMat : public RendererMaterial<IrradianceComputeSHFragMat>
		{
			RMAT_DEF("IrradianceComputeSHFrag.bsl")

		public:
			IrradianceComputeSHFragMat() = default;
			void Initialize() override;

			/**
			 * Computes spherical harmonic coefficients from a face of an input cube radiance texture and outputs them to the
			 * specified face of the output cube texture. Only a single coefficient is output per execution. The output texture
			 * will contain the coefficients for red, green and blue channels in the corresponding texture channels, and
			 * per-texel weight in the alpha channel. Output coefficients must be summed up and normalized before use (using
			 * IrradianceAccumulateCubeSH).
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 coefficientIdx, const SPtr<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. Note that the
			 * output texture is a cubemap but the execute() method expects a render target that is a single face of a
			 * cubemap.
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& source);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceAccumulateSHParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeMip)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gHalfPixel)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

		/**
		 * Downsamples a cubemap face containing SH coefficient and weight values as output by IrradianceComputeSHFragMat. Each
		 * downsample sums up 2x2 pixel area coefficients/weights from the previous mip level.
		 */
		class IrradianceAccumulateSHMat : public RendererMaterial<IrradianceAccumulateSHMat>
		{
			RMAT_DEF("IrradianceAccumulateSH.bsl")

		public:
			IrradianceAccumulateSHMat() = default;
			void Initialize() override;

			/**
			 * Downsamples the provided face and mip level of the source texture and outputs the downsampled (i.e summed up)
			 * values in the resulting output texture.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 face, u32 sourceMip, const SPtr<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. Note the output
			 * is a cubemap.
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc(const SPtr<Texture>& source);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		/**
		 * Accumulates SH coefficient values from all six faces of a cubemap and normalizes them. The cubemap is expected to be
		 * 1x1 in size (previously downsampled by IrradianceAccumulateSHMat). After this shader is ran for all SH coefficients
		 * the output texture will contain final valid set of SH coefficients.
		 */
		class IrradianceAccumulateCubeSHMat : public RendererMaterial<IrradianceAccumulateCubeSHMat>
		{
			RMAT_DEF("IrradianceAccumulateCubeSH.bsl")

		public:
			IrradianceAccumulateCubeSHMat() = default;
			void Initialize() override;

			/**
			 * Sums up all faces of the input cube texture and writes the value to the corresponding index in the output
			 * texture. The source mip should point to a mip level with size 1x1.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, u32 sourceMip, const Vector2I& outputOffset, u32 coefficientIdx, const SPtr<RenderTarget>& output);

			/**
			 * Returns the texture descriptor that can be used for initializing the output render target. The render target
			 * will be able to hold all required SH coefficients (even though execute() outputs just one coefficient at a time).
			 */
			static PooledRenderTextureCreateInformation GetOutputDesc();

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		B3D_UNIFORM_BUFFER_BEGIN(IrradianceProjectSHParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(int, gCubeFace)
		B3D_UNIFORM_BUFFER_END

		extern IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

		/**
		 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMat and projects them onto faces of
		 * a cubemap.
		 */
		class IrradianceProjectSHMat : public RendererMaterial<IrradianceProjectSHMat>
		{
			RMAT_DEF("IrradianceProjectSH.bsl")

		public:
			IrradianceProjectSHMat() = default;
			void Initialize() override;

			/**
			 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMat and projects them onto faces of
			 * a cubemap.
			 */
			void Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& shCoeffs, u32 face, const SPtr<RenderTarget>& target);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GpuParameterSampledTexture mInputTexture;
		};

		/** Render beast implementation of IBLUtility. */
		class RenderBeastIBLUtility : public IBLUtility
		{
		public:
			void FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const override;
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const override;
			void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const override;
			void ScaleCubemap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const override;

		private:
			/**
			 * Downsamples a cubemap using hardware bilinear filtering.
			 *
			 * @param[in]	src		Cubemap to downsample.
			 * @param[in]   srcMip	Determines which mip level of the source texture to downsample.
			 * @param[in]   dst		Desination texture to output the scaled data to. Must be usable as a render target.
			 * @param[in]   dstMip	Determines which mip level of the destination texture to scale.
			 */
			static void DownsampleCubemap(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip);

			/**
			 * Generates irradiance SH coefficients from the input cubemap and writes them to a 1D texture. Does not make
			 * use of the compute shader.
			 */
			static void FilterCubemapForIrradianceNonCompute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, u32 outputIdx, const SPtr<RenderTexture>& output);
		};

		/** @} */
	} // namespace render
} // namespace b3d
