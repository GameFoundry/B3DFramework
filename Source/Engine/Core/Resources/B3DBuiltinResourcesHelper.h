//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "ThirdParty/json.hpp"

namespace b3d
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**	Provides various methods commonly used for managing builtin resources. */
	class B3D_EXPORT BuiltinResourcesHelper
	{
	public:
		/** Flags that can control asset import. */
		enum class AssetType
		{
			/** No flags, just import asset as normal. Each entry is expected to have an "UUID" and a "Path" field. */
			Normal,
			/**
			 * Assumes imported assets are textures. Will generate sprite assets for each imported texture. Expects
			 * "TextureUUID", "Path" and "SpriteUUID" fields present in per-entry JSON.
			 */
			Sprite,
		};

		/**
		 * Iterates over all entires in the provided json file, imports the files linked by the entries them and stores them
		 * in the corresponding folder. Also registers the imported files in the provided manifest.
		 *
		 * @param[in]	entries			JSON array containing the entries to parse, with each entry containing
		 *								data determine by set ImportMode.
		 * @param[in]	importFlags		A set of import flags (one for each entry) that specify which entries need to be
		 *								imported.
		 * @param[in]	inputFolder		Folder in which to look for the input files.
		 * @param[in]	outputFolder	Folder in which to store the imported resources.
		 * @param[in]	mode			Mode that controls how are files imported.
		 * @param[in]	compress		True if the imported asset should be compressed when saved to the disk.
		 * @param[in]	mipmap			True if mipmaps should be generated.
		 */
		static void ImportAssets(const nlohmann::json& entries, const Vector<bool>& importFlags, const Path& inputFolder, const Path& outputFolder, AssetType mode = AssetType::Normal, bool compress = false, bool mipmap = false);

		/**
		 * Imports a font from the specified file. Imported font assets are saved in the output folder. All saved resources
		 * are registered in the provided resource manifest.
		 */
		static void ImportFont(const Path& inputFile, const String& outputName, const Path& outputFolder, bool antialiasing, const UUID& UUID);

		/**
		 * Iterates over all the provided entries and generates a list of flags that determine should the asset be imported
		 * or not. This is done by comparing file modification dates with the last update time and/or checking if any
		 * dependencies require import.
		 *
		 * @param[in]	entries				JSON array containing entries to iterate over.
		 * @param[in]	inputFolder			Folder in which to look for the input files.
		 * @param[in]	lastUpdateTime		Timestamp of when the last asset import occurred.
		 * @param[in]	forceImport			If true, all entries will be marked for import.
		 * @param[in]	dependencies		Optional map of entries that map each entry in the @p entries array, to a list
		 *									of dependencies. The dependencies will then also be checked for modifications
		 *									and if modified the entry will be marked for reimport.
		 * @param[in]	dependencyFolder	Folder in which dependeny files reside. Only relevant if @p dependencies is
		 *									provided.
		 * @return							An array of the same size as the @p entries array, containing value true if
		 *									an asset should be imported, or false otherwise.
		 */
		static Vector<bool> GenerateImportFlags(const nlohmann::json& entries, const Path& inputFolder, time_t lastUpdateTime, bool forceImport, const nlohmann::json* dependencies = nullptr, const Path& dependencyFolder = Path::kBlank);

		/**
		 * Scans the provided folder for any files that are currently not part of the provided JSON entries. If some are
		 * found they are appended to the JSON entry array. Returns true if any new files were found, false otherwise.
		 *
		 * @param[in]		folder		Folder to check for new entries.
		 * @param[in]		type		Type of entries in the folder. Determines the type of JSON data generated.
		 * @param[in, out]	entries		Current data file entries.
		 */
		static bool UpdateJson(const Path& folder, AssetType type, nlohmann::json& entries);

		/** Writes a timestamp with the current date and time in the specified file. */
		static void WriteTimestamp(const Path& file);

		/**
		 * Checks all files in the specified folder for modifications compared to the time stored in the timestamp file.
		 * Timestamp file must have been saved using writeTimestamp(). Returns 0 if no changes, 1 if timestamp is out date,
		 * or 2 if timestamp doesn't exist. @p lastUpdateTime will contain the time stored in the timestamp, if it exist.
		 */
		static u32 CheckForModifications(const Path& folder, const Path& timeStampFile, time_t& lastUpdateTime);
	};

	/** @} */
} // namespace b3d
