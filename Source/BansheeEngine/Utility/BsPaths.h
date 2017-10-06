//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

namespace bs
{
	/** @addtogroup Utility-Engine-Internal
	 *  @{
	 */

	constexpr const char* ENGINE_ASSEMBLY = "MBansheeEngine";
	constexpr const char* SCRIPT_GAME_ASSEMBLY = "MScriptGame";
	constexpr const char* GAME_RESOURCES_FOLDER_NAME = "Resources/";
	constexpr const char* GAME_SETTINGS_NAME = "GameSettings.asset";
	constexpr const char* GAME_RESOURCE_MANIFEST_NAME = "ResourceManifest.asset";
	constexpr const char* GAME_RESOURCE_MAPPING_NAME = "ResourceMapping.asset";


	/** Contains common engine paths. */
	class BS_EXPORT Paths
	{
	public:
		/**	Returns the absolute path where the managed release assemblies are located. */
		static const Path& getReleaseAssemblyPath();

		/**	Returns the absolute path where the managed debug assemblies are located. */
		static const Path& getDebugAssemblyPath();

		/**	Returns the absolute path where the builtin assets are located. */
		static const Path& getRuntimeDataPath();

		/**	Returns the absolute path where the builtin engine-specific assets are located. */
		static const Path& getEngineDataPath();

		/**	Returns the absolute path to the game settings file used by editor-built executables. */
		static const Path& getGameSettingsPath();

		/**	Returns the absolute path to the game resources folder used by editor-built executables. */
		static const Path& getGameResourcesPath();

		/** Returns the absolute path where the engine binaries are located in. */
		static const Path& getBinariesPath();

		/**
		 * Searches common locations for a specified path by querying if the file/directory exists and returns the found 
		 * path.
		 * 			
		 * @param[in]	path	Relative path to search for (for example "Data\").				
		 * @return				Path at which the relative path was found at. This path will be relative to the working 
		 *						directory.
		 */
		static Path findPath(const Path& path);

		static const Path RELEASE_ASSEMBLY_PATH;
		static const Path DEBUG_ASSEMBLY_PATH;
		static const Path RUNTIME_DATA_PATH;
		static const Path ENGINE_DATA_PATH;
	};

	/** @} */
}