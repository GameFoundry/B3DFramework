//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsCoreRenderer.h"

namespace bs
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/** @copydoc CoreRenderer */
	class BS_EXPORT Renderer : public CoreRenderer
	{
	public:
		virtual ~Renderer() { }

		/**
		 * Called whenever a new renderable is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableAdded(RenderableCore* renderable) { }

		/**
		 * Called whenever a renderable is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableUpdated(RenderableCore* renderable) { }

		/**
		 * Called whenever a renderable is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyRenderableRemoved(RenderableCore* renderable) { }

		/**
		 * Called whenever a new light is created.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightAdded(LightCore* light) { }

		/**
		 * Called whenever a light is updated.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightUpdated(LightCore* light) { }

		/**
		 * Called whenever a light is destroyed.
		 *
		 * @note	Core thread.
		 */
		virtual void notifyLightRemoved(LightCore* light) { }
	};

	/**	Provides easy access to Renderer. */
	SPtr<Renderer> BS_EXPORT gRenderer();

	/** @} */
}