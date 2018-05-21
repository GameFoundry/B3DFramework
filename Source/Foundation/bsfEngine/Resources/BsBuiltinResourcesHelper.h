//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "ThirdParty/json.hpp"
#include "GUI/BsGUIElementStyle.h"

namespace bs
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	class GUIElementStyleLoader;

	/**	Provides various methods commonly used for managing builtin resources. */
	class BS_EXPORT BuiltinResourcesHelper
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
		 * @param[in]	manifest		Manifest in which to register the imported resources in.
		 * @param[in]	mode			Mode that controls how are files imported.
		 * @param[in]	dependencies	Optional map that be updated with any dependencies the imported assets depend on.
		 * @param[in]	compress		True if the imported asset should be compressed when saved to the disk.
		 */
		static void importAssets(const nlohmann::json& entries, const Vector<bool>& importFlags, const Path& inputFolder, 
			const Path& outputFolder, const SPtr<ResourceManifest>& manifest, AssetType mode = AssetType::Normal,
			nlohmann::json* dependencies = nullptr, bool compress = false);

		/**
		 * Imports a font from the specified file. Imported font assets are saved in the output folder. All saved resources
		 * are registered in the provided resource manifest.
		 */
		static void importFont(const Path& inputFile, const String& outputName, const Path& outputFolder, 
			const Vector<UINT32>& fontSizes, bool antialiasing, const UUID& UUID, const SPtr<ResourceManifest>& manifest);

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
		static Vector<bool> generateImportFlags(const nlohmann::json& entries, const Path& inputFolder,
			time_t lastUpdateTime, bool forceImport, const nlohmann::json* dependencies = nullptr, 
			const Path& dependencyFolder = Path::BLANK);

		/** 
		 * Scans the provided folder for any files that are currently not part of the provided JSON entries. If some are
		 * found they are appended to the JSON entry array. Returns true if any new files were found, false otherwise.
		 * 
		 * @param[in]		folder		Folder to check for new entries.
		 * @param[in]		type		Type of entries in the folder. Determines the type of JSON data generated.
		 * @param[in, out]	entries		Current data file entries.
		 */
		static bool updateJSON(const Path& folder, AssetType type, nlohmann::json& entries);

		/** 
		 * Updates the resource manifest from the UUID's and paths provided in the JSON. 
		 * 
		 * @param[in]		folder		Folder containing the imported assets the manifest will point to.
		 * @param[in]		entries		JSON entries detailing each asset.
		 * @param[in]		manifest	Manifest in which to register the assets in.
		 * @param[in]		type		Type of assets we're registering.
		 */
		static void updateManifest(const Path& folder, const nlohmann::json& entries, 
			const SPtr<ResourceManifest>& manifest, AssetType type);

		/** Writes a timestamp with the current date and time in the specified file. */
		static void writeTimestamp(const Path& file);

		/**
		 * Checks all files in the specified folder for modifications compared to the time stored in the timestamp file. 
		 * Timestamp file must have been saved using writeTimestamp(). Returns 0 if no changes, 1 if timestamp is out date,
		 * or 2 if timestamp doesn't exist. @p lastUpdateTime will contain the time stored in the timestamp, if it exist.
		 */
		static UINT32 checkForModifications(const Path& folder, const Path& timeStampFile, time_t& lastUpdateTime);

		/** Checks if the shader compiled properly and reports the problem if it hasn't. Returns true if shader is valid. */
		static bool verifyAndReportShader(const HShader& shader);

		/** Loads the shader at the specified path, updates its bytecode if required, and re-saves the shader file. */
		static void updateShaderBytecode(const Path& path);

		/** Constructs a GUIElementStyle from the provided JSON entry. */
		static GUIElementStyle loadGUIStyleFromJSON(const nlohmann::json& entry, const GUIElementStyleLoader& loader);
	};

	/** 
	 * Determines how are resources for GUIElementStyle loaded, when it is being decoded from a non-binary format that only
	 * stores resource names.
	 */
	class GUIElementStyleLoader
	{
	public:
		virtual ~GUIElementStyleLoader() = default;

		/** Loads a font with the specified name. */
		virtual HFont loadFont(const String& name) const = 0;

		/** Loads a sprite texture with the specified name. */
		virtual HSpriteTexture loadTexture(const String& name) const = 0;
	};

	/** Handles loading of GUIELementStyle resources by retrieving them from the builtin resources folder. */
	class BuiltinResourceGUIElementStyleLoader final : public GUIElementStyleLoader
	{
	public:
		BuiltinResourceGUIElementStyleLoader(const Path& fontPath, const Path& texturePath);

		/** Loads a font with the specified name. */
		HFont loadFont(const String& name) const override;

		/** Loads a sprite texture with the specified name. */
		HSpriteTexture loadTexture(const String& name) const override;

	private:
		Path mFontPath;
		Path mTexturePath;
	};
	/** @} */
}
