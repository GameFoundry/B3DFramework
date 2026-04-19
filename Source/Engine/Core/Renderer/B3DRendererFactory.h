//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Factory class for creating Renderer objects. Implement this class for any custom renderer classes you may have, and
	 * register it with renderer manager.
	 *
	 * @see		RendererManager
	 */
	class B3D_EXPORT RendererFactory
	{
	public:
		virtual ~RendererFactory() = default;

		/**	Creates a new instance of the renderer. */
		virtual SPtr<render::Renderer> Create() = 0;

		/**	Returns the name of the renderer this factory creates. */
		virtual const String& Name() const = 0;
	};

	/** @} */
} // namespace b3d
