//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsFileSystem.h"
#include "Debug/BsDebug.h"

using namespace bs;

void FileSystem::Copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
{
	Stack<std::tuple<Path, Path>> todo;
	todo.push(std::make_tuple(oldPath, newPath));

	while(!todo.empty())
	{
		auto current = todo.top();
		todo.pop();

		Path sourcePath = std::get<0>(current);
		if(!FileSystem::Exists(sourcePath))
			continue;

		bool srcIsFile = FileSystem::IsFile(sourcePath);
		Path destinationPath = std::get<1>(current);
		bool destExists = FileSystem::Exists(destinationPath);

		if(destExists)
		{
			if(FileSystem::IsFile(destinationPath))
			{
				if(overwriteExisting)
					FileSystem::Remove(destinationPath);
				else
				{
					BS_LOG(Warning, FileSystem, "Copy operation failed because another file already exists at the new "
												"path: \"{0}\"",
						   destinationPath);
					return;
				}
			}
		}

		if(srcIsFile)
		{
			FileSystem::CopyFile(sourcePath, destinationPath);
		}
		else
		{
			if(!destExists)
				FileSystem::CreateDir(destinationPath);

			Vector<Path> files;
			Vector<Path> directories;
			GetChildren(destinationPath, files, directories);

			for(auto& file : files)
			{
				Path fileDestPath = destinationPath;
				fileDestPath.Append(file.GetTail());

				todo.push(std::make_tuple(file, fileDestPath));
			}

			for(auto& dir : directories)
			{
				Path dirDestPath = destinationPath;
				dirDestPath.Append(dir.GetTail());

				todo.push(std::make_tuple(dir, dirDestPath));
			}
		}
	}
}

void FileSystem::Remove(const Path& path, bool recursively)
{
	if(!FileSystem::Exists(path))
		return;

	if(recursively)
	{
		Vector<Path> files;
		Vector<Path> directories;

		GetChildren(path, files, directories);

		for(auto& file : files)
			Remove(file, false);

		for(auto& dir : directories)
			Remove(dir, true);
	}

	FileSystem::RemoveFile(path);
}

void FileSystem::Move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
{
	if(FileSystem::Exists(newPath))
	{
		if(overwriteExisting)
			FileSystem::Remove(newPath);
		else
		{
			BS_LOG(Warning, FileSystem, "Move operation failed because another file already exists at the new "
										"path: \"{0}\"",
				   newPath);
			return;
		}
	}

	FileSystem::MoveFile(oldPath, newPath);
}

Mutex FileScheduler::mMutex;
