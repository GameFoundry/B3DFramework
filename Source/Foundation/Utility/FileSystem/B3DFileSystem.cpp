//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/B3DFileSystem.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

bool FileSystem::Copy(const Path& oldPath, const Path& newPath, bool overwriteExisting)
{
	Stack<std::tuple<Path, Path>> todo;
	todo.push(std::make_tuple(oldPath, newPath));

	bool anyFailed = false;
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
				{
					if(!FileSystem::Remove(destinationPath))
						anyFailed = true;
				}
				else
				{
					B3D_LOG(Warning, FileSystem, "Copy operation failed because another file already exists at the new path: \"{0}\"", destinationPath);
					anyFailed = true;
					continue;
				}
			}
		}

		if(srcIsFile)
		{
			if(!FileSystem::CopyFile(sourcePath, destinationPath))
				anyFailed = true;
		}
		else
		{
			if(!destExists)
			{
				if(!FileSystem::CreateDir(destinationPath))
				{
					anyFailed = true;
					continue;
				}
			}

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

	return !anyFailed;
}

bool FileSystem::Remove(const Path& path, bool recursively)
{
	if(!FileSystem::Exists(path))
		return true;

	bool anyFailed = false;
	if(recursively)
	{
		Vector<Path> files;
		Vector<Path> directories;

		GetChildren(path, files, directories);

		for(auto& file : files)
		{
			if(!Remove(file, false))
				anyFailed = true;
		}

		for(auto& dir : directories)
		{
			if(!Remove(dir, true))
				anyFailed = true;
		}
	}

	return FileSystem::RemoveFile(path) && !anyFailed;
}

bool FileSystem::Move(const Path& oldPath, const Path& newPath, bool overwriteExisting)
{
	if(FileSystem::Exists(newPath))
	{
		if(overwriteExisting)
		{
			if(!FileSystem::Remove(newPath))
				return false;
		}
		else
		{
			B3D_LOG(Warning, FileSystem, "Move operation failed because another file already exists at the new path: \"{0}\"", newPath);
			return false;
		}
	}

	return FileSystem::MoveFile(oldPath, newPath);
}

Mutex FileScheduler::mMutex;
