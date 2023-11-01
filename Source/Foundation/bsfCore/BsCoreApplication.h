//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Threading/BsSignalEvent.h"
#include "Utility/BsEvent.h"
#include "Utility/BsPersistentCache.h"

namespace bs
{
	class Scheduler;
	class GpuDevice;
/** @addtogroup Application-Core
	 *  @{
	 */

	/**	Structure containing parameters for starting the application. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Application), ExportAsStruct(true), API(Framework)) START_UP_DESC
	{
		String RenderApi; /**< Name of the render system plugin to use. */
		String Renderer; /**< Name of the renderer plugin to use. */
		String Physics; /**< Name of physics plugin to use. */
		String Audio; /**< Name of the audio plugin to use. */
		String Input; /**< Name of the input plugin to use. */

		/**
		 * True if physics cooking library should be loaded. Cooking is useful for creating collision meshes during
		 * development type, but might be unnecessary in the final application. When turned off you can save on space by
		 * not shipping the cooking library.
		 */
		bool PhysicsCooking = true;

		/**
		 * True if animation should be evaluated at the same time while rendering is happening. This introduces a one
		 * frame delay to all animations but can result in better performance. If false the animation will be forced
		 * to finish evaluating before rendering starts, ensuring up-to-date frame but potentially blocking the rendering
		 * thread from moving forward until the animation finishes.
		 */
		bool AsyncAnimation = true;

		RENDER_WINDOW_DESC PrimaryWindowDesc; /**< Describes the window to create during start-up. */

		Vector<String> Importers; /**< A list of importer plugins to load. */

		/**
		 *If specified Log will call this function whenever a new log message is added. If this returns true then
		 * the default action of the log will be skipped.
		 */
		B3D_SCRIPT_EXPORT(Exclude(true))
		std::function<bool(const String& message, LogVerbosity verbosity, const char* categoryName)> LogCallback;

		/** Crash handling customization */
		B3D_SCRIPT_EXPORT(Exclude(true))
		CrashHandlerSettings CrashHandling;
	};

	/**
	 * Represents the primary entry point for the core systems. Handles start-up, shutdown, primary loop and allows you to
	 * load and unload plugins.
	 *
	 * @note	Main thread only.
	 */
	class B3D_CORE_EXPORT CoreApplication : public Module<CoreApplication>
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
		void RunMainLoop();

		/**	Stops the (infinite) main loop from running. The loop will complete its current cycle before stopping. */
		void StopMainLoop();

		bool IsMainLoopRunning() const { return mRunMainLoop; }

		/** Changes the maximum FPS the application is allowed to run in. Zero means unlimited. */
		void SetFpsLimit(u32 limit);

		/**
		 * Issues a request for the application to close. Application may choose to ignore the request depending on the
		 * circumstances and the implementation.
		 */
		virtual void QuitRequested();

		/** Call before the first time runMainLoopFrame is called */
		virtual void BeginMainLoop();

		/** Call after the last time runMainLoopFrame is called */
		virtual void EndMainLoop();

		/** Alternative to runMainLoop, processes one step at a time */
		void RunMainLoopFrame();

		/** Waits until previous frame is complete */
		void WaitUntilFrameFinished();

		/**	Returns the main window that was created on application start-up. */
		SPtr<RenderWindow> GetPrimaryWindow() const { return mPrimaryWindow; }

		/** Returns the primary GPU on which to perform rendering. */
		SPtr<GpuDevice> GetPrimaryGpuDevice() const { return mPrimaryGpu; }

		/**
		 * Returns a unique identifier of the main thread.
		 *
		 * @note	Thread safe.
		 */
		ThreadId GetMainThreadId() const { return mSimThreadId; }

		/**	Returns true if the application is running in an editor, false if standalone. */
		virtual bool IsEditor() const { return false; }

		/** Returns cache for storing application-wide data that persists application reset. */
		PersistentCache& GetApplicationCache() { return *mApplicationCache; }

		/** Returns the scheduler on which you can queue tasks on for execution on worker threads. */
		Scheduler& GetTaskScheduler() const { return *mTaskScheduler; }

		/** Returns the scheduler responsible for running tasks on the main thread. */
		Scheduler& GetMainThreadScheduler() { return mMainThreadScheduler; }

		/**
		 * Loads a plugin.
		 *
		 * @param[in]	pluginName	Name of the plugin to load, without extension.
		 * @param[out]	outLibrary	Specify as not null to receive a reference to the loaded library.
		 * @param[in]	passThrough	Optional parameter that will be passed to the loadPlugin function.
		 * @return					Value returned from the plugin start-up method.
		 */
		void* LoadPlugin(const String& pluginName, DynamicLibrary** outLibrary = nullptr, void* passThrough = nullptr);

		/**	Unloads a previously loaded plugin. */
		void UnloadPlugin(const String& pluginName);

		/** Unloads all loaded plugins. */
		void UnloadAllPlugins();

	protected:
		void OnStartUp() override;

		/**	Called for each iteration of the main loop. Called before any game objects or plugins are updated. */
		virtual void PreUpdate();

		/**	Called for each iteration of the main loop. Called after all game objects and plugins are updated. */
		virtual void PostUpdate();

		/** Called during the fixed update of the main loop. Called after preUpdate and before postUpdate. */
		virtual void FixedUpdate();

		/**	Initializes the renderer specified during construction. Called during initialization. */
		virtual void StartUpRenderer();

		/**	Returns a handler that is used for resolving shader include file paths. */
		virtual SPtr<IShaderIncludeHandler> GetShaderIncludeHandler() const;

		/**	Called when the frame finishes rendering. */
		void FrameRenderingFinishedCallback();

		/**	Called by the render thread to begin profiling. */
		void BeginRenderThreadProfiling();

		/**	Called by the render thread to end profiling. */
		void EndRenderThreadProfiling();

	protected:
		typedef void (*UpdatePluginFunctionPointer)();
		typedef void (*UnloadPluginFunctionPointer)();

		/** Information about a loaded plugin. */
		struct LoadedPlugin
		{
			DynamicLibrary* Library = nullptr;
			UpdatePluginFunctionPointer UpdateCallback = nullptr;
			UnloadPluginFunctionPointer UnloadCallback = nullptr;
		};

		SPtr<RenderWindow> mPrimaryWindow;
		SPtr<GpuDevice> mPrimaryGpu;
		Scheduler mMainThreadScheduler;
		SPtr<Scheduler> mTaskScheduler;
		START_UP_DESC mStartUpDesc;

		// Frame limiting
		u64 mFrameStep = 16666; // 60 times a second in microseconds
		u64 mLastFrameTime = 0; // Microseconds

		SPtr<PersistentCache> mApplicationCache;
		UnorderedMap<String, LoadedPlugin> mLoadedPlugins;

		SignalEvent mFrameRenderingFinishedSignal;
		ThreadId mSimThreadId;

		volatile bool mRunMainLoop;
	};

	/**	Provides easy access to CoreApplication. */
	B3D_CORE_EXPORT CoreApplication& GetCoreApplication();

	/** Returns false if currently not at the main thread, and triggers an ensure. */
	B3D_CORE_EXPORT inline bool EnsureMainThread()
	{
		return B3D_ENSURE(B3D_CURRENT_THREAD_ID == CoreApplication::Instance().GetMainThreadId());
	}

	/** @} */
} // namespace bs
