//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsFileSystem.h"
#include "Debug/BsDebug.h"

namespace bs
{
	void FileSystem::Copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		Stack<std::tuple<Path, Path>> todo;
		todo.Push(std::make_tuple(oldPath, newPath));

		while (!todo.Empty())
		{
			auto current = todo.Top();
			todo.Pop();

			Path sourcePath = std::get<0>(current);
			if (!FileSystem::exists(sourcePath))
				continue;

			bool srcIsFile = FileSystem::isFile(sourcePath);
			Path destinationPath = std::get<1>(current);
			bool destExists = FileSystem::exists(destinationPath);

			if (destExists)
			{
				if (FileSystem::isFile(destinationPath))
				{
					if (overwriteExisting)
						FileSystem::remove(destinationPath);
					else
					{
						BS_LOG(Warning, FileSystem, "Copy operation failed because another file already exists at the new "
							"path: \"{0}\"", destinationPath);
						return;
					}
				}
			}

			if (srcIsFile)
			{
				FileSystem::copyFile(sourcePath, destinationPath);
			}
			else
			{
				if (!destExists)
					FileSystem::createDir(destinationPath);

				Vector<Path> files;
				Vector<Path> directories;
				getChildren(destinationPath, files, directories);

				for (auto& file : files)
				{
					Path fileDestPath = destinationPath;
					fileDestPath.Append(file.getTail());

					todo.Push(std::make_tuple(file, fileDestPath));
				}

				for (auto& dir : directories)
				{
					Path dirDestPath = destinationPath;
					dirDestPath.Append(dir.getTail());

					todo.Push(std::make_tuple(dir, dirDestPath));
				}
			}
		}
	}

	void FileSystem::Remove(const Path& path, bool recursively)
	{
		if (!FileSystem::exists(path))
			return;

		if (recursively)
		{
			Vector<Path> files;
			Vector<Path> directories;

			getChildren(path, files, directories);

			for (auto& file : files)
				remove(file, false);

			for (auto& dir : directories)
				remove(dir, true);
		}

		FileSystem::removeFile(path);
	}

	void FileSystem::Move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		if (FileSystem::exists(newPath))
		{
			if (overwriteExisting)
				FileSystem::remove(newPath);
			else
			{
				BS_LOG(Warning, FileSystem, "Move operation failed because another file already exists at the new "
					"path: \"{0}\"", newPath);
				return;
			}
		}

		FileSystem::moveFile(oldPath, newPath);
	}

	Mutex FileScheduler::mMutex;
}
