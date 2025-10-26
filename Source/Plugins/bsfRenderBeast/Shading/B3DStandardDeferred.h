//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Renderer/B3DRendererMaterial.h"
#include "B3DRendererLight.h"
#include "B3DRendererReflectionProbe.h"

namespace b3d
{
	namespace render
	{
		class RendererLight;

		B3D_PARAM_BLOCK_BEGIN(PerLightParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightPositionAndSrcRadius)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightColorAndLuminance)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightSpotAnglesAndSqrdInvAttRadius)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightDirectionAndBoundRadius)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gShiftedLightPositionAndType)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightGeometry)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatConeTransform)
		B3D_PARAM_BLOCK_END

		extern PerLightParamDef gPerLightParamDef;

		/** Shader that renders directional light sources during deferred rendering light pass. */
		class DeferredDirectionalLightMat : public RendererMaterial<DeferredDirectionalLightMat>
		{
			RMAT_DEF("DeferredDirectionalLight.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool MSAA, bool MSAA_RESOLVE_0TH>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", MSAA),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", MSAA_RESOLVE_0TH) });

				return variation;
			}

		public:
			DeferredDirectionalLightMat() = default;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredDirectionalLightMat* GetVariation(bool msaa, bool singleSampleMSAA = false);
		};

		/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
		class DeferredPointLightMat : public RendererMaterial<DeferredPointLightMat>
		{
			RMAT_DEF("DeferredPointLight.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool INSIDE_GEOMETRY, bool MSAA, bool MSAA_RESOLVE_0TH>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", MSAA),
					  ShaderVariationParameter("INSIDE_GEOMETRY", INSIDE_GEOMETRY),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", MSAA_RESOLVE_0TH) });

				return variation;
			}

		public:
			DeferredPointLightMat() = default;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	inside				Set to true if viewer is inside the light's stencil geometry.
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredPointLightMat* GetVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
		};

		B3D_PARAM_BLOCK_BEGIN(PerProbeParamDef)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gPosition)
			B3D_PARAM_BLOCK_ENTRY(Vector3, gExtents)
			B3D_PARAM_BLOCK_ENTRY(float, gTransitionDistance)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gInvBoxTransform)
			B3D_PARAM_BLOCK_ENTRY(i32, gCubemapIdx)
			B3D_PARAM_BLOCK_ENTRY(i32, gType)
		B3D_PARAM_BLOCK_END

		extern PerProbeParamDef gPerProbeParamDef;

		/**
		 * Shader that prepares the surface for image based lighting.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Needs to be followed by execution of all other DeferredIBL* materials.
		 */
		class DeferredIBLSetupMat : public RendererMaterial<DeferredIBLSetupMat>
		{
			RMAT_DEF("DeferredIBLSetup.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			DeferredIBLSetupMat() = default;
			void Initialize() override;

			/** Binds the material for rendering and sets up any parameters. */
			void Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuBuffer>& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLSetupMat* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParams mIBLParams;
		};

		/**
		 * Shader that renders an individual reflection probe for image based lighting.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Must be preceeded by DeferredIBLSetupMat and followed by DeferredIBLSkyMat and DeferredIBLFinalizeMat.
		 */
		class DeferredIBLProbeMat : public RendererMaterial<DeferredIBLProbeMat>
		{
			RMAT_DEF("DeferredIBLProbe.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool inside, bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("INSIDE_GEOMETRY", inside),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			DeferredIBLProbeMat() = default;
			void Initialize() override;

			/** Binds the material for rendering and sets up any parameters. */
			void Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuBuffer>& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	inside				Set to true if viewer is inside the probe's stencil geometry.
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLProbeMat* GetVariation(bool inside, bool msaa, bool singleSampleMSAA = false);

		private:
			SPtr<GpuBuffer> mParamBuffer;
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParams mIBLParams;
		};

		/**
		 * Shader that renders the sky reflections. The results are additively blended with the currently bound render target.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Must be preceeded by DeferredIBLSetupMat and followed by DeferredIBLFinalizeMat.
		 */
		class DeferredIBLSkyMat : public RendererMaterial<DeferredIBLSkyMat>
		{
			RMAT_DEF("DeferredIBLSky.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			DeferredIBLSkyMat() = default;
			void Initialize() override;

			/** Binds the material for rendering and sets up any parameters. */
			void Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const Skybox* skybox, const SPtr<GpuBuffer>& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLSkyMat* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParams mIBLParams;
		};

		/**
		 * Material that finalizes the rendering of reflections. As input it takes the texture output by previous DeferredIBL*
		 * materials, and the resulting output is blended additively with the current render target.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 */
		class DeferredIBLFinalizeMat : public RendererMaterial<DeferredIBLFinalizeMat>
		{
			RMAT_DEF("DeferredIBLFinalize.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool msaa, bool singleSampleMSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("MSAA", msaa),
					  ShaderVariationParameter("MSAA_RESOLVE_0TH", singleSampleMSAA) });

				return variation;
			}

		public:
			DeferredIBLFinalizeMat() = default;
			void Initialize() override;

			/** Binds the material for rendering and sets up any parameters. */
			void Bind(GpuCommandBuffer& commandBuffer, const GBufferTextures& gBufferInput, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const SPtr<GpuBuffer>& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
			 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLFinalizeMat* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParams mIBLParams;
			GpuParameterSampledTexture mIBLRadiance;
		};

		/** Provides functionality for standard (non-tiled) deferred rendering. */
		class StandardDeferred : public Module<StandardDeferred>
		{
		public:
			/** Material variation key for grouping lights by material type. */
			struct MaterialVariationKey
			{
				LightType Type;
				bool IsMSAA;
				bool IsInside; // For point/spot lights only
				bool IsSingleSampleMSAA;

				bool operator<(const MaterialVariationKey& other) const
				{
					if(Type != other.Type) return Type < other.Type;
					if(IsMSAA != other.IsMSAA) return IsMSAA < other.IsMSAA;
					if(IsInside != other.IsInside) return IsInside < other.IsInside;
					return IsSingleSampleMSAA < other.IsSingleSampleMSAA;
				}
			};

			/** Information about a single light instance in a batch. */
			struct BatchedLightInstance
			{
				const RendererLight* Light;
				u32 UniformBufferOffset; /**< Byte offset in the instanced uniform buffer. */
			};

			/** Group of lights sharing the same material variation. */
			struct LightBatch
			{
				// Material pointers
				SPtr<Mesh> StencilMesh; // For point/spot lights

				// Lights in this group
				TArray<BatchedLightInstance> Lights;

				// Shared GPU resources
				SPtr<GpuBuffer> PerLightUniformBuffer; // Instanced buffer
				SPtr<GpuParameters> GpuParameters; // Single GpuParameters for all lights
				u32 DynamicOffsetIndex; // Index for SetDynamicBufferOffset
				u32 UniformStride; // Stride between light instances in buffer
			};

			/** Batch of lights grouped by material variation. */
			struct LightBatches
			{
				Map<MaterialVariationKey, LightBatch> Batches;
			};

			/**
			 * Groups lights using the same material together in a batch, and prepares uniform buffers so all the light rendering can happen with the same
			 * uniform buffer (using dynamic offsets).
			 *
			 * @param lights            Lights to batch.
			 * @param view              View to render from.
			 * @param gBufferInput      GBuffer textures.
			 * @param lightOcclusion    Shadow occlusion texture (or Texture::kBlack if no shadows).
			 * @return                  Prepared batch containing grouped lights and GPU resources.
			 */
			LightBatches PrepareLightBatches(const TArrayView<const RendererLight*>& lights, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion);

			/**
			 * Renders a prepared light batch using dynamic offsets.
			 * Must be called within an active render pass.
			 *
			 * @param commandBuffer     Command buffer to render with.
			 * @param batches           Prepared light batch.
			 */
			void RenderLightBatches(GpuCommandBuffer& commandBuffer, const LightBatches& batches);

			/** Calculates lighting for the specified light, using the standard deferred renderer. */
			void RenderLight(GpuCommandBuffer& commandBuffer, LightType type, const RendererLight& light, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion);

			/**
			 * Evaluates filtered radiance from a single reflection probe and blends it into the current render target.
			 * Alpha value of the render target is used for determining the contribution and will be updated with new
			 * contibution after blending.
			 */
			void RenderReflProbe(GpuCommandBuffer& commandBuffer, const ReflProbeData& probeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const SPtr<GpuBuffer>& reflProbeParams);
		};
	} // namespace render
} // namespace b3d
