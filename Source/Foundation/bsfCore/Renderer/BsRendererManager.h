//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
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
	class BS_CORE_EXPORT RendererManager : public Module<RendererManager>
	{
	public:
		~RendererManager();

		/**
		 * Attempts to find a renderer with the specified name and makes it active. Exception is thrown if renderer with
		 * the specified name doesn't exist. You must call initialize() after setting the active renderer to properly
		 * activate it.
		 */
		void setActive(const String& name);

		/** Initializes the currently active renderer, making it ready to render. */
		void initialize();

		/**	Returns the currently active renderer. Null if no renderer is active. */
		SPtr<ct::Renderer> getActive() { return mActiveRenderer; }

		/**
		 * Registers a new renderer factory. Any renderer you try to make active with setActive() you will need to have
		 * previously registered here.
		 */
		void RegisterFactoryInternal(SPtr<RendererFactory> factory);
	private:
		Vector<SPtr<RendererFactory>> mAvailableFactories;

		SPtr<ct::Renderer> mActiveRenderer;
	};

	/** @} */
}

