//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace bs
{
	/** @addtogroup Utility-Engine-Internal
	 *  @{
	 */

#if BS_IS_BANSHEE3D
	constexpr const char* ENGINE_ASSEMBLY = "MBansheeEngine";
	constexpr const char* SCRIPT_GAME_ASSEMBLY = "MScriptGame";
	constexpr const char* GAME_RESOURCES_FOLDER_NAME = "Resources/";
	constexpr const char* GAME_SETTINGS_NAME = "GameSettings.asset";
	constexpr const char* GAME_RESOURCE_MANIFEST_NAME = "ResourceManifest.asset";
	constexpr const char* GAME_RESOURCE_MAPPING_NAME = "ResourceMapping.asset";
#endif

	/** Contains common engine paths and utility method for searching for paths. */
	class BS_EXPORT Paths
	{
	public:	
		/**	Returns the absolute path where the builtin framework-specific assets are located. */
		static const Path& getDataPath();

		/** Returns the absolute path where the engine binaries are located in. */
		static const Path& getBinariesPath();

#if BS_IS_BANSHEE3D
		/**	Returns the absolute path where the managed release assemblies are located. */
		static const Path& getReleaseAssemblyPath();

		/**	Returns the absolute path where the managed debug assemblies are located. */
		static const Path& getDebugAssemblyPath();

		/**	Returns the absolute path to the game settings file used by editor-built executables. */
		static const Path& getGameSettingsPath();

		/**	Returns the absolute path to the game resources folder used by editor-built executables. */
		static const Path& getGameResourcesPath();
#endif
		
		/**
		 * Searches common locations for a specified path by querying if the file/directory exists and returns the found 
		 * path.
		 * 			
		 * @param[in]	path	Relative path to search for (for example "Data\").
		 * @return				Path at which the relative path was found at. This path will be relative to the working 
		 *						directory.
		 */
		static Path findPath(const Path& path);

		/** Path to the root data directory. Relative to working directory, or RAW_APP_ROOT. */
		static const Path FRAMEWORK_DATA_PATH;
		
#if BS_IS_BANSHEE3D
		/** Path where the release configuration managed assemblies are located at, relative to the working directory. */
		static const Path RELEASE_ASSEMBLY_PATH;

		/** Path where the debug configuration managed assemblies are located at, relative to the working directory. */
		static const Path DEBUG_ASSEMBLY_PATH;;
#endif
	};

	/** @} */
}