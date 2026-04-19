//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Utility/B3DModule.h"
#include "Serialization/B3DScriptAssemblyManager.h"

namespace b3d
{
	class ScriptObjectWrapper;

	/** @addtogroup bsfScript
	 *  @{
	 */

	/** Information required for reloading an assembly. */
	struct AssemblyRefreshInfo
	{
		AssemblyRefreshInfo() = default;

		AssemblyRefreshInfo(const char* name, const Path* path)
			: Name(name), Path(path)
		{}

		const char* Name = nullptr;
		const Path* Path = nullptr;
	};

	/**	Keeps track of all script interop objects and handles assembly refresh. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptObjectManager : public Module<ScriptObjectManager>
	{
	public:
		ScriptObjectManager() = default;
		~ScriptObjectManager();

		/**	Registers a newly created script object wrapper. */
		void RegisterScriptObjectWrapper(ScriptObjectWrapper* scriptObjectWrapper);

		/**	Unregisters a script object wrapper that is no longer valid. */
		void UnregisterScriptObjectWrapper(ScriptObjectWrapper* scriptObjectWrapper);

		/**
		 * Refreshes the list of active assemblies. Unloads all current assemblies and loads the newly provided set. This
		 * involves backup up managed object data, destroying all managed objects and restoring the objects after reload.
		 *
		 * @param[in]	assemblies	A list of assembly names and paths to load. First value represents the assembly name,
		 *							and second a path its the assembly .dll. Assemblies will be loaded in order specified.
		 */
		void RefreshAssemblies(const Vector<AssemblyRefreshInfo>& assemblies);

		/**	Called once per frame. Triggers garbage collection and queued finalizer callbacks. */
		void Update();

		/**
		 * Call this when object finalizer is triggered. At the end of the frame all objects queued with this method will
		 * have their NotifyScriptObjectDestroyed() methods triggered.
		 *
		 * @note	Thread safe.
		 */
		void NotifyObjectFinalized(ScriptObjectWrapper* instance);

		/**
		 * Triggers _onManagedInstanceDeleted deleted callbacks on all objects that were finalized this frame. This allows
		 * the native portions of those objects to properly clean up any resources. @p assemblyRefresh lets the
		 * script objects know if finalization is happening due to assembly refresh.
		 */
		void ProcessFinalizedObjects(bool assemblyRefresh = false);

		/**
		 * Triggered just before the assembly refresh starts. At this point all managed objects are still valid, but are
		 * about to be destroyed. You should release any relevant object references at this point.
		 */
		Event<void()> OnRefreshStarted;

		/**
		 * Triggered when all non-persistent objects were destroyed during assembly refresh, just before managed assemblies will be unloaded.
		 * You should clear any assembly information at this point.
		 */
		Event<void()> OnWillUnloadAssemblies;

		/**
		 * Triggered right after a domain was reloaded. This signals the outside world that they should update any kept Mono
		 * references as the old ones will no longer be valid. You should reload assembly information at this point.
		 */
		Event<void()> OnRefreshAssembliesLoaded;

		/**	Triggered after the assembly refresh ends. Persistent objects will be restored at this point. Use this to create new object handles if necessary. */
		Event<void()> OnRefreshComplete;

	private:
		/**
		 * Iterates over all active script object wrappers and finds wrappers that contain native objects referenced purely by script object (reference count == 1).
		 * Strong handles on such script objects are released so they may be freed when the last script object goes out of scope.
		 */
		void PerformGarbageCollection();

		UnorderedSet<ScriptObjectWrapper*> mScriptObjectWrappers;
		Vector<ScriptObjectWrapper*> mFinalizedScriptObjectWrappers[2];
		u32 mFinalizedQueueIdx = 0;
		Mutex mMutex;

		float mGarbageCollectionIntervalMs = 1.0f / 60.0f;
		float mLastGarbageCollectionTimeMs = 0.0f;
	};

	/** @} */
} // namespace b3d
