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
		static START_UP_DESC BuildStartUpDesc(VideoMode videoMode, const String& title, bool fullscreen);
	public:
		Application(const START_UP_DESC& desc);
		virtual ~Application();

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template<class T = Application>
		static void StartUp(VideoMode videoMode, const String& title, bool fullscreen)
		{
			START_UP_DESC desc = BuildStartUpDesc(videoMode, title, fullscreen);
			CoreApplication::StartUp<T>(desc);
		}

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template<class T = Application>
		static void StartUp(const START_UP_DESC& desc)
		{
			CoreApplication::StartUp<T>(desc);
		}

		/**
		 * Shows the profiler overlay.
		 *
		 * @param[in]	type	Type of information to display on the overlay.
		 * @param[in]	camera	Camera to show the overlay on. If none the overlay will be shown on the main camera.
		 */
		void ShowProfilerOverlay(ProfilerOverlayType type, const SPtr<Camera>& camera = nullptr);

		/** Hides the profiler overlay. */
		void HideProfilerOverlay();

	protected:
		/** @copydoc Module::onStartUp */
		void OnStartUp() override;

		/** @copydoc Module::onShutDown */
		void OnShutDown() override;

		/** @copydoc CoreApplication::preUpdate */
		void PreUpdate() override;

		/** @copydoc CoreApplication::postUpdate */
		void PostUpdate() override;

		/** @copydoc CoreApplication::StartUpRenderer */
		void StartUpRenderer() override;

		/** Initializes the script manager. */
		virtual void StartUpScriptManager();

		/** Calls per-frame update on the script manager. */
		virtual void UpdateScriptManager();

		/** @copydoc CoreApplication::getShaderIncludeHandler */
		SPtr<IShaderIncludeHandler> GetShaderIncludeHandler() const override;

		SPtr<ProfilerOverlay> mProfilerOverlay;
	};

	/** Easy way to access Application. */
	BS_EXPORT Application& gApplication();

	/** @} */
}
