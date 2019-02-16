//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "BsCoreApplication.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Application-Engine
	 *  @{
	 */

	/**	Primary entry point for the framework. Handles startup and shutdown. */
	class BS_EXPORT Application : public CoreApplication
	{
	private:
		/** 
		 * Builds the start-up descriptor structure, filling out the provided parameters and using the default values
		 * for the rest. 
		 */
		static START_UP_DESC buildStartUpDesc(VideoMode videoMode, const String& title, bool fullscreen);
	public:
		Application(const START_UP_DESC& desc);
		virtual ~Application();

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template<class T = Application>
		static void startUp(VideoMode videoMode, const String& title, bool fullscreen)
		{
			START_UP_DESC desc = buildStartUpDesc(videoMode, title, fullscreen);
			CoreApplication::startUp<T>(desc);
		}

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template<class T = Application>
		static void startUp(const START_UP_DESC& desc)
		{
			CoreApplication::startUp<T>(desc);
		}

		/** 
		 * Shows the profiler overlay. 
		 *
		 * @param[in]	type	Type of information to display on the overlay.
		 * @param[in]	camera	Camera to show the overlay on. If none the overlay will be shown on the main camera.
		 */
		void showProfilerOverlay(ProfilerOverlayType type, const SPtr<Camera>& camera = nullptr);

		/** Hides the profiler overlay. */
		void hideProfilerOverlay();

	protected:
		/** @copydoc Module::onStartUp */
		void onStartUp() override;

		/** @copydoc Module::onShutDown */
		void onShutDown() override;

		/** @copydoc CoreApplication::preUpdate */
		void preUpdate() override;

		/** @copydoc CoreApplication::postUpdate */
		void postUpdate() override;

		/** @copydoc CoreApplication::startUpRenderer */
		void startUpRenderer() override;

		/** Initializes the script manager. */
		virtual void startUpScriptManager();

		/** @copydoc CoreApplication::getShaderIncludeHandler */
		SPtr<IShaderIncludeHandler> getShaderIncludeHandler() const override;

		SPtr<ProfilerOverlay> mProfilerOverlay;
	};

	/** Easy way to access Application. */
	BS_EXPORT Application& gApplication();

	/** @} */
}