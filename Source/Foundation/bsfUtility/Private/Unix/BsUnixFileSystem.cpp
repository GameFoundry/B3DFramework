//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsFileSystem.h"

#include "Error/BsException.h"
#include "FileSystem/BsDataStream.h"
#include "Debug/BsDebug.h"

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
	BS_LOG(Error, FileSystem, (String(__FUNCTION__) + ": " + (path__) + ": " + (strerror(errno__))));

namespace bs
{
	bool unix_pathExists(const String& path)
	{
		struct stat st_buf;
		if (stat(path.c_str(), &st_buf) == 0)
			return true;
		else
			if (errno == ENOENT)    // No such file or directory
				return false;
			else
			{
				HANDLE_PATH_ERROR(path, errno);
				return false;
			}
	}

	bool unix_stat(const String& path, struct stat *st_buf)
	{
		if (stat(path.c_str(), st_buf) != 0)
		{
			HANDLE_PATH_ERROR(path, errno);
			return false;
		}
		return true;
	}

	bool unix_isFile(const String& path)
	{
		struct stat st_buf;
		if (unix_stat(path, &st_buf))
			return S_ISREG(st_buf.st_mode);

		return false;
	}

	bool unix_isDirectory(const String& path)
	{
		struct stat st_buf;
		if (unix_stat(path, &st_buf))
			return S_ISDIR(st_buf.st_mode);

		return false;
	}

	bool unix_createDirectory(const String& path)
	{
		if (unix_pathExists(path) && unix_isDirectory(path))
			return false;

		if (mkdir(path.c_str(), 0755))
		{
			HANDLE_PATH_ERROR(path, errno);
			return false;
		}

		return true;
	}

	void FileSystem::RemoveFile(const Path& path)
	{
		String pathStr = path.ToString();
		if (unix_isDirectory(pathStr))
		{
			if (rmdir(pathStr.c_str()))
				HANDLE_PATH_ERROR(pathStr, errno);
		}
		else
		{
			if (unlink(pathStr.c_str()))
				HANDLE_PATH_ERROR(pathStr, errno);
		}
	}

	void FileSystem::CopyFile(const Path& source, const Path& destination)
	{
		std::ifstream SourceStream(source.ToString().c_str(), std::ios::binary);
		std::ofstream DestinationStream(destination.ToString().c_str(), std::ios::binary);

		destinationStream << sourceStream.Rdbuf();
		sourceStream.Close();
		destinationStream.Close();
	}

	void FileSystem::MoveFile(const Path& oldPath, const Path& newPath)
	{
		String oldPathStr = oldPath.ToString();
		String newPathStr = newPath.ToString();
		if (std::rename(oldPathStr.c_str(), newPathStr.c_str()) == -1)
		{
			// Cross-filesystem copy is likely needed (for example, /tmp to Banshee install dir while copying assets)
			std::ifstream Src(oldPathStr.c_str(), std::ios::binary);
			std::ofstream Dst(newPathStr.c_str(), std::ios::binary);
			dst << src.Rdbuf(); // First, copy

			// Error handling
			src.Close();
			if (!src)
			{
				BS_LOG(Error, FileSystem, String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr +
						": " + strerror(errno));
				return; // Do not remove source if we failed!
			}

			// Then, remove source file (hopefully succeeds)
			if (std::remove(oldPathStr.c_str()) == -1)
			{
				BS_LOG(Error, FileSystem, String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr +
						": " + strerror(errno));
			}
		}
	}

	SPtr<DataStream> FileSystem::OpenFile(const Path& path, bool readOnly)
	{
		String pathString = path.ToString();

		DataStream::AccessMode accessMode = DataStream::READ;
		if (!readOnly)
			accessMode = (DataStream::AccessMode)((UINT32)accessMode | (UINT32)DataStream::WRITE);

		return bs_shared_ptr_new<FileDataStream>(path, accessMode, true);
	}

