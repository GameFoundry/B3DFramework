//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Renderer/B3DRendererMaterial.h"
#include "RenderState/B3DLightRenderState.h"
#include "B3DRendererReflectionProbe.h"

namespace b3d
{
	namespace render
	{
		class LightRenderState;

		B3D_UNIFORM_BUFFER_BEGIN(PerLightUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gLightPositionAndSrcRadius)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gLightColorAndLuminance)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gLightSpotAnglesAndSqrdInvAttRadius)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gLightDirectionAndBoundRadius)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gShiftedLightPositionAndType)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gLightGeometry)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatConeTransform)
		B3D_UNIFORM_BUFFER_END

		extern PerLightUniformDefinition gPerLightUniformDefinition;

		/** Shader that renders directional light sources during deferred rendering light pass. */
		class DeferredDirectionalLightMaterial : public RendererMaterial<DeferredDirectionalLightMaterial>
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
			DeferredDirectionalLightMaterial() = default;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredDirectionalLightMaterial* GetVariation(bool msaa, bool singleSampleMSAA = false);
		};

		/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
		class DeferredPointLightMaterial : public RendererMaterial<DeferredPointLightMaterial>
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
			DeferredPointLightMaterial() = default;

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	inside				Set to true if viewer is inside the light's stencil geometry.
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredPointLightMaterial* GetVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
		};

		B3D_UNIFORM_BUFFER_BEGIN(PerProbeUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gPosition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gExtents)
			B3D_UNIFORM_BUFFER_MEMBER(float, gTransitionDistance)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gInvBoxTransform)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gCubemapIdx)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gType)
		B3D_UNIFORM_BUFFER_END

		extern PerProbeUniformDefinition gPerProbeUniformDefinition;

		/**
		 * Shader that prepares the surface for image based lighting.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Needs to be followed by execution of all other DeferredIBL* materials.
		 */
		class DeferredIBLSetupMaterial : public RendererMaterial<DeferredIBLSetupMaterial>
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
			DeferredIBLSetupMaterial() = default;
			void Initialize() override;

			/** Prepares material parameters for rendering. */
			void Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const GpuBufferSuballocation& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLSetupMaterial* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParameterBinding mIBLParams;
		};

		/**
		 * Shader that renders an individual reflection probe for image based lighting.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Must be preceeded by DeferredIBLSetupMaterial and followed by DeferredIBLSkyMaterial and DeferredIBLFinalizeMaterial.
		 */
		class DeferredIBLProbeMaterial : public RendererMaterial<DeferredIBLProbeMaterial>
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
			DeferredIBLProbeMaterial() = default;

			/** Populates the provided GPU parameters with the provided parameters. */
			static void PopulateParameters(GpuDevice& gpuDevice, const SPtr<GpuParameterSet>& gpuParameters, const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const SceneInfo& sceneInfo, const GpuBufferSuballocation& perProbeUniformBuffer, const GpuBufferSuballocation& globalProbeUniformBuffer);

			/** Creates a new transient uniform buffer containing provided per-probe data. */
			static GpuBufferSuballocation CreatePerProbeUniformBuffer(const ReflectioneProbeData& probeData);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	inside				Set to true if viewer is inside the probe's stencil geometry.
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLProbeMaterial* GetVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
		};

		/**
		 * Shader that renders the sky reflections. The results are additively blended with the currently bound render target.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 * Must be preceeded by DeferredIBLSetupMaterial and followed by DeferredIBLFinalizeMaterial.
		 */
		class DeferredIBLSkyMaterial : public RendererMaterial<DeferredIBLSkyMaterial>
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
			DeferredIBLSkyMaterial() = default;
			void Initialize() override;

			/** Prepares material parameters for rendering. */
			void Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const Skybox* skybox, const GpuBufferSuballocation& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLSkyMaterial* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParameterBinding mIBLParams;
		};

		/**
		 * Material that finalizes the rendering of reflections. As input it takes the texture output by previous DeferredIBL*
		 * materials, and the resulting output is blended additively with the current render target.
		 *
		 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
		 */
		class DeferredIBLFinalizeMaterial : public RendererMaterial<DeferredIBLFinalizeMaterial>
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
			DeferredIBLFinalizeMaterial() = default;
			void Initialize() override;

			/** Prepares material parameters for rendering. */
			void Prepare(const GBufferTextures& gBufferInput, const GpuBufferSuballocation& perCamera, const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, const GpuBufferSuballocation& reflProbeParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	msaa				True if the shader will operate on a multisampled surface.
			 * @param	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
			 *									evaluated. Otherwise all samples will be evaluated.
			 * @return							Requested variation of the material.
			 */
			static DeferredIBLFinalizeMaterial* GetVariation(bool msaa, bool singleSampleMSAA = false);

		private:
			GBufferParameterBinding mGBufferParams;
			ImageBasedLightingParameterBinding mIBLParams;
			GpuParameterSampledTexture mIBLRadiance;
		};

		/** Provides functionality for standard (non-tiled) deferred rendering. */
		class StandardDeferred : public Module<StandardDeferred>
		{
		public:
			/** Material variation key for grouping lights by material type. */
			struct LightMaterialVariationKey
			{
				LightType Type;
				bool IsMSAA;
				bool IsInside; // For point/spot lights only
				bool IsSingleSampleMSAA;

				bool operator<(const LightMaterialVariationKey& other) const
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
				const LightRenderState* Light;
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
				SPtr<GpuParameterSet> GpuParameters; // Single GpuParameters for all lights
				u32 DynamicOffsetIndex; // Index for SetDynamicBufferOffset
				u32 UniformStride; // Stride between light instances in buffer
			};

			/** Batch of lights grouped by material variation. */
			struct LightBatches
			{
				Map<LightMaterialVariationKey, LightBatch> Batches;
			};

			/** Contains information required for rendering a single reflection probe. */
			struct ReflectionProbeRenderInformation
			{
				bool IsViewerInside = false;
				u32 Type = 0;
				SPtr<GpuParameterSet> GpuParameters;
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
			LightBatches PrepareLightBatches(const TArrayView<const LightRenderState*>& lights, const RendererView& view, const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion);

			/**
			 * Renders a prepared light batch using dynamic offsets.
			 * Must be called within an active render pass.
			 *
			 * @param commandBuffer     Command buffer to render with.
			 * @param batches           Prepared light batch.
			 */
			void RenderLightBatches(GpuCommandBuffer& commandBuffer, const LightBatches& batches);

			/** Prepares all GPU parameters required for rendering reflection probes. */
			TArray<ReflectionProbeRenderInformation> PrepareReflectionProbes(GpuDevice& device, const VisibleReflectionProbeData& visibleReflectionProbeData, const RendererView& view, const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, const GpuBufferSuballocation& globalReflectionProbeUniformBuffer);

			/**
			 * Evaluates filtered radiance from provided reflection probes and blends it into the current render target.
			 * Alpha value of the render target is used for determining the contribution and will be updated with new contibution after blending.
			 * Probes must have been prepared for rendering via a previous call to PrepareReflectionProbes.
			 */
			void RenderReflectionProbes(GpuCommandBuffer& commandBuffer, const TArray<ReflectionProbeRenderInformation>& probeRenderInformation, const RendererView& view);
		};
	} // namespace render
} // namespace b3d
