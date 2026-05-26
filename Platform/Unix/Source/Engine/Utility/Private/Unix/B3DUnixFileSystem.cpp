//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/B3DFileSystem.h"

#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <climits>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#define HANDLE_PATH_ERROR(path__, errno__) \
	B3D_LOG(Error, LogFileSystem, (String(__FUNCTION__) + ": " + (path__) + ": " + (strerror(errno__))));

using namespace b3d;

bool unix_pathExists(const String& path)
{
	struct stat st_buf;
	if(stat(path.c_str(), &st_buf) == 0)
		return true;
	else if(errno == ENOENT) // No such file or directory
		return false;
	else
	{
		HANDLE_PATH_ERROR(path, errno);
		return false;
	}
}

bool unix_stat(const String& path, struct stat* st_buf)
{
	if(stat(path.c_str(), st_buf) != 0)
	{
		HANDLE_PATH_ERROR(path, errno);
		return false;
	}
	return true;
}

bool unix_isFile(const String& path)
{
	struct stat st_buf;
	if(unix_stat(path, &st_buf))
		return S_ISREG(st_buf.st_mode);

	return false;
}

bool unix_isDirectory(const String& path)
{
	struct stat st_buf;
	if(unix_stat(path, &st_buf))
		return S_ISDIR(st_buf.st_mode);

	return false;
}

bool unix_createDirectory(const String& path)
{
	if(unix_pathExists(path) && unix_isDirectory(path))
		return false;

	if(mkdir(path.c_str(), 0755))
	{
		HANDLE_PATH_ERROR(path, errno);
		return false;
	}

	return true;
}

bool FileSystem::RemoveFile(const Path& path)
{
	String pathStr = path.ToString();
	if(unix_isDirectory(pathStr))
	{
		if(rmdir(pathStr.c_str()))
		{
			HANDLE_PATH_ERROR(pathStr, errno);
			return false;
		}
	}
	else
	{
		if(unlink(pathStr.c_str()))
		{
			HANDLE_PATH_ERROR(pathStr, errno);
			return false;
		}
	}

	return true;
}

bool FileSystem::CopyFile(const Path& source, const Path& destination)
{
	std::ifstream sourceStream(source.ToString().c_str(), std::ios::binary);
	std::ofstream destinationStream(destination.ToString().c_str(), std::ios::binary);

	destinationStream << sourceStream.rdbuf();
	sourceStream.close();
	destinationStream.close();

	return !destinationStream.bad();
}

bool FileSystem::MoveFile(const Path& oldPath, const Path& newPath)
{
	String oldPathStr = oldPath.ToString();
	String newPathStr = newPath.ToString();
	if(std::rename(oldPathStr.c_str(), newPathStr.c_str()) == -1)
	{
		// Cross-filesystem copy is likely needed (for example, /tmp to Banshee install dir while copying assets)
		std::ifstream src(oldPathStr.c_str(), std::ios::binary);
		std::ofstream dst(newPathStr.c_str(), std::ios::binary);
		dst << src.rdbuf(); // First, copy

		// Error handling
		src.close();
		if(!src)
		{
			B3D_LOG(Error, LogFileSystem, String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr + ": " + strerror(errno));
			return false; // Do not remove source if we failed!
		}

		// Then, remove source file (hopefully succeeds)
		if(std::remove(oldPathStr.c_str()) == -1)
		{
			B3D_LOG(Error, LogFileSystem, String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr + ": " + strerror(errno));
			return false;
		}
	}

	return true;
}

TShared<DataStream> FileSystem::CreateFileStream(const Path& path, FileAccessFlags access)
{
	TShared<FileDataStream> fileDataStream = B3DMakeShared<FileDataStream>(path, access);
	if(!fileDataStream->Open())
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to open file at path '{0}'. File stream failed to open.", path);
		return nullptr;
	}

	return fileDataStream;
}

u64 FileSystem::GetFileSize(const Path& path)
{
	struct stat st_buf;

	if(stat(path.ToString().c_str(), &st_buf) == 0)
	{
		return (u64)st_buf.st_size;
	}
	else
	{
		HANDLE_PATH_ERROR(path.ToString(), errno);
		return (u64)-1;
	}
}

bool FileSystem::Exists(const Path& path)
{
	return unix_pathExists(path.ToString());
}

bool FileSystem::IsFile(const Path& path)
{
	String pathStr = path.ToString();
	return unix_pathExists(pathStr) && unix_isFile(pathStr);
}

