//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DCoreApplication.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup Application-Engine
	 *  @{
	 */

	/**	Primary entry point for the framework. Handles startup and shutdown. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() Application : public CoreApplication
	{
	public:
		Application(const ApplicationCreateInformation& desc);
		virtual ~Application();

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template <class T = Application>
		static void StartUp(VideoMode videoMode, const String& title, bool fullscreen)
		{
			ApplicationCreateInformation desc = BuildCreateInformation(videoMode, title, fullscreen);
			CoreApplication::StartUp<T>(desc);
		}

		/** Starts the framework. If using a custom Application system, provide it as a template parameter. */
		template <class T = Application>
		static void StartUp(const ApplicationCreateInformation& desc)
		{
			CoreApplication::StartUp<T>(desc);
		}

		/**
		 * Shows the profiler overlay.
		 *
		 * @param[in]	type	Type of information to display on the overlay.
		 * @param[in]	camera	Camera to show the overlay on. If none the overlay will be shown on the main camera.
		 */
		void ShowProfilerOverlay(ProfilerOverlayType type, const HCamera& camera = nullptr);

		/** Hides the profiler overlay. */
		void HideProfilerOverlay();

		/**
		 * Builds the start-up descriptor structure, filling out the provided parameters and using the default values
		 * for the rest.
		 */
		static ApplicationCreateInformation BuildCreateInformation(VideoMode videoMode, const String& title, bool fullscreen);

	protected:
		void OnStartUp() override;
		void OnShutDown() override;

		void PreUpdate() override;
		void PostUpdate() override;

		void StartUpRenderer() override;

		/** Initializes the script manager. */
		virtual void StartUpScriptManager();

		/** Calls per-frame update on the script manager. */
		virtual void UpdateScriptManager();

		SPtr<IShaderIncludeHandler> GetShaderIncludeHandler() const override;

		SPtr<ProfilerOverlay> mProfilerOverlay;
	};

	/** Easy way to access Application. */
	B3D_EXPORT Application& GetApplication();

	/** @} */
} // namespace b3d
