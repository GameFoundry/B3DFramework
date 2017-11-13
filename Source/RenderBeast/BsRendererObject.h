//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderableElement.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsParamBlocks.h"
#include "Material/BsMaterialParam.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "BsImageBasedLighting.h"

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

	struct MaterialSamplerOverrides;

	/**
	 * @copydoc	RenderableElement
	 *
	 * Contains additional data specific to RenderBeast renderer.
	 */
	class BeastRenderableElement : public RenderableElement
	{
	public:
		/**
		 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
		 * on a global scale (for example filtering most commonly).
		 */
		MaterialSamplerOverrides* samplerOverrides;

		/** All GPU parameters from the material used by the renderable. */
		SPtr<GpuParamsSet> params;

		/**	Identifier of the owner renderable. */
		UINT32 renderableId;

		/** Identifier of the animation running on the renderable's mesh. -1 if no animation. */
		UINT64 animationId;

		/** Type of animation applied to this element, if any. */
		RenderableAnimType animType;

		/** Index of the technique in the material to render the element with. */
		UINT32 techniqueIdx;

		/** Binding indices representing where should the per-camera param block buffer be bound to. */
		GpuParamBinding perCameraBindings[GPT_COUNT];

		/** Binding indices representing where should lights param block buffer be bound to. */
		GpuParamBinding gridParamsBindings[GPT_COUNT];

		/** 
		 * Parameter to which to bind a buffer containing light grid offsets and size, per grid cell. Used for forward
		 * rendering. 
		 */
		GpuParamBuffer gridLightOffsetsAndSizeParam;

		/** Parameter to which to bind a buffer containing all light indices, as mapped by grid offsets & size. */
		GpuParamBuffer gridLightIndicesParam;

		/** Parameter to which to bind light buffer used for forward rendering. */
		GpuParamBuffer lightsBufferParam;

		/** 
		 * Parameter to which to bind a buffer containing reflection probe grid offsets and size, per grid cell. Used for
		 * forward rendering. 
		 */
		GpuParamBuffer gridProbeOffsetsAndSizeParam;

		/** Collection of parameters used for image based lighting. */
		ImageBasedLightingParams imageBasedParams;

		/** GPU buffer containing element's bone matrices, if it requires any. */
		SPtr<GpuBuffer> boneMatrixBuffer;

		/** Vertex buffer containing element's morph shape vertices, if it has any. */
		SPtr<VertexBuffer> morphShapeBuffer;

		/** Vertex declaration used for rendering meshes containing morph shape information. */
		SPtr<VertexDeclaration> morphVertexDeclaration;

		/** Version of the morph shape vertices in the buffer. */
		mutable UINT32 morphShapeVersion;
	};

	 /** Contains information about a Renderable, used by the Renderer. */
	struct RendererObject
	{
		RendererObject();

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
		Vector<BeastRenderableElement> elements;

		SPtr<GpuParamBlockBuffer> perObjectParamBuffer;
		SPtr<GpuParamBlockBuffer> perCallParamBuffer;
	};

	/** @} */
}}
