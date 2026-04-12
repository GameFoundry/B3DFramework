//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Plugin/B3DPluginLoader.h"
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
		 * Loads the renderer plugin with the provided name and makes it the active renderer. You must call Initialize()
		 * after setting the active renderer to properly activate it.
		 */
		void SetActive(const String& pluginName);

		/** Initializes the currently active renderer on the provided GPU device, making it ready to render. */
		void Initialize(const SPtr<GpuDevice>& gpuDevice);

		/** Queues GPU command capture of the next frame, if a frame capture is set up. */
		B3D_SCRIPT_EXPORT()
		void RequestFrameCapture();

		/**	Returns the currently active renderer. Null if no renderer is active. */
		SPtr<render::Renderer> GetActive() { return mActiveRenderer; }

	private:
		LoadedPlugin mPlugin;
		RendererFactory* mFactory = nullptr;
		SPtr<render::Renderer> mActiveRenderer;
	};

	/** @} */
} // namespace b3d
