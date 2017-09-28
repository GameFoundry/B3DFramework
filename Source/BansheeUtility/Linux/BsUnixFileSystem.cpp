//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
	LOGERR(String(__FUNCTION__) + ": " + (path__) + ": " + (strerror(errno__)));

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

	void FileSystem::removeFile(const Path& path)
	{
		String pathStr = path.toString();
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

	void FileSystem::copyFile(const Path& source, const Path& destination)
	{
		std::ifstream sourceStream(source.toString().c_str(), std::ios::binary);
		std::ofstream destinationStream(destination.toString().c_str(), std::ios::binary);

		destinationStream << sourceStream.rdbuf();
		sourceStream.close();
		destinationStream.close();
	}

	void FileSystem::moveFile(const Path& oldPath, const Path& newPath)
	{
		String oldPathStr = oldPath.toString();
		String newPathStr = newPath.toString();
		if (std::rename(oldPathStr.c_str(), newPathStr.c_str()) == -1)
		{
			LOGERR(String(__FUNCTION__) + ": renaming " + oldPathStr + " to " + newPathStr +
			       ": " + strerror(errno));
		}
	}

	SPtr<DataStream> FileSystem::openFile(const Path& path, bool readOnly)
	{
		String pathString = path.toString();

		DataStream::AccessMode accessMode = DataStream::READ;
		if (!readOnly)
			accessMode = (DataStream::AccessMode)(accessMode | (UINT32)DataStream::WRITE);

		return bs_shared_ptr_new<FileDataStream>(path, accessMode, true);
	}

	SPtr<DataStream> FileSystem::createAndOpenFile(const Path& path)
	{
		return bs_shared_ptr_new<FileDataStream>(path, DataStream::AccessMode::WRITE, true);
	}

	UINT64 FileSystem::getFileSize(const Path& path)
	{
		struct stat st_buf;

		if (stat(path.toString().c_str(), &st_buf) == 0)
		{
			return st_buf.st_size;
		}
		else
		{
			HANDLE_PATH_ERROR(path.toString(), errno);
			return -1;
		}
	}

	bool FileSystem::exists(const Path& path)
	{
		return unix_pathExists(path.toString());
	}

	bool FileSystem::isFile(const Path& path)
	{
		String pathStr = path.toString();
		return unix_pathExists(pathStr) && unix_isFile(pathStr);
	}

	bool FileSystem::isDirectory(const Path& path)
	{
		String pathStr = path.toString();
		return unix_pathExists(pathStr) && unix_isDirectory(pathStr);
	}

	void FileSystem::createDir(const Path& path)
	{
		Path parentPath = path;
		while (!exists(parentPath) && parentPath.getNumDirectories() > 0)
		{
			parentPath = parentPath.getParent();
		}

		for (UINT32 i = parentPath.getNumDirectories(); i < path.getNumDirectories(); i++)
		{
			parentPath.append(path[i]);
			unix_createDirectory(parentPath.toString());
		}
	}

	void FileSystem::getChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
	{
		const String pathStr = dirPath.toString();

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
			const String filename(ep->d_name);
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

	std::time_t FileSystem::getLastModifiedTime(const Path& path)
	{
		struct stat st_buf;
		stat(path.toString().c_str(), &st_buf);
		std::time_t time = st_buf.st_mtime;

		return time;
	}

	Path FileSystem::getWorkingDirectoryPath()
	{
		char *buffer = bs_newN<char>(PATH_MAX);

		String wd;
		if (getcwd(buffer, PATH_MAX) != NULL)
			wd = buffer;

		bs_free(buffer);

		const int error = errno;
		if (error)
			LOGERR(String("Error when calling getcwd(): ") + strerror(error));

		return Path(wd);
	}

	bool FileSystem::iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
		std::function<bool(const Path&)> dirCallback, bool recursive)
	{
		BS_ASSERT(!"TODOPORT: implement FileSystem::iterate()");
		return true;
	}

	Path FileSystem::getTempDirectoryPath()
	{
		String tmpdir;

		// Try different things:
		// 1) If defined, honor the TMPDIR environnement variable
		char *TMPDIR = getenv("TMPDIR");
		if (TMPDIR != NULL)
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

		tmpdir.append("/banshee-XXXXXX");
		size_t len = tmpdir.size()+1;
		char *nameTemplate = bs_newN<char>(len);
		snprintf(nameTemplate, len, "%s", tmpdir.c_str());

		char *directoryName = mkdtemp(nameTemplate);
		if (directoryName == NULL)
		{
			LOGERR(String(__FUNCTION__) + ": " + strerror(errno));
			return Path(StringUtil::BLANK);
		}

		return Path(String(directoryName) + "/");
	}
}
