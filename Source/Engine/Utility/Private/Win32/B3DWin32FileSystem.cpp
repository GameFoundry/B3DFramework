//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"
#include "String/B3DUnicode.h"
#include <windows.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "shlobj.h"

#undef CopyFile
#undef MoveFile

using namespace b3d;

void Win32HandleError(DWORD error, const WString& path)
{
	switch(error)
	{
	case ERROR_FILE_NOT_FOUND:
		B3D_LOG(Error, LogFileSystem, "File at path: \"{0}\" not found.", path);
		break;
	case ERROR_PATH_NOT_FOUND:
	case ERROR_BAD_NETPATH:
	case ERROR_CANT_RESOLVE_FILENAME:
	case ERROR_INVALID_DRIVE:
		B3D_LOG(Error, LogFileSystem, "Path \"{0}\" not found.", path);
		break;
	case ERROR_ACCESS_DENIED:
		B3D_LOG(Error, LogFileSystem, "Access to path \"{0}\" denied.", path);
		break;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
		B3D_LOG(Error, LogFileSystem, "File/folder at path \"{0}\" already exists.", path);
		break;
	case ERROR_INVALID_NAME:
	case ERROR_DIRECTORY:
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_BAD_PATHNAME:
		B3D_LOG(Error, LogFileSystem, "Invalid path string: \"{0}\".", path);
		break;
	case ERROR_FILE_READ_ONLY:
		B3D_LOG(Error, LogFileSystem, "File at path \"{0}\" is read only.", path);
		break;
	case ERROR_CANNOT_MAKE:
		B3D_LOG(Error, LogFileSystem, "Cannot create file/folder at path: \"{0}\".", path);
		break;
	case ERROR_DIR_NOT_EMPTY:
		B3D_LOG(Error, LogFileSystem, "Directory at path \"{0}\" not empty.", path);
		break;
	case ERROR_WRITE_FAULT:
		B3D_LOG(Error, LogFileSystem, "Error while writing a file at path \"{0}\".", path);
		break;
	case ERROR_READ_FAULT:
		B3D_LOG(Error, LogFileSystem, "Error while reading a file at path \"{0}\".", path);
		break;
	case ERROR_SHARING_VIOLATION:
		B3D_LOG(Error, LogFileSystem, "Sharing violation at path \"{0}\".", path);
		break;
	case ERROR_LOCK_VIOLATION:
		B3D_LOG(Error, LogFileSystem, "Lock violation at path \"{0}\".", path);
		break;
	case ERROR_HANDLE_EOF:
		B3D_LOG(Error, LogFileSystem, "End of file reached for file at path \"{0}\".", path);
		break;
	case ERROR_HANDLE_DISK_FULL:
	case ERROR_DISK_FULL:
		B3D_LOG(Error, LogFileSystem, "Disk full.");
		break;
	case ERROR_NEGATIVE_SEEK:
		B3D_LOG(Error, LogFileSystem, "Negative seek.");
		break;
	default:
		B3D_LOG(Error, LogFileSystem, "Undefined file system exception: {0}", (u32)error);
		break;
	}
}

WString Win32GetCurrentDirectory()
{
	DWORD len = GetCurrentDirectoryW(0, NULL);
	if(len > 0)
	{
		wchar_t* buffer = (wchar_t*)B3DAllocate(len * sizeof(wchar_t));

		DWORD n = GetCurrentDirectoryW(len, buffer);
		if(n > 0 && n <= len)
		{
			WString result(buffer);
			if(result[result.size() - 1] != L'\\')
				result.append(L"\\");

			B3DFree(buffer);
			return result;
		}

		B3DFree(buffer);
	}

	return StringUtility::kWblank;
}

WString Win32GetTempDirectory()
{
	DWORD len = GetTempPathW(0, NULL);
	if(len > 0)
	{
		wchar_t* buffer = (wchar_t*)B3DAllocate(len * sizeof(wchar_t));

		DWORD n = GetTempPathW(len, buffer);
		if(n > 0 && n <= len)
		{
			WString result(buffer);
			if(result[result.size() - 1] != L'\\')
				result.append(L"\\");

			B3DFree(buffer);
			return result;
		}

		B3DFree(buffer);
	}

	return StringUtility::kWblank;
}

