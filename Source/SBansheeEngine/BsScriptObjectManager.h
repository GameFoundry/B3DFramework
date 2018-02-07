//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup SBansheeEngine
	 *  @{
	 */

	/**	Keeps track of all script interop objects and handles assembly refresh. */
	class BS_SCR_BE_EXPORT ScriptObjectManager : public Module <ScriptObjectManager>
	{
	public:
		ScriptObjectManager();
		~ScriptObjectManager();

		/**	Registers a newly created script interop object. */
		void registerScriptObject(ScriptObjectBase* instance);

		/**	Unregisters a script interop object that is no longer valid. */
		void unregisterScriptObject(ScriptObjectBase* instance);

		/**
		 * Refreshes the list of active assemblies. Unloads all current assemblies and loads the newly provided set. This
		 * involves backup up managed object data, destroying all managed objects and restoring the objects after reload.
		 *
		 * @param[in]	assemblies	A list of assembly names and paths to load. First value represents the assembly name,
		 *							and second a path its the assembly .dll. Assemblies will be loaded in order specified.
		 */
		void refreshAssemblies(const Vector<std::pair<String, Path>>& assemblies);

		/**	Called once per frame. Triggers queued finalizer callbacks. */
		void update();

		/**
		 * Call this when object finalizer is triggered. At the end of the frame all objects queued with this method will
		 * have their _onManagedInstanceDeleted methods triggered.
		 *
		 * @note	Thread safe.
		 */
		void notifyObjectFinalized(ScriptObjectBase* instance);

		/**
		 * Triggers _onManagedInstanceDeleted deleted callbacks on all objects that were finalized this frame. This allows
		 * the native portions of those objects to properly clean up any resources. @p assemblyRefresh lets the
		 * script objects know if finalization is happening due to assembly refresh.
		 */
		void processFinalizedObjects(bool assemblyRefresh = false);

		/**
		 * Triggered right after a domain was reloaded. This signals the outside world that they should update any kept Mono
		 * references as the old ones will no longer be valid.
		 */
		Event<void()> onRefreshDomainLoaded;

		/**
		 * Triggered just before the assembly refresh starts. At this point all managed objects are still valid, but are
		 * about to be destroyed.
		 */
		Event<void()> onRefreshStarted;

		/**	Triggered after the assembly refresh ends. New assemblies should be loaded at this point. */
		Event<void()> onRefreshComplete;
	private:
		Set<ScriptObjectBase*> mScriptObjects;

		Vector<ScriptObjectBase*> mFinalizedObjects[2];
		UINT32 mFinalizedQueueIdx;
		Mutex mMutex;
	};

	/** @} */
}