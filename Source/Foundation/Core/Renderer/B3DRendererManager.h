//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Allows you to change and retrieve the active renderer. Active renderer will be used for rendering all objects in
	 * the following frame.
	 *
	 * @note	No renderer is active by default. You must make a renderer active before doing any rendering.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) RendererManager : public Module<RendererManager>
	{
	public:
		~RendererManager();

		/**
		 * Attempts to find a renderer with the specified name and makes it active. Exception is thrown if renderer with
		 * the specified name doesn't exist. You must call Initialize() after setting the active renderer to properly
		 * activate it.
		 */
		void SetActive(const String& name);

		/** Initializes the currently active renderer on the provided GPU device, making it ready to render. */
		void Initialize(const SPtr<GpuDevice>& gpuDevice);

		/** Queues GPU command capture of the next frame, if a frame capture is set up. */
		B3D_SCRIPT_EXPORT()
		void RequestFrameCapture();

		/**	Returns the currently active renderer. Null if no renderer is active. */
		SPtr<render::Renderer> GetActive() { return mActiveRenderer; }

		/**
		 * Registers a new renderer factory. Any renderer you try to make active with setActive() you will need to have
		 * previously registered here.
		 */
		void RegisterFactoryInternal(SPtr<RendererFactory> factory);

	private:
		Vector<SPtr<RendererFactory>> mAvailableFactories;

		SPtr<render::Renderer> mActiveRenderer;
	};

	/** @} */
} // namespace b3d
