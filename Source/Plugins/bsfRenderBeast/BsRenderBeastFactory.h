//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererFactory.h"

namespace bs
{
	/** @addtogroup RenderBeast
	 *  @{
	 */
	/**
	 * Renderer factory implementation that creates and initializes the default Banshee renderer. Used by the
	 * RendererManager.
	 */
	class RenderBeastFactory : public RendererFactory
	{
	public:
		static constexpr const char* SystemName = "bsfRenderBeast";

		/** @copydoc RendererFactory::create */
		SPtr<ct::Renderer> create() override;

		/** @copydoc RendererFactory::name */
		const String& name() const override;
	};

	/** @} */
}