bool Win32PathExists(const WString& path)
{
	DWORD attr = GetFileAttributesW(path.c_str());
	if(attr == 0xFFFFFFFF)
	{
		switch(GetLastError())
		{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_NOT_READY:
		case ERROR_INVALID_DRIVE:
			return false;
		default:
			Win32HandleError(GetLastError(), path);
		}
	}
	return true;
}

bool Win32IsDirectory(const WString& path)
{
	DWORD attr = GetFileAttributesW(path.c_str());
	if(attr == 0xFFFFFFFF)
		Win32HandleError(GetLastError(), path);

	return (attr & FILE_ATTRIBUTE_DIRECTORY) != FALSE;
}

bool Win32IsDevice(const WString& path)
{
	WString ucPath = path;
	StringUtility::ToUpperCase(ucPath);

	return ucPath.compare(0, 4, L"\\\\.\\") == 0 ||
		ucPath.compare(L"CON") == 0 ||
		ucPath.compare(L"PRN") == 0 ||
		ucPath.compare(L"AUX") == 0 ||
		ucPath.compare(L"NUL") == 0 ||
		ucPath.compare(L"LPT1") == 0 ||
		ucPath.compare(L"LPT2") == 0 ||
		ucPath.compare(L"LPT3") == 0 ||
		ucPath.compare(L"LPT4") == 0 ||
		ucPath.compare(L"LPT5") == 0 ||
		ucPath.compare(L"LPT6") == 0 ||
		ucPath.compare(L"LPT7") == 0 ||
		ucPath.compare(L"LPT8") == 0 ||
		ucPath.compare(L"LPT9") == 0 ||
		ucPath.compare(L"COM1") == 0 ||
		ucPath.compare(L"COM2") == 0 ||
		ucPath.compare(L"COM3") == 0 ||
		ucPath.compare(L"COM4") == 0 ||
		ucPath.compare(L"COM5") == 0 ||
		ucPath.compare(L"COM6") == 0 ||
		ucPath.compare(L"COM7") == 0 ||
		ucPath.compare(L"COM8") == 0 ||
		ucPath.compare(L"COM9") == 0;
}

bool Win32IsFile(const WString& path)
{
	return !Win32IsDirectory(path) && !Win32IsDevice(path);
}

bool Win32CreateFile(const WString& path)
{
	HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return true;
	}
	else if(GetLastError() == ERROR_FILE_EXISTS)
		return false;
	else
		Win32HandleError(GetLastError(), path);

	return false;
}

bool Win32CreateDirectory(const WString& path)
{
	if(Win32PathExists(path) && Win32IsDirectory(path))
		return false;

	if(CreateDirectoryW(path.c_str(), 0) == FALSE)
	{
		Win32HandleError(GetLastError(), path);
		return false;
	}

	return true;
}

u64 Win32GetFileSize(const WString& path)
{
	WIN32_FILE_ATTRIBUTE_DATA attrData;
	if(GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &attrData) == FALSE)
		Win32HandleError(GetLastError(), path);

	LARGE_INTEGER li;
	li.LowPart = attrData.nFileSizeLow;
	li.HighPart = attrData.nFileSizeHigh;
	return (u64)li.QuadPart;
}

std::time_t Win32GetLastModifiedTime(const WString& path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if(GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &fad) == 0)
		Win32HandleError(GetLastError(), path);

	ULARGE_INTEGER ull;
	ull.LowPart = fad.ftLastWriteTime.dwLowDateTime;
	ull.HighPart = fad.ftLastWriteTime.dwHighDateTime;

	return (std::time_t)((ull.QuadPart / 10000000ULL) - 11644473600ULL);
}

bool FileSystem::RemoveFile(const Path& path)
{
	WString pathStr = UTF8::ToWide(path.ToString());
	if(Win32IsDirectory(pathStr))
	{
		if(RemoveDirectoryW(pathStr.c_str()) == 0)
		{
			Win32HandleError(GetLastError(), pathStr);
			return false;
		}
	}
	else
	{
		if(DeleteFileW(pathStr.c_str()) == 0)
		{
			Win32HandleError(GetLastError(), pathStr);
			return false;
		}
	}

	return true;
}

