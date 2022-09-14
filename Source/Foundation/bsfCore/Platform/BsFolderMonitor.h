//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Types of notifications we would like to receive when we start a FolderMonitor on a certain folder. */
	enum class FolderChangeBit
	{
		FileName 	= 1 << 0, /**< Called when file is created, moved or removed. */
		DirName 	= 1 << 1, /**< Called when directory is created, moved or removed. */
		FileWrite 	= 1 << 2, /**< Called when file is written to. */
	};

	typedef Flags<FolderChangeBit> FolderChangeBits;
	BS_FLAGS_OPERATORS(FolderChangeBit)

	/**
	 * Allows monitoring a file system folder for changes. Depending on the flags set this monitor can notify you when file
	 * is changed/moved/renamed and similar.
	 */
	class BS_CORE_EXPORT FolderMonitor
	{
		class FileNotifyInfo;
	public:
		struct Pimpl;
		struct FolderWatchInfo;

		FolderMonitor();
		~FolderMonitor();

		/**
		 * Starts monitoring a folder at the specified path.
		 *
		 * @param[in]	folderPath		Absolute path to the folder you want to monitor.
		 * @param[in]	subdirectories	If true, provided folder and all of its subdirectories will be monitored for
		 *								changes. Otherwise only the provided folder will be monitored.
		 * @param[in]	changeFilter	A set of flags you may OR together. Different notification events will trigger
		 *								depending on which flags you set.
		 */
		void startMonitor(const Path& folderPath, bool subdirectories, FolderChangeBits changeFilter);

		/** Stops monitoring the folder at the specified path. */
		void stopMonitor(const Path& folderPath);

		/**	Stops monitoring all folders that are currently being monitored. */
		void stopMonitorAll();

		/** Triggers callbacks depending on events that ocurred. Expected to be called once per frame. */
		void UpdateInternal();

		/** Triggers when a file in the monitored folder is modified. Provides absolute path to the file. */
		Event<void(const Path&)> onModified;

		/**	Triggers when a file/folder is added in the monitored folder. Provides absolute path to the file/folder. */
		Event<void(const Path&)> onAdded;

		/**	Triggers when a file/folder is removed from the monitored folder. Provides absolute path to the file/folder. */
		Event<void(const Path&)> onRemoved;

		/**	Triggers when a file/folder is renamed in the monitored folder. Provides absolute path with old and new names. */
		Event<void(const Path&, const Path&)> onRenamed;

		/**
		 * @name Internal
		 * @{
		 */

		/** Returns private data, for use by internal helper classes and methods. */
		Pimpl* GetPrivateDataInternal() const { return m; }

		/** @} */
	private:
		/**	Worker method that monitors the IO ports for any modification notifications. */
		void workerThreadMain();

		/**	Called by the worker thread whenever a modification notification is received. */
		void handleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo);

		Pimpl* m;
	};

	/** @} */
}
