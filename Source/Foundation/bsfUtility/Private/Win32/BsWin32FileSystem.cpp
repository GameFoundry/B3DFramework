//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/BsFileSystem.h"
#include "Error/BsException.h"
#include "FileSystem/BsDataStream.h"
#include "Debug/BsDebug.h"
#include <windows.h>
#include "String/BsUnicode.h"

namespace bs
{
	void win32_handleError(DWORD error, const WString& path)
	{
		switch (error)
		{
		case ERROR_FILE_NOT_FOUND:
			BS_LOG(Error, FileSystem, "File at path: \"{0}\" not found.", path);
			break;
		case ERROR_PATH_NOT_FOUND:
		case ERROR_BAD_NETPATH:
		case ERROR_CANT_RESOLVE_FILENAME:
		case ERROR_INVALID_DRIVE:
			BS_LOG(Error, FileSystem, "Path \"{0}\" not found.", path);
			break;
		case ERROR_ACCESS_DENIED:
			BS_LOG(Error, FileSystem, "Access to path \"{0}\" denied.", path);
			break;
		case ERROR_ALREADY_EXISTS:
		case ERROR_FILE_EXISTS:
			BS_LOG(Error, FileSystem, "File/folder at path \"{0}\" already exists.", path);
			break;
		case ERROR_INVALID_NAME:
		case ERROR_DIRECTORY:
		case ERROR_FILENAME_EXCED_RANGE:
		case ERROR_BAD_PATHNAME:
			BS_LOG(Error, FileSystem, "Invalid path string: \"{0}\".", path);
			break;
		case ERROR_FILE_READ_ONLY:
			BS_LOG(Error, FileSystem, "File at path \"{0}\" is read only.", path);
			break;
		case ERROR_CANNOT_MAKE:
			BS_LOG(Error, FileSystem, "Cannot create file/folder at path: \"{0}\".", path);
			break;
		case ERROR_DIR_NOT_EMPTY:
			BS_LOG(Error, FileSystem, "Directory at path \"{0}\" not empty.", path);
			break;
		case ERROR_WRITE_FAULT:
			BS_LOG(Error, FileSystem, "Error while writing a file at path \"{0}\".", path);
			break;
		case ERROR_READ_FAULT:
			BS_LOG(Error, FileSystem, "Error while reading a file at path \"{0}\".", path);
			break;
		case ERROR_SHARING_VIOLATION:
			BS_LOG(Error, FileSystem, "Sharing violation at path \"{0}\".", path);
			break;
		case ERROR_LOCK_VIOLATION:
			BS_LOG(Error, FileSystem, "Lock violation at path \"{0}\".", path);
			break;
		case ERROR_HANDLE_EOF:
			BS_LOG(Error, FileSystem, "End of file reached for file at path \"{0}\".", path);
			break;
		case ERROR_HANDLE_DISK_FULL:
		case ERROR_DISK_FULL:
			BS_LOG(Error, FileSystem, "Disk full.");
			break;
		case ERROR_NEGATIVE_SEEK:
			BS_LOG(Error, FileSystem, "Negative seek.");
			break;
		default:
			BS_LOG(Error, FileSystem, "Undefined file system exception: {0}", (UINT32)error);
			break;
		}
	}

	WString win32_getCurrentDirectory()
	{
		DWORD len = GetCurrentDirectoryW(0, NULL);
		if (len > 0)
		{
			wchar_t* buffer = (wchar_t*)bs_alloc(len * sizeof(wchar_t));

			DWORD n = GetCurrentDirectoryW(len, buffer);
			if (n > 0 && n <= len)
			{
				WString Result(buffer);
				if (result[result.size() - 1] != L'\\')
					result.Append(L"\\");

				bs_free(buffer);
				return result;
			}

			bs_free(buffer);
		}

		return StringUtil::WBLANK;
	}

	WString win32_getTempDirectory()
	{
		DWORD len = GetTempPathW(0, NULL);
		if (len > 0)
		{
			wchar_t* buffer = (wchar_t*)bs_alloc(len * sizeof(wchar_t));

			DWORD n = GetTempPathW(len, buffer);
			if (n > 0 && n <= len)
			{
				WString Result(buffer);
				if (result[result.size() - 1] != L'\\')
					result.Append(L"\\");

				bs_free(buffer);
				return result;
			}

			bs_free(buffer);
		}

		return StringUtil::WBLANK;
	}

