//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRenderElement.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Material/B3DMaterialParam.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Renderer/B3DRendererMaterial.h"

namespace b3d
{
	namespace render
	{
		class Decal;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(DecalParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gWorldToDecal)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gDecalNormal)
			B3D_UNIFORM_BUFFER_MEMBER(float, gNormalTolerance)
			B3D_UNIFORM_BUFFER_MEMBER(float, gFlipDerivatives)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gLayerMask)
		B3D_UNIFORM_BUFFER_END

		extern DecalParamDef gDecalParamDef;

		struct MaterialSamplerOverrides;

		/** Default material used for rendering decals, when no other is available. */
		class DefaultDecalMat : public RendererMaterial<DefaultDecalMat>
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

		/** Contains information required for rendering a single Decal. */
		class DecalRenderElement : public RenderElement
		{
		public:
			/**
			 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
			 * on a global scale (for example filtering most commonly).
			 */
			MaterialSamplerOverrides* SamplerOverrides;

			/** Binding indices representing where should the per-camera param block buffer be bound to. */
			GpuParameterBinding PerCameraBinding;

			/** Indices for different variations of the used material. */
			u32 TechniqueIndices[2][3];

			/** Time to used for evaluating material animation. */
			float MaterialAnimationTime = 0.0f;

			/** Texture input for the depth buffer. */
			GpuParameterSampledTexture DepthInputTexture;

			/** Texture input for the mask buffer. */
			GpuParameterSampledTexture MaskInputTexture;

			void Draw(GpuCommandBuffer& commandBuffer) const override;
		};

		/** Contains information about a Decal, used by the Renderer. */
		struct RendererDecal
		{
			RendererDecal();

			/** Updates the per-object GPU buffer according to the currently set properties. */
			void UpdatePerObjectBuffer();

			Decal* Decal;
			mutable DecalRenderElement RenderElement;

			SPtr<GpuBuffer> DecalParamBuffer;
			SPtr<GpuBuffer> PerObjectParamBuffer;
			SPtr<GpuBuffer> PerCallParamBuffer;
		};

		/** @} */
	} // namespace render
} // namespace b3d
