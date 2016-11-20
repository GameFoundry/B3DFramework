//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsCoreThreadAccessor.h"
#include "BsRenderWindow.h"
#include "BsEvent.h"

namespace bs
{
	/** @addtogroup Application-Core
	 *  @{
	 */

	/**	Structure containing parameters for starting the application. */
	struct START_UP_DESC
	{
		String renderAPI; /**< Name of the render system plugin to use. */
		String renderer; /**< Name of the renderer plugin to use. */
		String physics; /**< Name of physics plugin to use. */
		String audio; /**< Name of the audio plugin to use. */
		String input; /**< Name of the input plugin to use. */

		RENDER_WINDOW_DESC primaryWindowDesc; /**< Describes the window to create during start-up. */

		Vector<String> importers; /**< A list of importer plugins to load. */

		/** Optional callback function to be called every frame while the application is running. */
		std::function<void()> updateCallback; 
	};

	/**
	 * Represents the primary entry point for the core systems. Handles start-up, shutdown, primary loop and allows you to
	 * load and unload plugins.
	 *
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT CoreApplication : public Module<CoreApplication>
	{
		public:
			CoreApplication(START_UP_DESC desc);
			virtual ~CoreApplication();

			/**
			 * Executes the main loop. This will update your components and modules, queue objects for rendering and run 
			 * the simulation. Usually called immediately after startUp().
			 * 			
			 * This will run infinitely until stopMainLoop is called (usually from another thread or internally).
			 */
			void runMainLoop();

			/**	Stops the (infinite) main loop from running. The loop will complete its current cycle before stopping. */
			void stopMainLoop();

			/** Changes the maximum FPS the application is allowed to run in. Zero means unlimited. */
			void setFPSLimit(UINT32 limit);

			/**
			 * Issues a request for the application to close. Application may choose to ignore the request depending on the
			 * circumstances and the implementation.
			 */
			virtual void quitRequested();

			/**	Returns the main window that was created on application start-up. */
			SPtr<RenderWindow> getPrimaryWindow() const { return mPrimaryWindow; }

			/**
			 * Returns the id of the simulation thread.
			 *
			 * @note	Thread safe.
			 */
			ThreadId getSimThreadId() { return mSimThreadId; }

			/**	Returns true if the application is running in an editor, false if standalone. */
			virtual bool isEditor() const { return false; }

			/**
			 * Loads a plugin.
			 *
			 * @param[in]	pluginName	Name of the plugin to load, without extension.
			 * @param[out]	library		Specify as not null to receive a reference to the loaded library.
			 * @param[in]	passThrough	Optional parameter that will be passed to the loadPlugin function.
			 * @return					Value returned from the plugin start-up method.
			 */
			void* loadPlugin(const String& pluginName, DynLib** library = nullptr, void* passThrough = nullptr);

			/**	Unloads a previously loaded plugin. */
			void unloadPlugin(DynLib* library);

	protected:
		/** @copydoc Module::onStartUp */
		void onStartUp() override;

		/**	Called for each iteration of the main loop. Called before any game objects or plugins are updated. */
		virtual void preUpdate();

		/**	Called for each iteration of the main loop. Called after all game objects and plugins are updated. */
		virtual void postUpdate();

		/**	Initializes the renderer specified during construction. Called during initialization. */
		virtual void startUpRenderer();

		/**	Returns a handler that is used for resolving shader include file paths. */
		virtual SPtr<IShaderIncludeHandler> getShaderIncludeHandler() const;

	private:
		/**	Called when the frame finishes rendering. */
		void frameRenderingFinishedCallback();

		/**	Called by the core thread to begin profiling. */
		void beginCoreProfiling();

		/**	Called by the core thread to end profiling. */
		void endCoreProfiling();

	protected:
		typedef void(*UpdatePluginFunc)();

		SPtr<RenderWindow> mPrimaryWindow;
		START_UP_DESC mStartUpDesc;

		UINT64 mFrameStep; // Microseconds
		UINT64 mLastFrameTime; // Microseconds

		DynLib* mRendererPlugin;

		Map<DynLib*, UpdatePluginFunc> mPluginUpdateFunctions;

		bool mIsFrameRenderingFinished;
		Mutex mFrameRenderingFinishedMutex;
		Signal mFrameRenderingFinishedCondition;
		ThreadId mSimThreadId;

		volatile bool mRunMainLoop;
	};

	/**	Provides easy access to CoreApplication. */
	BS_CORE_EXPORT CoreApplication& gCoreApplication();

	/** @} */
}