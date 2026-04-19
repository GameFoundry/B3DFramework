//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Platform
	 *  @{
	 */

	constexpr const char* kGameResourcesVirtualPathPrefix = "/Game/";

#if B3D_IS_ENGINE
	constexpr const char* kGameResourcesFolderName = "Resources/";
	constexpr const char* kGameResourcePackageName = "GameResources";
	constexpr const char* kGameSettingsName = "GameSettings.asset";
#endif

	/** Contains common engine paths and utility method for searching for paths. */
	class B3D_EXPORT Paths
	{
	public:
		/**	Returns the absolute path where the builtin framework-specific assets are located. */
		static const Path& GetDataPath();

		/** Returns the absolute path where the engine binaries are located in. */
		static const Path& GetBinariesPath();

#if B3D_WITH_EDITOR
		/**	Returns the absolute path where the builtin editor-specific assets are located. */
		static const Path& GetEditorDataPath();
#endif

#if B3D_IS_ENGINE
		/**	Returns the absolute path to the game settings file used by editor-built executables. */
		static const Path& GetGameSettingsPath();

		/**	Returns the absolute path to the game resources folder used by editor-built executables. */
		static const Path& GetGameResourcesPath();
#endif

		/**
		 * Searches common locations for a specified path by querying if the file/directory exists and returns the found
		 * path.
		 *
		 * @param[in]	path	Relative path to search for (for example "Data\").
		 * @return				Path at which the relative path was found at. This path will be relative to the working
		 *						directory.
		 */
		static Path FindPath(const Path& path);

		/** Path to the root data directory. Relative to working directory, or RAW_APP_ROOT. */
		static const Path kFrameworkDataPath;

		/** Path where the release configuration managed assemblies are located at, relative to the working directory. */
		static const Path kReleaseAssemblyPath;

		/** Path where the debug configuration managed assemblies are located at, relative to the working directory. */
		static const Path kDebugAssemblyPath;

#if B3D_WITH_EDITOR 
		/** Path to the root editor data directory. Relative to working directory, or RAW_APP_ROOT. */
		static const Path kEditorDataPath;
#endif
	};

	/** @} */
} // namespace b3d
