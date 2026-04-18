//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Platform
	 *  @{
	 */

	/** Types of notifications we would like to receive when we start a FolderMonitor on a certain folder. */
	enum class B3D_SCRIPT_EXPORT() FolderChangeFlag
	{
		FileAddedOrRemoved = 1 << 0, /**< Called when file is created, moved or removed. */
		DirectoryAddedOrRemoved = 1 << 1, /**< Called when directory is created, moved or removed. */
		FileWritten = 1 << 2, /**< Called when file is written to. */
	};

	typedef Flags<FolderChangeFlag> FolderChangeFlags;
	B3D_FLAGS_OPERATORS(FolderChangeFlag)

	/**
	 * Allows monitoring a file system folder for changes. Depending on the flags set this monitor can notify you when file
	 * is changed/moved/renamed and similar.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(API(Editor)) FolderMonitor : public IScriptExportable
	{
		class FileNotifyInfo;

	public:
		struct Pimpl;

		/**
		 * Starts monitoring a folder at the specified path.
		 *
		 * @param	folderPath				Absolute path to the folder you want to monitor.
		 * @param	monitorSubdirectories	If true, provided folder and all of its subdirectories will be monitored for changes. Otherwise only the provided folder will be monitored.
		 * @param	changeFilter			A set of flags you may OR together. Different notification events will trigger depending on which flags you set.
		 */
		B3D_SCRIPT_EXPORT()
		FolderMonitor(const Path& folderPath, bool monitorSubdirectories, FolderChangeFlags changeFilter);

		/** Stops monitor the folder. */
		~FolderMonitor();

		/** Triggers callbacks depending on events that ocurred. Expected to be called once per frame. */
		void Update();

		/** Triggers when a file in the monitored folder is modified. Provides absolute path to the file. */
		B3D_SCRIPT_EXPORT()
		Event<void(const Path&)> OnModified;

		/**	Triggers when a file/folder is added in the monitored folder. Provides absolute path to the file/folder. */
		B3D_SCRIPT_EXPORT()
		Event<void(const Path&)> OnAdded;

		/**	Triggers when a file/folder is removed from the monitored folder. Provides absolute path to the file/folder. */
		B3D_SCRIPT_EXPORT()
		Event<void(const Path&)> OnRemoved;

		/**	Triggers when a file/folder is renamed in the monitored folder. Provides absolute path with old and new names. */
		B3D_SCRIPT_EXPORT()
		Event<void(const Path&, const Path&)> OnRenamed;

		/**
		 * @name Internal
		 * @{
		 */

		/** Returns private data, for use by internal helper classes and methods. */
		Pimpl* GetPrivateDataInternal() const { return m; }

		/** @} */
	private:
		/**	Worker method that monitors the IO ports for any modification notifications. */
		void WorkerThreadMain();

		/**	Called by the worker thread whenever a modification notification is received. */
		void HandleNotifications(FileNotifyInfo& notifyInfo);

		Pimpl* m;
	};

	/** @} */

	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/**	Manages all active managed folder monitor objects. */
	class B3D_EXPORT FolderMonitorManager : public Module<FolderMonitorManager>
	{
	public:
		/**	Triggers updates on all active folder monitor objects. Should be called once per frame. */
		void Update();

	private:
		friend class FolderMonitor;

		/**	Registers a new folder monitor. */
		void RegisterMonitor(FolderMonitor* monitor);

		/**	Unregisters a folder monitor. */
		void UnregisterMonitor(FolderMonitor* monitor);

		UnorderedSet<FolderMonitor*> mMonitors;
	};

	/** @} */
} // namespace b3d