bool FileSystem::IsDirectory(const Path& path)
{
	String pathStr = path.ToString();
	return unix_pathExists(pathStr) && unix_isDirectory(pathStr);
}

bool FileSystem::CreateDir(const Path& path)
{
	Path parentPath = path;
	while(!Exists(parentPath) && parentPath.GetDirectoryCount() > 0)
	{
		parentPath = parentPath.GetParent();
	}

	for(u32 directoryIndex = parentPath.GetDirectoryCount(); directoryIndex < path.GetDirectoryCount(); directoryIndex++)
	{
		parentPath.Append(path[directoryIndex]);
		if(!unix_createDirectory(parentPath.ToString()))
			return false;
	}

	// Last "file" entry is also considered a directory
	if(!parentPath.Equals(path))
	{
		if(!unix_createDirectory(path.ToString()))
			return false;
	}

	return true;
}

void FileSystem::GetChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
{
	const String pathStr = dirPath.ToString();

	if(unix_isFile(pathStr))
		return;

	DIR* dp = opendir(pathStr.c_str());
	if(dp == NULL)
	{
		HANDLE_PATH_ERROR(pathStr, errno);
		return;
	}

	struct dirent* ep;
	while((ep = readdir(dp)))
	{
		const String filename(ep->d_name);
		if(filename != "." && filename != "..")
		{
			if(unix_isDirectory(pathStr + "/" + filename))
				directories.push_back(dirPath + (filename + "/"));
			else
				files.push_back(dirPath + filename);
		}
	}
	closedir(dp);
}

std::time_t FileSystem::GetLastModifiedTime(const Path& path)
{
	struct stat st_buf;
	stat(path.ToString().c_str(), &st_buf);
	std::time_t time = st_buf.st_mtime;

	return time;
}

Path FileSystem::GetWorkingDirectoryPath()
{
	char* buffer = B3DNewMultiple<char>(PATH_MAX);

	String wd;
	if(getcwd(buffer, PATH_MAX) != nullptr)
		wd = buffer;
	else
		B3D_LOG(Error, LogFileSystem, String("Error when calling getcwd(): ") + strerror(errno));

	B3DFree(buffer);
	return Path(wd);
}

bool FileSystem::Iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback, std::function<bool(const Path&)> dirCallback, bool recursive)
{
	String pathStr = dirPath.ToString();

	if(unix_isFile(pathStr))
		return false;

	DIR* dirHandle = opendir(pathStr.c_str());
	if(dirHandle == nullptr)
	{
		HANDLE_PATH_ERROR(pathStr, errno);
		return false;
	}

	dirent* entry;
	while((entry = readdir(dirHandle)))
	{
		String filename(entry->d_name);
		if(filename == "." || filename == "..")
			continue;

		Path fullPath = dirPath;
		if(unix_isDirectory(pathStr + "/" + filename))
		{
			Path childDir = fullPath.Append(filename + "/");
			if(dirCallback != nullptr)
			{
				if(!dirCallback(childDir))
				{
					closedir(dirHandle);
					return false;
				}
			}

			if(recursive)
			{
				if(!Iterate(childDir, fileCallback, dirCallback, recursive))
				{
					closedir(dirHandle);
					return false;
				}
			}
		}
		else
		{
			Path filePath = fullPath.Append(filename);
			if(fileCallback != nullptr)
			{
				if(!fileCallback(filePath))
				{
					closedir(dirHandle);
					return false;
				}
			}
		}
	}
	closedir(dirHandle);

	return true;
}

Path FileSystem::GetTempDirectoryPath()
{
	String tmpdir;

	// Try different things:
	// 1) If defined, honor the TMPDIR environnement variable
	char* TMPDIR = getenv("TMPDIR");
	if(TMPDIR != nullptr)
		tmpdir = TMPDIR;
	else
	{
		// 2) If defined, honor the P_tmpdir macro
#ifdef P_tmpdir
		tmpdir = String(P_tmpdir);
#else
		// 3) If everything else fails, simply default to /tmp
		tmpdir = String("/tmp");
#endif
	}

	tmpdir.append("/bsf-XXXXXX");

	// null terminated, modifiable tmpdir name template
	Vector<char> nameTemplate(tmpdir.c_str(), tmpdir.c_str() + tmpdir.size() + 1);
	char* directoryName = mkdtemp(nameTemplate.data());

	if(directoryName == nullptr)
	{
		B3D_LOG(Error, LogFileSystem, String(__FUNCTION__) + ": " + strerror(errno));
		return Path(StringUtil::BLANK);
	}

	return Path(String(directoryName) + "/");
}
