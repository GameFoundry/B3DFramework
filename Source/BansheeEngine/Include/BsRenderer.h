//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsCoreRenderer.h"

namespace BansheeEngine
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

		/**
		 * Attempts to retrieve a renderable element for a renderable with the specified renderable ID, and a sub-mesh
		 * index of the mesh. Returns null if element cannot be found. Returned value is transient and should be stored.
		 */
		virtual const RenderableElement* getRenderableElem(UINT32 id, UINT32 subMeshIdx) = 0;
	};

	/**	Provides easy access to Renderer. */
	SPtr<Renderer> BS_EXPORT gRenderer();

	/** @} */
}