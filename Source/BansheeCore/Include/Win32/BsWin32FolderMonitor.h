//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/** Types of notifications we would like to receive when we start a FolderMonitor on a certain folder. */
	enum class FolderChange
	{
		FileName = 0x0001, /**< Called when filename changes. */
		DirName = 0x0002, /**< Called when directory name changes. */
		Attributes = 0x0004, /**< Called when attributes changes. */
		Size = 0x0008, /**< Called when file size changes. */
		LastWrite = 0x0010, /**< Called when file is written to. */
		LastAccess = 0x0020, /**< Called when file is accessed. */
		Creation = 0x0040, /**< Called when file is created. */
		Security = 0x0080 /**< Called when file security descriptor changes. */
	};

	/**
	 * Allows monitoring a file system folder for changes. Depending on the flags set this monitor can notify you when file
	 * is changed/moved/renamed and similar.
	 */
	class BS_CORE_EXPORT FolderMonitor
	{
		struct Pimpl;
		class FileNotifyInfo;
		struct FolderWatchInfo;
	public:
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
		void startMonitor(const Path& folderPath, bool subdirectories, FolderChange changeFilter);

		/** Stops monitoring the folder at the specified path. */
		void stopMonitor(const Path& folderPath);

		/**	Stops monitoring all folders that are currently being monitored. */
		void stopMonitorAll();

		/** Callbacks will only get fired after update is called. */
		void _update();

		/** Triggers when a file in the monitored folder is modified. Provides absolute path to the file. */
		Event<void(const Path&)> onModified;

		/**	Triggers when a file/folder is added in the monitored folder. Provides absolute path to the file/folder. */
		Event<void(const Path&)> onAdded;

		/**	Triggers when a file/folder is removed from the monitored folder. Provides absolute path to the file/folder. */
		Event<void(const Path&)> onRemoved;

		/**	Triggers when a file/folder is renamed in the monitored folder. Provides absolute path with old and new names. */
		Event<void(const Path&, const Path&)> onRenamed;

	private:
		/**	Worker method that monitors the IO ports for any modification notifications. */
		void workerThreadMain();

		/**	Called by the worker thread whenever a modification notification is received. */
		void handleNotifications(FileNotifyInfo& notifyInfo, FolderWatchInfo& watchInfo);

		Pimpl* mPimpl;
	};

	/** @} */
}