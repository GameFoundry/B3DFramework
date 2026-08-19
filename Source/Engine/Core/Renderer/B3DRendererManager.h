//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
		 * Loads the renderer plugin with the provided name, makes it the active renderer, and binds it to the
		 * provided GPU device. Call Initialize() afterwards, once built-in resources are available, to make the
		 * renderer ready to render.
		 *
		 * @param	pluginName	Name of the renderer plugin to load.
		 * @param	gpuDevice	Device the renderer will perform its work against.
		 */
		void SetActive(const String& pluginName, const TShared<GpuDevice>& gpuDevice);

		/** Completes initialization of the currently active renderer, making it ready to render. See SetActive(). */
		void Initialize();

		/** Queues capture of the next frame's GPU commands, if a command capture tool is available. */
		B3D_SCRIPT_EXPORT()
		void RequestGPUCommandCapture();

		/**	Returns the currently active renderer. Null if no renderer is active. */
		TShared<render::Renderer> GetActive() { return mActiveRenderer; }

	private:
		LoadedPlugin mPlugin;
		RendererFactory* mFactory = nullptr;
		TShared<render::Renderer> mActiveRenderer;
	};

	/** @} */
} // namespace b3d
