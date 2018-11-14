//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderElement.h"
#include "Renderer/BsParamBlocks.h"
#include "Material/BsMaterialParam.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "Renderer/BsRendererMaterial.h"

namespace bs { namespace ct
{
	class Decal;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(DecalParamDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gWorldToDecal)
		BS_PARAM_BLOCK_ENTRY(Vector3, gDecalNormal)
		BS_PARAM_BLOCK_ENTRY(float, gNormalTolerance)
		BS_PARAM_BLOCK_ENTRY(INT32, gLayerMask)
	BS_PARAM_BLOCK_END

	extern DecalParamDef gDecalParamDef;

	struct MaterialSamplerOverrides;

	/** Default material used for rendering decals, when no other is available. */
	class DefaultDecalMat : public RendererMaterial<DefaultDecalMat> { RMAT_DEF("Decal.bsl"); };

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
	template<bool INSIDE_GEOMETRY, MSAAMode MSAA_MODE>
	static const ShaderVariation& getDecalShaderVariation()
	{
		static ShaderVariation variation = ShaderVariation(
		{
			ShaderVariation::Param("INSIDE_GEOMETRY", INSIDE_GEOMETRY),
			ShaderVariation::Param("MSAA_MODE", (INT32)MSAA_MODE),
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
		MaterialSamplerOverrides* samplerOverrides;

		/** Binding indices representing where should the per-camera param block buffer be bound to. */
		GpuParamBinding perCameraBindings[GPT_COUNT];

		/** Indices for different variations of the used material. */
		UINT32 techniqueIndices[2][3];

		/** Time to used for evaluating material animation. */
		float materialAnimationTime = 0.0f;

		/** Texture input for the depth buffer. */
		GpuParamTexture depthInputTexture;

		/** Texture input for the mask buffer. */
		GpuParamTexture maskInputTexture;

		/** @copydoc RenderElement::draw */
		void draw() const override;
	};

	 /** Contains information about a Decal, used by the Renderer. */
	struct RendererDecal
	{
		RendererDecal();

		/** Updates the per-object GPU buffer according to the currently set properties. */
		void updatePerObjectBuffer();

		/** 
		 * Updates the per-call GPU buffer according to the provided parameters. 
		 * 
		 * @param[in]	viewProj	Combined view-projection matrix of the current camera.
		 * @param[in]	flush		True if the buffer contents should be immediately flushed to the GPU.
		 */
		void updatePerCallBuffer(const Matrix4& viewProj, bool flush = true) const;

		Decal* decal;
		mutable DecalRenderElement renderElement;

		SPtr<GpuParamBlockBuffer> decalParamBuffer;
		SPtr<GpuParamBlockBuffer> perObjectParamBuffer;
		SPtr<GpuParamBlockBuffer> perCallParamBuffer;
	};

	/** @} */
}}
