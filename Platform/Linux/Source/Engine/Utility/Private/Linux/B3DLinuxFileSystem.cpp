//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "FileSystem/B3DFileSystem.h"

#include "Private/Linux/B3DLinuxFileDataStream.h"
#include "Debug/B3DDebug.h"

using namespace b3d;

TShared<DataStream> FileSystem::CreateFileStream(const Path& path, FileAccessFlags access)
{
	TShared<LinuxFileDataStream> stream = B3DMakeShared<LinuxFileDataStream>(path, access);
	if(!stream->Open())
	{
		B3D_LOG(Warning, LogFileSystem, "Failed to open file at path '{0}'. File stream failed to open.", path);
		return nullptr;
	}

	return stream;
}

void FileSystem::PlatformStartUp()
{
	// Bring up the shared io_uring instance
	LinuxFileIOManager::StartUp();
}

void FileSystem::PlatformShutDown()
{
	LinuxFileIOManager::ShutDown();
}
