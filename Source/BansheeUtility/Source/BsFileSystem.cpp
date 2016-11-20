//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

#include "BsFileSystem.h"
#include "BsDebug.h"

namespace bs
{
	void FileSystem::copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		Stack<std::tuple<Path, Path>> todo;
		todo.push(std::make_tuple(oldPath, newPath));

		while (!todo.empty())
		{
			auto current = todo.top();
			todo.pop();

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
						LOGWRN("Copy operation failed because another file already exists at the new path: \"" + destinationPath.toString() + "\"");
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
					fileDestPath.append(file.getWTail());

					todo.push(std::make_tuple(file, fileDestPath));
				}

				for (auto& dir : directories)
				{
					Path dirDestPath = destinationPath;
					dirDestPath.append(dir.getWTail());

					todo.push(std::make_tuple(dir, dirDestPath));
				}
			}
		}
	}

	void FileSystem::remove(const Path& path, bool recursively)
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

	void FileSystem::move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
	{
		if (FileSystem::exists(newPath))
		{
			if (overwriteExisting)
				FileSystem::remove(newPath);
			else
			{
				LOGWRN("Move operation failed because another file already exists at the new path: \"" + newPath.toString() + "\"");
				return;
			}
		}

		FileSystem::moveFile(oldPath, newPath);
	}

}
