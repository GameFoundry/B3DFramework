//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderableElement.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

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
		SPtr<GpuParamsSetCore> params;

		/**	Identifier of the owner renderable. */
		UINT32 renderableId;

		/** Identifier of the animation running on the renderable's mesh. -1 if no animation. */
		UINT64 animationId;

		/** Index of the technique in the material to render the element with. */
		UINT32 techniqueIdx;

		/** 
		 * Parameter for setting global bone pose transforms used for an element with skeletal animation, null otherwise. 
		 */
		MaterialParamBufferCore boneMatricesParam;

		/** GPU buffer containing element's bone matrices, if it requires any. */
		SPtr<GpuBufferCore> boneMatrixBuffer;
	};

	 /** Contains information about a Renderable, used by the Renderer. */
	struct RendererObject
	{
		RenderableCore* renderable;
		Vector<BeastRenderableElement> elements;
	};

	/** @} */
}