bool FileSystem::CopyFile(const Path& from, const Path& to)
{
	WString fromStr = UTF8::ToWide(from.ToString());
	WString toStr = UTF8::ToWide(to.ToString());

	if(CopyFileW(fromStr.c_str(), toStr.c_str(), FALSE) == FALSE)
	{
		Win32HandleError(GetLastError(), fromStr);
		return false;
	}

	return true;
}

bool FileSystem::MoveFile(const Path& oldPath, const Path& newPath)
{
	WString oldPathStr = UTF8::ToWide(oldPath.ToString());
	WString newPathStr = UTF8::ToWide(newPath.ToString());

	if(MoveFileW(oldPathStr.c_str(), newPathStr.c_str()) == 0)
	{
		Win32HandleError(GetLastError(), oldPathStr);
		return false;
	}

	return true;
}

TShared<DataStream> FileSystem::OpenFile(const Path& fullPath, bool readOnly)
{
	WString pathWString = UTF8::ToWide(fullPath.ToString());
	const wchar_t* pathString = pathWString.c_str();

	if(!Win32PathExists(pathString) || !Win32IsFile(pathString))
	{
		B3D_LOG(Warning, LogPlatform, "Failed to open file at path '{0}'. File doesn't exist.", fullPath);
		return nullptr;
	}

	DataStream::AccessMode accessMode = DataStream::READ;
	if(!readOnly)
		accessMode = (DataStream::AccessMode)(accessMode | (u32)DataStream::WRITE);

	TShared<FileDataStream> fileDataStream = B3DMakeShared<FileDataStream>(fullPath, accessMode);
	if(!fileDataStream->Open())
	{
		B3D_LOG(Warning, LogPlatform, "Failed to open file at path '{0}'. File stream failed to open.", fullPath);
		return nullptr;
	}

	return fileDataStream;
}

TShared<DataStream> FileSystem::CreateAndOpenFile(const Path& fullPath)
{
	TShared<FileDataStream> fileDataStream = B3DMakeShared<FileDataStream>(fullPath, DataStream::AccessMode::WRITE);
	if(!fileDataStream->Open())
	{
		B3D_LOG(Warning, LogPlatform, "Failed to create file at path '{0}'. File stream failed to open.", fullPath);
		return nullptr;
	}

	return fileDataStream;
}

u64 FileSystem::GetFileSize(const Path& fullPath)
{
	return Win32GetFileSize(UTF8::ToWide(fullPath.ToString()));
}

bool FileSystem::Exists(const Path& fullPath)
{
	return Win32PathExists(UTF8::ToWide(fullPath.ToString()));
}

bool FileSystem::IsFile(const Path& fullPath)
{
	WString pathStr = UTF8::ToWide(fullPath.ToString());

	return Win32PathExists(pathStr) && Win32IsFile(pathStr);
}

bool FileSystem::IsFolder(const Path& fullPath)
{
	WString pathStr = UTF8::ToWide(fullPath.ToString());

	return Win32PathExists(pathStr) && Win32IsDirectory(pathStr);
}

bool FileSystem::CreateFolder(const Path& fullPath)
{
	Path parentPath = fullPath;
	while(!Exists(parentPath) && parentPath.GetDirectoryCount() > 0)
	{
		parentPath = parentPath.GetParent();
	}

	for(u32 directoryIndex = parentPath.GetDirectoryCount(); directoryIndex < fullPath.GetDirectoryCount(); directoryIndex++)
	{
		parentPath.Append(fullPath[directoryIndex]);
		if(!Win32CreateDirectory(UTF8::ToWide(parentPath.ToString())))
			return false;
	}

	if(fullPath.IsFile())
	{
		if(!Win32CreateDirectory(UTF8::ToWide(fullPath.ToString())))
			return false;
	}

	return true;
}

void FileSystem::GetChildren(const Path& dirPath, Vector<Path>& files, Vector<Path>& directories)
{
	WString findPath = UTF8::ToWide(dirPath.ToString());

	if(Win32IsFile(findPath))
		return;

	if(dirPath.IsFile()) // Assuming the file is a folder, just improperly formatted in Path
		findPath.append(L"\\*");
	else
		findPath.append(L"*");

	WIN32_FIND_DATAW findData;
	HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		Win32HandleError(GetLastError(), findPath);
		return;
	}

	WString tempName;
	do
	{
		tempName = findData.cFileName;

		if(tempName != L"." && tempName != L"..")
		{
			Path fullPath = dirPath;
			if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				directories.push_back(fullPath.Append(UTF8::FromWide(tempName) + "/"));
			else
				files.push_back(fullPath.Append(UTF8::FromWide(tempName)));
		}

		if(FindNextFileW(fileHandle, &findData) == FALSE)
		{
			if(GetLastError() != ERROR_NO_MORE_FILES)
				Win32HandleError(GetLastError(), findPath);

			break;
		}
	}
	while(true);

	FindClose(fileHandle);
}

