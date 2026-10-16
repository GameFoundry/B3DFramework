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

			/**
			 * Creates the parameter set of @p pass, which renders into a face of the downsampled cubemap, and assigns @p mip of
			 * @p source as its input. Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, u32 mip);

			/** Binds the material and @p set, a set from Prepare(), and draws a quad covering the pass's attachment. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

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

			/**
			 * Creates the parameter set of @p pass, which renders into @p face and @p mip of the filtered cubemap, and assigns
			 * @p source as the cubemap to importance sample. Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, u32 mip);

			/** Binds the material and @p set, a set from Prepare(), and draws a quad covering the pass's attachment. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

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
			 * Creates the parameter set of @p pass, which computes spherical harmonic coefficients from @p face of a radiance
			 * texture and outputs a list of coefficient sets (one set of coefficients for each thread group). Coefficients must
			 * be reduced and normalized by IrradianceReduceSHMaterial before use. Returns the set to pass to Execute().
			 *
			 * @param	pass		Pass that dispatches the material.
			 * @param	source		Radiance cubemap.
			 * @param	face		Face of @p source to compute the coefficients of.
			 * @param	output		Buffer to write the coefficient sets to, created from GetOutputBufferInformation().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& source, u32 face, const TShared<GpuBuffer>& output);

			/** Binds the material and @p set, a set from Prepare(), and dispatches the thread groups for a face of @p source. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const TShared<Texture>& source) const;

			/**
			 * Returns the information of a buffer of adequate size to be used as output for this material, and the number of
			 * coefficient sets it holds in @p numCoeffSets.
			 */
			GpuBufferCreateInformation GetOutputBufferInformation(const TShared<Texture>& source, u32& numCoeffSets);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are
			 *					supported.
			 */
			static IrradianceComputeSHMaterial* GetVariation(int order = 5);

		private:
			/** Returns the number of thread groups that compute the coefficients of a face of @p source. */
			static Vector2I GetDispatchSize(const TShared<Texture>& source);

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
			 * Creates the parameter set of @p pass, which sums spherical harmonic coefficients calculated by each thread group of
			 * IrradianceComputeSHMaterial and outputs a single set of normalized coefficients. Returns the set to pass to
			 * Execute().
			 *
			 * @param	pass			Pass that dispatches the material.
			 * @param	source			Coefficient sets output by IrradianceComputeSHMaterial.
			 * @param	numCoeffSets	Number of coefficient sets in @p source.
			 * @param	output			Texture to write the coefficients to, created from GetOutputTextureInformation().
			 * @param	outputIdx		Position in @p output to write the coefficients at.
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& source, u32 numCoeffSets, const TShared<Texture>& output, u32 outputIdx);

			/** Binds the material and @p set, a set from Prepare(), and dispatches it. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

			/** Returns the information of a texture of adequate size to be used as output for this material. */
			TextureCreateInformation GetOutputTextureInformation(u32 numCoeffSets);

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
			 * Creates the parameter set of @p pass, which renders into @p face of a cubemap, and assigns the spherical harmonic
			 * coefficients calculated by IrradianceReduceSHMaterial to project onto it. Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<Texture>& shCoeffs, u32 face);

			/** Binds the material and @p set, a set from Prepare(), and draws a quad covering the pass's attachment. */
			void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set) const;

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mInputTextureParameter;
		};

		/** Render beast implementation of IBLUtility. */
		class RenderBeastIBLUtility : public IBLUtility
		{
		public:
			void FilterCubemapForSpecular(FrameGraph& graph, const TShared<Texture>& cubemap) const override;
			void FilterCubemapForIrradiance(FrameGraph& graph, const TShared<Texture>& cubemap, const TShared<Texture>& output) const override;
			void FilterCubemapForIrradiance(FrameGraph& graph, const TShared<Texture>& cubemap, const TShared<Texture>& output, u32 outputIdx) const override;
			void ScaleCubemap(FrameGraph& graph, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip) const override;

		private:
			/**
			 * Downsamples a cubemap using hardware bilinear filtering. Adds a pass per face to @p graph, whose resources @p src
			 * and @p dst must be.
			 *
			 * @param[in]	graph	Graph to add the passes to.
			 * @param[in]	src		Cubemap to downsample.
			 * @param[in]   srcMip	Determines which mip level of the source texture to downsample.
			 * @param[in]   dst		Desination texture to output the scaled data to. Must be usable as a render target.
			 * @param[in]   dstMip	Determines which mip level of the destination texture to scale.
			 */
			static void DownsampleCubemap(FrameGraph& graph, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip);

			/**
			 * Adds a pass per face of @p cubemap to @p graph, computing the spherical harmonic coefficient sets of order
			 * @p shOrder. @p cubemap must be a resource of @p graph. Returns the transient buffer holding the sets, and their
			 * number in @p numCoeffSets.
			 */
			static TShared<GpuBuffer> ComputeCoefficientSets(FrameGraph& graph, const TShared<Texture>& cubemap, int shOrder, u32& numCoeffSets);
		};

		/** @} */
	} // namespace render
} // namespace b3d
