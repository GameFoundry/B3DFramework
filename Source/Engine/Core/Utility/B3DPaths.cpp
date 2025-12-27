//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DPrerequisites.h"
#include "B3DEngineConfig.h"
#include "FileSystem/B3DFileSystem.h"
#include "Utility/B3DDynamicLibrary.h"

using namespace b3d;

const Path Paths::kReleaseAssemblyPath = "bin/Assemblies/Release/";
const Path Paths::kDebugAssemblyPath = "bin/Assemblies/Debug/";

const Path Paths::kFrameworkDataPath = "Data/";

#if BS_INCLUDE_B3D_PATHS
const Path Paths::kEditorDataPath = "EditorData/";
#endif

const Path& Paths::GetDataPath()
{
	static bool initialized = false;
	static Path path;

	if(!initialized)
	{
		if(FileSystem::Exists(kFrameworkDataPath))
			path = FileSystem::GetWorkingDirectoryPath() + kFrameworkDataPath;
		else
#if B3D_IS_ENGINE
			path = Path(kRawAppRoot) + Path("Framework") + kFrameworkDataPath;
#else
			path = Path(kRawAppRoot) + kFrameworkDataPath;
#endif

		initialized = true;
	}

	return path;
}

const Path& Paths::GetBinariesPath()
{
	static bool initialized = false;
	static Path path;

	if(!initialized)
	{
		path = FileSystem::GetWorkingDirectoryPath();

		// Look for bsf library to find the right path
		Path anchorFile = path;
		anchorFile.SetFilename("bsf." + String(DynamicLibrary::kExtension));

		if(!FileSystem::Exists(anchorFile))
		{
			path = kBinariesPath;
			if(!FileSystem::Exists(path))
				path = ""; // No path found, keep the default
		}

		initialized = true;
	}

	return path;
}

#if BS_INCLUDE_B3D_PATHS
const Path& Paths::GetEditorDataPath()
{
	static bool initialized = false;
	static Path path;

	if(!initialized)
	{
#	ifdef BS_IS_ASSET_TOOL
		// Asset tool always runs relative to the 'bsf' directory
		Path editorDataPath = Path("../../") + FRAMEWORK_DATA_PATH;

		if(FileSystem::Exists(editorDataPath))
			path = FileSystem::GetWorkingDirectoryPath() + editorDataPath;
#	else
		// Otherwise, look for the folder in the direct descendant of the working directory
		if(FileSystem::Exists(kEditorDataPath))
			path = FileSystem::GetWorkingDirectoryPath() + kEditorDataPath;
#	endif
		// Then check the source distribution itself, in case we're running directly from the build directory
		else
		{
			path = Path(kRawAppRoot) + kFrameworkDataPath;

			if(!FileSystem::Exists(path))
				B3D_LOG(Error, FileSystem, "Cannot find builtin assets for the editor at path '{0}'.", path);
		}

		initialized = true;
	}

	return path;
}

const Path& Paths::GetGameSettingsPath()
{
	static Path path = FindPath(kGameSettingsName);
	return path;
}

const Path& Paths::GetGameResourcesPath()
{
	static Path path = FindPath(kGameResourcesFolderName);
	return path;
}
#endif

Path Paths::FindPath(const Path& path)
{
	// Note: These paths should be searched for during start-up and cached

	// First, look for the direct descendant of the working directory
	Path output = path;
	if(FileSystem::Exists(path))
	{
		output.MakeAbsolute(FileSystem::GetWorkingDirectoryPath());
		return output;
	}

	// Then, check the build directory itself, in case we're running directly from it (during development)
	output.MakeAbsolute(kBuildAppRoot);
	if(FileSystem::Exists(output))
		return output;

	// No path found, but return the initial value by default
	return path;
}