bool FileSystem::Iterate(const Path& dirPath, std::function<bool(const Path&)> fileCallback, std::function<bool(const Path&)> dirCallback, bool recursive)
{
	WString findPath = UTF8::ToWide(dirPath.ToString());

	if(IsFile(dirPath) || !Exists(dirPath))
		return false;

	if(dirPath.IsFile()) // Assuming the file is a folder, just improperly formatted in Path
		findPath.append(L"\\*");
	else
		findPath.append(L"*");

	WIN32_FIND_DATAW findData;
	HANDLE fileHandle = FindFirstFileW(findPath.c_str(), &findData);
	if(fileHandle == INVALID_HANDLE_VALUE)
	{
		Win32HandleError(GetLastError(), findPath);
		return false;
	}

	WString tempName;
	do
	{
		tempName = findData.cFileName;

		if(tempName != L"." && tempName != L"..")
		{
			Path fullPath = dirPath;
			if((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				Path childDir = fullPath.Append(UTF8::FromWide(tempName) + u8"/");
				if(dirCallback != nullptr)
				{
					if(!dirCallback(childDir))
					{
						FindClose(fileHandle);
						return false;
					}
				}

				if(recursive)
				{
					if(!Iterate(childDir, fileCallback, dirCallback, recursive))
					{
						FindClose(fileHandle);
						return false;
					}
				}
			}
			else
			{
				Path filePath = fullPath.Append(UTF8::FromWide(tempName));
				if(fileCallback != nullptr)
				{
					if(!fileCallback(filePath))
					{
						FindClose(fileHandle);
						return false;
					}
				}
			}
		}

		if(FindNextFileW(fileHandle, &findData) == FALSE)
		{
			if(GetLastError() != ERROR_NO_MORE_FILES)
				Win32HandleError(GetLastError(), findPath);

			break;
		}
	}
	while(true);

	FindClose(fileHandle);
	return true;
}

std::time_t FileSystem::GetLastModifiedTime(const Path& fullPath)
{
	return Win32GetLastModifiedTime(UTF8::ToWide(fullPath.ToString()));
}

Path FileSystem::GetExecutableFolderPath()
{
	wchar_t path[MAX_PATH];
	DWORD characterCount = GetModuleFileNameW(nullptr, path, (DWORD)B3DSize(path));
	if(characterCount == 0)
	{
		B3D_LOG(Error, LogFileSystem, "Internal error. Failed to retrieve current module path.");
		return Path::kBlank;
	}

	Path pathToModule(UTF8::FromWide(path));
	pathToModule = pathToModule.GetParent();

	return pathToModule;
}

Path FileSystem::GetWorkingDirectoryPath()
{
	wchar_t path[MAX_PATH];
	DWORD length = GetCurrentDirectoryW(MAX_PATH, path);
	if(length == 0)
	{
		B3D_LOG(Error, LogFileSystem, "Internal error. Failed to retrieve current working directory.");
		return Path::kBlank;
	}

	return Path(UTF8::FromWide(path));
}

Path FileSystem::GetTemporaryFolderPath()
{
	const String utf8dir = UTF8::FromWide(Win32GetTempDirectory());
	return Path(utf8dir);
}

Path FileSystem::GetUniqueTemporaryFilePath()
{
	Path output = GetTemporaryFolderPath();
	output.SetFilename(UUIDGenerator::GenerateRandom().ToString());

	while(Exists(output))
		output.SetFilename(UUIDGenerator::GenerateRandom().ToString());

	return output;
}

Path FileSystem::GetApplicationDataFolder()
{
	wchar_t appDataPath[MAX_PATH];

	if(SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appDataPath)))
	{
		return Path(UTF8::FromWide(appDataPath)) + "Banshee3D";
	}

	// Fallback
	return GetExecutableFolderPath();
}

