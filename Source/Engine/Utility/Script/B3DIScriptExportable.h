//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DAny.h"

namespace b3d
{
	/** @addtogroup ScriptExport
	 *  @{
	 */

	class IScriptObjectWrapper;

	/** Structure to persist object data during script reload. Objects will store their data before reload happens, and then restore the data after it happens. */
	struct ScriptObjectReloadPersistentData
	{
		ScriptObjectReloadPersistentData() {}

		explicit ScriptObjectReloadPersistentData(const Any& data)
			: Data(data)
		{}

		Any Data; // TODO - Don't use Any
	};

	/**
	 * Interface to be implemented by types that are exported to scripting. Such types should also be decorated with B3D_SCRIPT_EXPORT() macro, along
	 * with any fields or methods that should be exported.
	 */
	class B3D_EXPORT IScriptExportable
	{
	public:
		IScriptExportable() = default;

		IScriptExportable(const IScriptExportable& other);
		IScriptExportable(IScriptExportable&& other);

		virtual ~IScriptExportable();

		IScriptExportable& operator=(const IScriptExportable& other);
		IScriptExportable& operator=(IScriptExportable&& other);

		/**
		 * Returns the script object wrapper associated with this object. This will be null if the object was never passed to script world, or if was passed
		 * but has gone out of scope.
		 */
		IScriptObjectWrapper* GetScriptObjectWrapper() const { return mScriptObjectWrapper; }

		/**
		 * @name Script reload
		 * @{
		 */

		/**
		 * If true, the object will be given an opportunity to back up its data before a script reload operation, and its script object will be automatically
		 * recreated once script reload ends, and given an opportunity to restore the backed up data. If false, the script object and its wrapper will
		 * be destroyed on script reload.
		 */
		virtual bool ShouldPersistScriptReload() const { return false; }

		/** Called on all script objects before script object reload happens. */
		virtual void NotifyScriptWillReload() { }

		/*
		 * Called on all script objects when script reload is about to happen, after NotifyScriptWillReload(). Allows the script object to back up its current state
		 * so it may be restored after reload completes. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual TOptional<ScriptObjectReloadPersistentData> BackupDataBeforeScriptReload() { return {}; }

		/**
		 * Called on all script objects after script assemblies have been reloaded. This needs to recreate the internal script object using the new assemblies,
		 * as the old one will have been destroyed during the reload. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 * Returns true if the script object was created, otherwise the responsibility will fall to the script object wrapper.
		 */
		virtual bool RecreateScriptObjectAfterScriptReload() { return false; }

		/**
		 * Called on all script objects after script objects have been created in RecreateScriptObjectAfterScriptReload(). Allows you to restore data backed up
		 * in BackupDataBeforeScriptReload() call to the newly created script object. Only relevant for script objects that persist script reload (i.e. ShouldPersistScriptReload() returns true).
		 */
		virtual void RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data) { }

		/**
		 * Called on all script object wrappers as the final step in script reload, after RestoreDataAfterScriptReload(). Allows you to perform actions that require
		 * the entire scripting world to be fully recreated.
		 */
		virtual void NotifyScriptReloadFinished() { }

		/** @} */

		/**
		 * @name Internal
		 * @{
		 */

		/** Clears the currently associated script object wrapper. Generally called before the script object is destroyed. */
		void ClearAssociatedScriptObjectWrapper() { mScriptObjectWrapper = nullptr; }

		/** @} */

	private:
		friend class IScriptObjectWrapper;

		/** Notifies the object that a script object wrapper has been created for it, allowing a script object to access the native object through it. */
		void AssociateWithScriptObjectWrapper(IScriptObjectWrapper* wrapper);

		IScriptObjectWrapper* mScriptObjectWrapper = nullptr;
	};

	/** @} */
} // namespace b3d
