//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderElement.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsParamBlocks.h"
#include "Material/BsMaterialParam.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "BsRendererReflectionProbe.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(PerObjectParamDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatWorld)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorld)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldNoScale)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorldNoScale)
		BS_PARAM_BLOCK_ENTRY(float, gWorldDeterminantSign)
	BS_PARAM_BLOCK_END

	extern PerObjectParamDef gPerObjectParamDef;

	BS_PARAM_BLOCK_BEGIN(PerCallParamDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldViewProj)
	BS_PARAM_BLOCK_END

	extern PerCallParamDef gPerCallParamDef;

	/** Helper class used for manipulating the PerObject parameter buffer. */
	class PerObjectBuffer
	{
	public:
		/** Updates the provided buffer with the data from the provided matrices. */
		static void update(SPtr<GpuParamBlockBuffer>& buffer, const Matrix4& tfrm, const Matrix4& tfrmNoScale);
	};

	struct MaterialSamplerOverrides;

	/**
	 * Contains information required for rendering a single Renderable sub-mesh, representing a generic static or animated
	 * 3D model.
	 */
	class RenderableElement : public RenderElement
	{
	public:
		/**
		 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
		 * on a global scale (for example filtering most commonly).
		 */
		MaterialSamplerOverrides* samplerOverrides;

		/** Identifier of the animation running on the renderable's mesh. -1 if no animation. */
		UINT64 animationId;

		/** Type of animation applied to this element, if any. */
		RenderableAnimType animType;

		/** Binding indices representing where should the per-camera param block buffer be bound to. */
		GpuParamBinding perCameraBindings[GPT_COUNT];

		/** Collection of parameters used for direct lighting using the forward rendering path. */
		ForwardLightingParams forwardLightingParams;

		/** Collection of parameters used for image based lighting. */
		ImageBasedLightingParams imageBasedParams;
		
		/** GPU buffer containing element's bone matrices, if it requires any. */
		SPtr<GpuBuffer> boneMatrixBuffer;

		/** Vertex buffer containing element's morph shape vertices, if it has any. */
		SPtr<VertexBuffer> morphShapeBuffer;

		/** Vertex declaration used for rendering meshes containing morph shape information. */
		SPtr<VertexDeclaration> morphVertexDeclaration;

		/** Time to used for evaluating material animation. */
		float materialAnimationTime = 0.0f;

		/** Version of the morph shape vertices in the buffer. */
		mutable UINT32 morphShapeVersion;

		/** @copydoc RenderElement::draw */
		void draw() const override;
	};

	 /** Contains information about a Renderable, used by the Renderer. */
	struct RendererRenderable
	{
		RendererRenderable();

		/** Updates the per-object GPU buffer according to the currently set properties. */
		void updatePerObjectBuffer();

		/** 
		 * Updates the per-call GPU buffer according to the provided parameters. 
		 * 
		 * @param[in]	viewProj	Combined view-projection matrix of the current camera.
		 * @param[in]	flush		True if the buffer contents should be immediately flushed to the GPU.
		 */
		void updatePerCallBuffer(const Matrix4& viewProj, bool flush = true);

		Renderable* renderable;
		Vector<RenderableElement> elements;

		SPtr<GpuParamBlockBuffer> perObjectParamBuffer;
		SPtr<GpuParamBlockBuffer> perCallParamBuffer;
	};

	/** @} */
}}
