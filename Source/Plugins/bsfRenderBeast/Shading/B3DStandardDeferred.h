//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Renderer/B3DRendererMaterial.h"
#include "RenderState/B3DLightRenderState.h"
#include "RenderState/B3DReflectionProbeRenderState.h"

namespace b3d
{
	namespace render
	{
		class RenderBeastScene;

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
				PackedRendererId LightId;
				u32 UniformBufferOffset; /**< Byte offset in the instanced uniform buffer. */
			};

			/** Group of lights sharing the same material variation. */
			struct LightBatch
			{
				// Material pointers
				TShared<Mesh> StencilMesh; // For point/spot lights

				// Lights in this group
				TArray<BatchedLightInstance> Lights;

				// Shared GPU resources
				TShared<GpuBuffer> PerLightUniformBuffer; // Instanced buffer
				TShared<GpuParameterSet> GpuParameters; // Single GpuParameters for all lights
				u32 DynamicOffsetIndex; // Index for SetDynamicBufferOffset
				u32 UniformStride; // Stride between light instances in buffer
			};

			/** Batch of lights grouped by material variation. */
			struct LightBatches
			{
				Map<LightMaterialVariationKey, LightBatch> Batches;
			};

			/**
			 * Groups lights using the same material together in a batch, and prepares uniform buffers so all the light rendering can happen with the same
			 * uniform buffer (using dynamic offsets). The parameter sets of the batches are added to @p pass.
			 *
			 * @param pass              Raster pass that renders the batches.
			 * @param lights            Lights to batch.
			 * @param view              View to render from.
			 * @param gBufferInput      GBuffer textures.
			 * @param lightOcclusion    Shadow occlusion texture (or Texture::kBlack if no shadows).
			 * @return                  Prepared batch containing grouped lights and GPU resources.
			 */
			LightBatches PrepareLightBatches(FrameGraphPass& pass, TArrayView<const PackedRendererId> lights, const RenderBeastScene& scene, const RendererView& view, const GBufferTextures& gBufferInput, const TShared<Texture>& lightOcclusion);

			/**
			 * Renders a prepared light batch using dynamic offsets.
			 *
			 * @param context           Context of the raster pass the batches were prepared for.
			 * @param batches           Prepared light batch.
			 */
			void RenderLightBatches(FrameGraphPassContext& context, const LightBatches& batches) const;
		};
	} // namespace render
} // namespace b3d
