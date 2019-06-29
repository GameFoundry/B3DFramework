//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsPrerequisites.h"
#include "BsEngineConfig.h"
#include "FileSystem/BsFileSystem.h"
#include "Utility/BsDynLib.h"

namespace bs
{
	const Path Paths::RELEASE_ASSEMBLY_PATH = "bin/Assemblies/Release/";
	const Path Paths::DEBUG_ASSEMBLY_PATH = "bin/Assemblies/Debug/";

	const Path Paths::FRAMEWORK_DATA_PATH = "Data/";

#if BS_INCLUDE_B3D_PATHS
	const Path Paths::EDITOR_DATA_PATH = "EditorData/";
#endif

	const Path& Paths::getDataPath()
	{
		static bool initialized = false;
		static Path path;

		if(!initialized)
		{
			if (FileSystem::exists(FRAMEWORK_DATA_PATH))
				path = FileSystem::getWorkingDirectoryPath() + FRAMEWORK_DATA_PATH;
			else
#if BS_IS_BANSHEE3D
				path = Path(RAW_APP_ROOT) + Path("Source/bsf") + FRAMEWORK_DATA_PATH;
#else
				path = Path(RAW_APP_ROOT) + FRAMEWORK_DATA_PATH;
#endif

			initialized = true;
		}

		return path;
	}

	const Path& Paths::getBinariesPath()
	{
		static bool initialized = false;
		static Path path;
		
		if(!initialized)
		{
			path = FileSystem::getWorkingDirectoryPath();
			
			// Look for bsf library to find the right path
			Path anchorFile = path;
			anchorFile.setFilename("bsf." + String(DynLib::EXTENSION));

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

#if BS_INCLUDE_B3D_PATHS
	const Path& Paths::getEditorDataPath()
	{
		static bool initialized = false;
		static Path path;

		if(!initialized)
		{
#ifdef BS_IS_ASSET_TOOL
			// Asset tool always runs relative to the 'bsf' directory
			Path editorDataPath = Path("../../") + FRAMEWORK_DATA_PATH;

			if (FileSystem::exists(editorDataPath))
				path = FileSystem::getWorkingDirectoryPath() + editorDataPath;
#else
			// Otherwise, look for the folder in the direct descendant of the working directory 
			if (FileSystem::exists(EDITOR_DATA_PATH))
				path = FileSystem::getWorkingDirectoryPath() + EDITOR_DATA_PATH;
#endif
			// Then check the source distribution itself, in case we're running directly from the build directory
			else
			{
				path = Path(RAW_APP_ROOT) + FRAMEWORK_DATA_PATH;

				if (!FileSystem::exists(path))
					BS_LOG(Error, FileSystem, "Cannot find builtin assets for the editor at path '{0}'.", path);
			}

			initialized = true;
		}

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
#endif

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

		// Then, check the build directory itself, in case we're running directly from it (during development)
		output.makeAbsolute(BUILD_APP_ROOT);
		if (FileSystem::exists(output))
			return output;

		// No path found, but return the initial value by default
		return path;
	}
}