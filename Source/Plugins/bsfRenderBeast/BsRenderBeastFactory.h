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
	 * Renderer factory implementation that creates and initializes the default framework renderer. Used by the
	 * RendererManager.
	 */
	class RenderBeastFactory : public RendererFactory
	{
	public:
		static constexpr const char* kSystemName = "bsfRenderBeast";

		SPtr<ct::Renderer> Create() override;
		const String& Name() const override;
	};

	/** @} */
} // namespace bs