	bool win32_pathExists(const WString& path)
	{
		DWORD attr = GetFileAttributesW(path.c_str());
		if (attr == 0xFFFFFFFF)
		{
			switch (GetLastError())
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
			case ERROR_NOT_READY:
			case ERROR_INVALID_DRIVE:
				return false;
			default:
				win32_handleError(GetLastError(), path);
			}
		}
		return true;
	}

	bool win32_isDirectory(const WString& path)
	{
		DWORD attr = GetFileAttributesW(path.c_str());
		if (attr == 0xFFFFFFFF)
			win32_handleError(GetLastError(), path);

		return (attr & FILE_ATTRIBUTE_DIRECTORY) != FALSE;
	}

	bool win32_isDevice(const WString& path)
	{
		WString ucPath = path;
		StringUtil::toUpperCase(ucPath);

		return
			ucPath.Compare(0, 4, L"\\\\.\\") == 0 ||
			ucPath.Compare(L"CON") == 0 ||
			ucPath.Compare(L"PRN") == 0 ||
			ucPath.Compare(L"AUX") == 0 ||
			ucPath.Compare(L"NUL") == 0 ||
			ucPath.Compare(L"LPT1") == 0 ||
			ucPath.Compare(L"LPT2") == 0 ||
			ucPath.Compare(L"LPT3") == 0 ||
			ucPath.Compare(L"LPT4") == 0 ||
			ucPath.Compare(L"LPT5") == 0 ||
			ucPath.Compare(L"LPT6") == 0 ||
			ucPath.Compare(L"LPT7") == 0 ||
			ucPath.Compare(L"LPT8") == 0 ||
			ucPath.Compare(L"LPT9") == 0 ||
			ucPath.Compare(L"COM1") == 0 ||
			ucPath.Compare(L"COM2") == 0 ||
			ucPath.Compare(L"COM3") == 0 ||
			ucPath.Compare(L"COM4") == 0 ||
			ucPath.Compare(L"COM5") == 0 ||
			ucPath.Compare(L"COM6") == 0 ||
			ucPath.Compare(L"COM7") == 0 ||
			ucPath.Compare(L"COM8") == 0 ||
			ucPath.Compare(L"COM9") == 0;
	}

	bool win32_isFile(const WString& path)
	{
		return !win32_isDirectory(path) && !win32_isDevice(path);
	}

	bool win32_createFile(const WString& path)
	{
		HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile);
			return true;
		}
		else if (GetLastError() == ERROR_FILE_EXISTS)
			return false;
		else
			win32_handleError(GetLastError(), path);

		return false;
	}

	bool win32_createDirectory(const WString& path)
	{
		if (win32_pathExists(path) && win32_isDirectory(path))
			return false;

		if (CreateDirectoryW(path.c_str(), 0) == FALSE)
			win32_handleError(GetLastError(), path);

		return true;
	}

	UINT64 win32_getFileSize(const WString& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA attrData;
		if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attrData) == FALSE)
			win32_handleError(GetLastError(), path);

		LARGE_INTEGER li;
		li.LowPart = attrData.nFileSizeLow;
		li.HighPart = attrData.nFileSizeHigh;
		return (UINT64)li.QuadPart;
	}

	std::time_t win32_getLastModifiedTime(const WString& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad) == 0)
			win32_handleError(GetLastError(), path);

		ULARGE_INTEGER ull;
		ull.LowPart = fad.ftLastWriteTime.dwLowDateTime;
		ull.HighPart = fad.ftLastWriteTime.dwHighDateTime;

		return (std::time_t) ((ull.QuadPart / 10000000ULL) - 11644473600ULL);
	}

	void FileSystem::RemoveFile(const Path& path)
	{
		WString pathStr = UTF8::toWide(path.ToString());
		if (win32_isDirectory(pathStr))
		{
			if (RemoveDirectoryW(pathStr.c_str()) == 0)
				win32_handleError(GetLastError(), pathStr);
		}
		else
		{
			if (DeleteFileW(pathStr.c_str()) == 0)
				win32_handleError(GetLastError(), pathStr);
		}
	}

	void FileSystem::CopyFile(const Path& from, const Path& to)
	{
		WString fromStr = UTF8::toWide(from.ToString());
		WString toStr = UTF8::toWide(to.ToString());

		if (CopyFileW(fromStr.c_str(), toStr.c_str(), FALSE) == FALSE)
			win32_handleError(GetLastError(), fromStr);
	}

	void FileSystem::MoveFile(const Path& oldPath, const Path& newPath)
	{
		WString oldPathStr = UTF8::toWide(oldPath.ToString());
		WString newPathStr = UTF8::toWide(newPath.ToString());

		if (MoveFileW(oldPathStr.c_str(), newPathStr.c_str()) == 0)
			win32_handleError(GetLastError(), oldPathStr);
	}

	SPtr<DataStream> FileSystem::OpenFile(const Path& fullPath, bool readOnly)
	{
		WString pathWString = UTF8::toWide(fullPath.ToString());
		const wchar_t* pathString = pathWString.c_str();

		if (!win32_pathExists(pathString) || !win32_isFile(pathString))
		{
			BS_LOG(Warning, Platform, "Attempting to open a file that doesn't exist: {0}", fullPath);
			return nullptr;
		}

		DataStream::AccessMode accessMode = DataStream::READ;
		if (!readOnly)
			accessMode = (DataStream::AccessMode)(accessMode | (UINT32)DataStream::WRITE);

		return bs_shared_ptr_new<FileDataStream>(fullPath, accessMode, true);
	}

	SPtr<DataStream> FileSystem::CreateAndOpenFile(const Path& fullPath)
	{
		return bs_shared_ptr_new<FileDataStream>(fullPath, DataStream::AccessMode::WRITE, true);
	}

	UINT64 FileSystem::GetFileSize(const Path& fullPath)
	{
		return win32_getFileSize(UTF8::toWide(fullPath.ToString()));
	}

	bool FileSystem::Exists(const Path& fullPath)
	{
		return win32_pathExists(UTF8::toWide(fullPath.ToString()));
	}

	bool FileSystem::IsFile(const Path& fullPath)
	{
		WString pathStr = UTF8::toWide(fullPath.ToString());

		return win32_pathExists(pathStr) && win32_isFile(pathStr);
	}

	bool FileSystem::IsFolder(const Path& fullPath)
	{
		WString pathStr = UTF8::toWide(fullPath.ToString());

		return win32_pathExists(pathStr) && win32_isDirectory(pathStr);
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
			win32_createDirectory(UTF8::toWide(parentPath.ToString()));
		}

		if (fullPath.IsFile())
			win32_createDirectory(UTF8::toWide(fullPath.ToString()));
	}

	void FileSystem::GetChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
	{
		WString findPath = UTF8::toWide(dirPath.ToString());

		if (win32_isFile(findPath))
			return;

		if(dirPath.IsFile()) // Assuming the file is a folder, just improperly formatted in Path
			findPath.Append(L"\\*");
		else
			findPath.Append(L"*");

		WIN32_FIND_DATAW findData;
		HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
		if(fileHandle == INVALID_HANDLE_VALUE)
		{
			win32_handleError(GetLastError(), findPath);
			return;
		}

		WString tempName;
		do
		{
			tempName = findData.cFileName;

			if (tempName != L"." && tempName != L"..")
			{
				Path fullPath = dirPath;
				if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
					directories.push_back(fullPath.Append(UTF8::fromWide(tempName) + u8"/"));
				else
					files.push_back(fullPath.Append(UTF8::fromWide(tempName)));
			}

			if(FindNextFileW(fileHandle, &findData) == FALSE)
			{
				if (GetLastError() != ERROR_NO_MORE_FILES)
					win32_handleError(GetLastError(), findPath);

				break;
			}
		} while (true);

		FindClose(fileHandle);
	}

	bool FileSystem::Iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback,
		std::function<bool(const Path&)> dirCallback, bool recursive)
	{
		WString findPath = UTF8::toWide(dirPath.ToString());

		if (win32_isFile(findPath))
			return false;

		if (dirPath.IsFile()) // Assuming the file is a folder, just improperly formatted in Path
			findPath.Append(L"\\*");
		else
			findPath.Append(L"*");

		WIN32_FIND_DATAW findData;
		HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
		if (fileHandle == INVALID_HANDLE_VALUE)
		{
			win32_handleError(GetLastError(), findPath);
			return false;
		}

		WString tempName;
		do
		{
			tempName = findData.cFileName;

			if (tempName != L"." && tempName != L"..")
			{
				Path fullPath = dirPath;
				if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				{
					Path childDir = fullPath.Append(UTF8::fromWide(tempName) + u8"/");
					if (dirCallback != nullptr)
					{
						if (!dirCallback(childDir))
						{
							FindClose(fileHandle);
							return false;
						}
					}

					if (recursive)
					{
						if (!iterate(childDir, fileCallback, dirCallback, recursive))
						{
							FindClose(fileHandle);
							return false;
						}
					}
				}
				else
				{
					Path filePath = fullPath.Append(UTF8::fromWide(tempName));
					if (fileCallback != nullptr)
					{
						if (!fileCallback(filePath))
						{
							FindClose(fileHandle);
							return false;
						}
					}
				}
			}

			if(FindNextFileW(fileHandle, &findData) == FALSE)
			{
				if (GetLastError() != ERROR_NO_MORE_FILES)
					win32_handleError(GetLastError(), findPath);

				break;
			}
		} while (true);

		FindClose(fileHandle);
		return true;
	}

	std::time_t FileSystem::GetLastModifiedTime(const Path& fullPath)
	{
		return win32_getLastModifiedTime(UTF8::toWide(fullPath.ToString()));
	}

	Path FileSystem::GetWorkingDirectoryPath()
	{
		const String utf8dir = UTF8::fromWide(win32_getCurrentDirectory());
		return Path(utf8dir);
	}

	Path FileSystem::GetTempDirectoryPath()
	{
		const String utf8dir = UTF8::fromWide(win32_getTempDirectory());
		return Path(utf8dir);
	}
}
