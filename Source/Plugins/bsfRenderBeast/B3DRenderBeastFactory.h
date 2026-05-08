//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererFactory.h"

namespace b3d
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

		TShared<render::Renderer> Create() override;
		const String& Name() const override;
	};

	/** @} */
} // namespace b3d
