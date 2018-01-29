//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPrerequisites.h"
#include "BsEngineConfig.h"
#include "FileSystem/BsFileSystem.h"
#include "Utility/BsDynLib.h"

namespace bs
{
	const Path Paths::RELEASE_ASSEMBLY_PATH = "bin/Assemblies/Release/";
	const Path Paths::DEBUG_ASSEMBLY_PATH = "bin/Assemblies/Debug/";
	const Path Paths::RUNTIME_DATA_PATH = "Data/";
	const Path Paths::ENGINE_DATA_PATH = RUNTIME_DATA_PATH + "Engine/";


	const Path& Paths::getReleaseAssemblyPath()
	{
		static Path path = findPath(RELEASE_ASSEMBLY_PATH);
		return path;
	}

	const Path& Paths::getDebugAssemblyPath()
	{
		static Path path = findPath(DEBUG_ASSEMBLY_PATH);
		return path;
	}

	const Path& Paths::getRuntimeDataPath()
	{
		static Path path = findPath(RUNTIME_DATA_PATH);
		return path;
	}

	const Path& Paths::getEngineDataPath()
	{
		static Path path = findPath(ENGINE_DATA_PATH);
		return path;
	}

	const Path& Paths::getGameSettingsPath()
	{
		static Path path = findPath(GAME_SETTINGS_NAME);
		return path;
	}

	const Path& Paths::getGameResourcesPath()
	{
		static Path path = findPath(GAME_RESOURCES_FOLDER_NAME);
		return path;
	}

	const Path& Paths::getBinariesPath()
	{
		static bool initialized = false;
		static Path path;
		
		if(!initialized)
		{
			path = FileSystem::getWorkingDirectoryPath();
			
			// Look for BansheeEngine library to find the right path
			Path anchorFile = path;
			anchorFile.setFilename("BansheeEngine" + String(DynLib::EXTENSION));

			if (!FileSystem::exists(anchorFile))
			{
				path = BINARIES_PATH;
				if (!FileSystem::exists(path))
					path = ""; // No path found, keep the default
			}

			initialized = true;
		}

		return path;
	}

	Path Paths::findPath(const Path& path)
	{
		// Note: These paths should be searched for during start-up and cached

		// First, look for the direct descendant of the working directory 
		Path output = path;
		if (FileSystem::exists(path))
		{
			output.makeAbsolute(FileSystem::getWorkingDirectoryPath());
			return output;
		}

		// Finally, check the source distribution itself, in case we're running directly from the build directory
		output.makeAbsolute(RAW_APP_ROOT);
		if (FileSystem::exists(output))
			return output;

		// No path found, but return the initial value by default
		return path;
	}
}