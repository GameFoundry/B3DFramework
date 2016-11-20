//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererFactory.h"

namespace bs
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	extern const char* SystemName;

	/**
	 * Renderer factory implementation that creates and initializes the default Banshee renderer. Used by the 
	 * RendererManager.
	 */
	class BS_BSRND_EXPORT RenderBeastFactory : public RendererFactory
	{
	public:
		/** @copydoc RendererFactory::create */
		virtual SPtr<CoreRenderer> create() override;

		/** @copydoc RendererFactory::name */
		virtual const String& name() const override;
	};

	/** @} */
}