	SPtr<DataStream> FileSystem::CreateAndOpenFile(const Path& path)
	{
		return bs_shared_ptr_new<FileDataStream>(path, DataStream::AccessMode::WRITE, true);
	}

	UINT64 FileSystem::GetFileSize(const Path& path)
	{
		struct stat st_buf;

		if (stat(path.ToString().c_str(), &st_buf) == 0)
		{
			return (UINT64)st_buf.st_size;
		}
		else
		{
			HANDLE_PATH_ERROR(path.ToString(), errno);
			return (UINT64)-1;
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

	bool FileSystem::IsFolder(const Path& fullPath)
	{
		String pathStr = fullPath.ToString();
		return unix_pathExists(pathStr) && unix_isDirectory(pathStr);
	}

	void FileSystem::CreateFolder(const Path& fullPath)
	{
		Path parentPath = fullPath;
		while (!exists(parentPath) && parentPath.GetNumDirectories() > 0)
		{
			parentPath = parentPath.GetParent();
		}

		for (UINT32 i = parentPath.GetNumDirectories(); i < fullPath.getNumDirectories(); i++)
		{
			parentPath.Append(fullPath[i]);
			unix_createDirectory(parentPath.ToString());
		}

		// Last "file" entry is also considered a directory
		if(!parentPath.Equals(fullPath))
			unix_createDirectory(fullPath.ToString());
	}

	void FileSystem::GetChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
	{
		const String pathStr = dirPath.ToString();

		if (unix_isFile(pathStr))
			return;

		DIR *dp = opendir(pathStr.c_str());
		if (dp == NULL)
		{
			HANDLE_PATH_ERROR(pathStr, errno);
			return;
		}

		struct dirent *ep;
		while ( (ep = readdir(dp)) )
		{
			const String Filename(ep->d_name);
			if (filename != "." && filename != "..")
			{
				if (unix_isDirectory(pathStr + "/" + filename))
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
		char *buffer = bs_newN<char>(PATH_MAX);

		String wd;
		if (getcwd(buffer, PATH_MAX) != nullptr)
			wd = buffer;
		else
			BS_LOG(Error, FileSystem, String("Error when calling getcwd(): ") + strerror(errno));

		bs_free(buffer);
		return Path(wd);
	}

	bool FileSystem::Iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
		std::function<bool(const Path&)> dirCallback, bool recursive)
	{
		String pathStr = dirPath.ToString();

		if (unix_isFile(pathStr))
			return false;

		DIR* dirHandle = opendir(pathStr.c_str());
		if (dirHandle == nullptr)
		{
			HANDLE_PATH_ERROR(pathStr, errno);
			return false;
		}

		dirent* entry;
		while((entry = readdir(dirHandle)))
		{
			String Filename(entry->d_name);
			if (filename == "." || filename == "..")
				continue;

			Path fullPath = dirPath;
			if (unix_isDirectory(pathStr + "/" + filename))
			{
				Path childDir = fullPath.Append(filename + "/");
				if (dirCallback != nullptr)
				{
					if (!dirCallback(childDir))
					{
						closedir(dirHandle);
						return false;
					}
				}

				if (recursive)
				{
					if (!iterate(childDir, fileCallback, dirCallback, recursive))
					{
						closedir(dirHandle);
						return false;
					}
				}
			}
			else
			{
				Path filePath = fullPath.Append(filename);
				if (fileCallback != nullptr)
				{
					if (!fileCallback(filePath))
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
		if (TMPDIR != nullptr)
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

		tmpdir.Append("/bsf-XXXXXX");
		
		// null terminated, modifiable tmpdir name template
		Vector<char> NameTemplate(tmpdir.c_str(), tmpdir.c_str() + tmpdir.size() + 1);
		char *directoryName = mkdtemp(nameTemplate.Data());

		if (directoryName == nullptr)
		{
			BS_LOG(Error, FileSystem, String(__FUNCTION__) + ": " + strerror(errno));
			return Path(StringUtil::BLANK);
		}

		return Path(String(directoryName) + "/");
	}
}
