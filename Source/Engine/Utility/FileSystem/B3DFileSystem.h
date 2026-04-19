//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Filesystem
	 *  @{
	 */

	/** Utility class for dealing with files. */
	class B3D_EXPORT FileSystem
	{
	public:
		/**
		 * Opens a file and returns a data stream capable of reading or writing to that file.
		 *
		 * @param	fullPath	Full path to a file.
		 * @param	readOnly	(optional) If true, returned stream will only be readable.
		 */
		static SPtr<DataStream> OpenFile(const Path& fullPath, bool readOnly = true);

		/**
		 * Opens a file and returns a data stream capable of reading and writing to that file. If file doesn't exist new
		 * one will be created.
		 *
		 * @param	fullPath	Full path to a file.
		 */
		static SPtr<DataStream> CreateAndOpenFile(const Path& fullPath);

		/**
		 * Returns the size of a file in bytes.
		 *
		 * @param	fullPath	Full path to a file.
		 */
		static u64 GetFileSize(const Path& fullPath);

		/**
		 * Deletes a file or a folder at the specified path.
		 *
		 * @param	fullPath   	Full path to a file or a folder..
		 * @param	recursively	(optional) If true, folders will have their contents deleted as well. Otherwise an
		 *						exception will be thrown for non-empty folders.
		 */
		static bool Remove(const Path& fullPath, bool recursively = true);

		/**
		 * Moves a file or a folder from one to another path. This can also be used as a rename operation.
		 *
		 * @param	oldPath			 	Full path to the old file/folder.
		 * @param	newPath			 	Full path to the new file/folder.
		 * @param	overwriteExisting	(optional) If true, any existing file/folder at the new location will be
		 *								overwritten, otherwise an exception will be thrown if a file/folder already exists.
		 */
		static bool Move(const Path& oldPath, const Path& newPath, bool overwriteExisting = true);

		/**
		 * Makes a copy of a file or a folder in the specified path.
		 *
		 * @param	oldPath			 	Full path to the old file/folder.
		 * @param	newPath			 	Full path to the new file/folder.
		 * @param	overwriteExisting	(optional) If true, any existing file/folder at the new location will be
		 *								overwritten, otherwise an exception will be thrown if a file/folder already exists.
		 */
		static bool Copy(const Path& oldPath, const Path& newPath, bool overwriteExisting = true);

		/**
		 * Creates a folder at the specified path.
		 *
		 * @param	fullPath	Full path to a full folder to create.
		 */
		static bool CreateFolder(const Path& fullPath);

		/**
		 * Returns true if a file or a folder exists at the specified path.
		 *
		 * @param	fullPath	Full path to a file or folder.
		 */
		static bool Exists(const Path& fullPath);

		/**
		 * Returns true if a file exists at the specified path.
		 *
		 * @param	fullPath	Full path to a file or folder.
		 */
		static bool IsFile(const Path& fullPath);

		/**
		 * Returns true if a folder exists at the specified path.
		 *
		 * @param	fullPath	Full path to a file or folder.
		 */
		static bool IsFolder(const Path& fullPath);

		/**
		 * Returns all files or folders located in the specified folder.
		 *
		 * @param	dirPath			Full path to the folder to retrieve children files/folders from.
		 * @param	outFiles	   	Full paths to all files located directly in specified folder.
		 * @param	outDirectories	Full paths to all folders located directly in specified folder.
		 */
		static void GetChildren(const Path& dirPath, Vector<Path>& outFiles, Vector<Path>& outDirectories);

		/**
		 * Iterates over all files and directories in the specified folder and calls the provided callback when a
		 * file/folder is iterated over.
		 *
		 * @param	dirPath			Directory over which to iterate
		 * @param	fileCallback	Callback to call whenever a file is found. If callback returns false iteration stops. Can be null.
		 * @param	dirCallback		Callback to call whenever a directory is found. If callback returns false iteration stops. Can be null.
		 * @param	recursive		If false then only the direct children of the provided folder will be iterated over,
		 *							and if true then child directories will be recursively visited as well.
		 * @return					True if iteration finished iterating over all files/folders, or false if it was
		 *							interrupted by a callback returning false.
		 */
		static bool Iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback, std::function<bool(const Path&)> dirCallback = nullptr, bool recursive = true);

		/**
		 * Returns the last modified time of a file or a folder at the specified path.
		 *
		 * @param	fullPath	Full path to a file or a folder.
		 */
		static std::time_t GetLastModifiedTime(const Path& fullPath);

		/** Returns the path to the directory containing the current executable. */
		static Path GetExecutableFolderPath();

		/** Returns the path to the current working directory. */
		static Path GetWorkingDirectoryPath();

		/** Returns the path to a directory where temporary files may be stored. */
		static Path GetTemporaryFolderPath();

		/** Returns a path to a file in the temporary directory. The path is guaranteed not to exist currently in the directory. */
		static Path GetUniqueTemporaryFilePath();

		/** Returns the application data folder for the current user. */
		static Path GetApplicationDataFolder();

	private:
		/** Copy a single file. Internal function used by copy(). */
		static bool CopyFile(const Path& oldPath, const Path& newPath);

		/** Remove a single file. Internal function used by remove(). */
		static bool RemoveFile(const Path& path);

		/** Move a single file. Internal function used by move(). */
		static bool MoveFile(const Path& oldPath, const Path& newPath);
	};

	/**
	 * Locks access to files on the same drive, allowing only one file to be read at a time, per drive. This prevents
	 * multiple threads accessing multiple files on the same drive at once, ruining performance on mechanical drives.
	 */
	class B3D_EXPORT FileScheduler final
	{
	public:
		/**
		 * Locks access and doesn't allow other threads to get past this point until access is unlocked. Any scheduled
		 * file access should happen past this point.
		 */
		static void Lock(const Path& path)
		{
			// Note: File path should be analyzed and determined on which drive does the path belong to. Locks can then
			// be issued on a per-drive basis, instead of having one global lock. This would allow multiple files to be
			// accessed at the same time, as long as they're on different drives.
			mMutex.lock();
		}

		/**
		 * Unlocks access and allows another thread to lock file access. Must be provided with the same file path as
		 * lock().
		 */
		static void Unlock(const Path& path)
		{
			mMutex.unlock();
		}

		/**
		 * Returns a lock object that immediately locks access (same as lock()), and then calls unlock() when it goes
		 * out of scope.
		 */
		static ::b3d::Lock GetLock(const Path& path)
		{
			return b3d::Lock(mMutex);
		}

	private:
		static Mutex mMutex;
	};

	/** @} */
} // namespace b3d
