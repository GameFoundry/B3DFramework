//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParam.h"
#include "BsRenderableElement.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"
#include "BsRendererObject.h"

namespace bs 
{ 
	struct RendererAnimationData;

	namespace ct
	{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(PerFrameParamDef)
		BS_PARAM_BLOCK_ENTRY(float, gTime)
	BS_PARAM_BLOCK_END

	extern PerFrameParamDef gPerFrameParamDef;

	/** Manages initialization and rendering of individual renderable object, represented as RenderableElement%s. */
	class ObjectRenderer
	{
	public:
		ObjectRenderer();

		/** Initializes the specified renderable element, making it ready to be used. */
		void initElement(RendererObject& owner, BeastRenderableElement& element);

		/** Updates global per frame parameter buffers with new values. To be called at the start of every frame. */
		void setParamFrameParams(float time);

	protected:
		SPtr<GpuParamBlockBuffer> mPerFrameParamBuffer;
	};

	/** Basic shader that is used when no other is available. */
	class DefaultMaterial : public RendererMaterial<DefaultMaterial> { RMAT_DEF("Default.bsl"); };

	/** @} */
}}