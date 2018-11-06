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
	BS_PARAM_BLOCK_END

	extern DecalParamDef gDecalParamDef;

	struct MaterialSamplerOverrides;

	/** Default material used for rendering decals, when no other is available. */
	class DefaultDecalMat : public RendererMaterial<DefaultDecalMat> { RMAT_DEF("Decal.bsl"); };

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

		/** Time to used for evaluating material animation. */
		float materialAnimationTime = 0.0f;

		/** Texture input for the depth buffer. */
		GpuParamTexture depthInputTexture;

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
