//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererId.h"
#include "Renderer/B3DDrawCommand.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Material/B3DMaterialParam.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Renderer/B3DRendererMaterial.h"
#include "B3DRenderState.h"

namespace b3d
{
	namespace render
	{
		class DecalProxy;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(DecalUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gWorldToDecal)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gDecalNormal)
			B3D_UNIFORM_BUFFER_MEMBER(float, gNormalTolerance)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFlipDerivatives)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gLayerMask)
		B3D_UNIFORM_BUFFER_END

		extern DecalUniformDefinition gDecalUniformDefinition;

		struct MaterialSamplerOverrides;

		/** Default material used for rendering decals, when no other is available. */
		class DefaultDecalMaterial : public RendererMaterial<DefaultDecalMaterial>
		{
			RMAT_DEF("Decal.bsl");
		};

		/** Determines how is decal blended with the underlying surface. */
		enum class DecalBlendMode
		{
			/** All decal textures are blended with the underlying surface, using alpha to determine blend amount. */
			Transparent,
			/** Albedo texture is multiplied with the underlying surface albedo, while all other textures are blended. */
			Stain,
			/** Only the normal texture is blended with the underlying surface. */
			Normal,
			/** Adds light contribution directly, without writing any other surface data. */
			Emissive
		};

		/** Returns a specific decal shader variation. */
		template <bool INSIDE_GEOMETRY, MSAAMode MSAA_MODE>
		static const ShaderVariationParameters& GetDecalShaderVariation()
		{
			static ShaderVariationParameters variation = ShaderVariationParameters(
				{
					ShaderVariationParameter("INSIDE_GEOMETRY", INSIDE_GEOMETRY),
					ShaderVariationParameter("MSAA_MODE", (i32)MSAA_MODE),
				});

			return variation;
		}

		/** Contains information required for drawing a decal. */
		class DecalDrawCommand : public DrawCommand
		{
		public:
			/**
			 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
			 * on a global scale (for example filtering most commonly).
			 */
			MaterialSamplerOverrides* SamplerOverrides;

			/** Parameter for binding the per-camera uniform buffer. */
			GpuParameterUniformBuffer PerCameraUniformBufferParameter;
			/** Parameter for binding the per-frame uniform buffer. */
			GpuParameterUniformBuffer PerFrameUniformBufferParameter;

			/** Indices for different variations of the used material. */
			u32 VariationIndices[2][3];

			/** Time to used for evaluating material animation. */
			float MaterialAnimationTime = 0.0f;

			/** Texture input for the depth buffer. */
			GpuParameterSampledTexture DepthInputTexture;

			/** Texture input for the mask buffer. */
			GpuParameterSampledTexture MaskInputTexture;

			/** Dynamic buffer offset for the decal parameters buffer. */
			u32 DecalParamBufferOffset = 0;

			void Draw(GpuCommandBuffer& commandBuffer) const override;
		};

		/** Renderer-specific state for a decal. */
		struct DecalRenderState : RenderState
		{
			/** Updates the per-object data from a DecalProxy. */
			void UpdatePerObjectData(const DecalProxy& proxy);

			mutable DecalDrawCommand DrawCommand;

			/** Suballocation for decal-specific uniform buffer data. */
			GpuBufferSuballocation DecalParamSuballocation;
		};

		/** @} */
	} // namespace render
} // namespace b3d
