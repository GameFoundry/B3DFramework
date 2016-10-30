//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "ThirdParty/json.hpp"

namespace BansheeEngine
{
	/** @addtogroup Resources-Engine-Internal
	 *  @{
	 */

	/**	Provides various methods commonly used for managing builtin resources. */
	class BS_EXPORT BuiltinResourcesHelper
	{
	public:
		/** Flags that can control asset import. */
		enum class ImportMode
		{
			/** No flags, just import asset as normal. Each entry is expected to have an "UUID" and a "Path" field. */
			None,
			/** 
			 * Assumes imported assets are textures. Will generate sprite assets for each imported texture. Expects 
			 * "UUID", "Path" and "SpriteUUID" fields present in per-entry JSON.
			 */
			Sprite,
		};
		
		/** 
		 * Iterates over all entires in the provided json file, imports the files linked by the entries them and stores them 
		 * in the corresponding folder. Also registers the imported files in the provided manifest.
		 * 
		 * @param[in]	entries			JSON array containing the entries to parse, with each entry containing
		 *								data determine by set ImportMode. 
		 * @param[in]	inputFolder		Folder in which to look for the input files.
		 * @param[in]	outputFolder	Folder in which to store the imported resources.
		 * @param[in]	manifest		Manifest in which to register the imported resources in.
		 * @param[in]	mode			Mode that controls how are files imported.
		 */
		static void importAssets(const nlohmann::json& entries, const Path& inputFolder, const Path& outputFolder, 
			const SPtr<ResourceManifest>& manifest, ImportMode mode = ImportMode::None);

		/**
		 * Imports a font from the specified file. Imported font assets are saved in the output folder. All saved resources
		 * are registered in the provided resource manifest.
		 */
		static void importFont(const Path& inputFile, const WString& outputName, const Path& outputFolder, 
			const Vector<UINT32>& fontSizes, bool antialiasing, const String& UUID, const SPtr<ResourceManifest>& manifest);

		/** Writes a timestamp with the current date and time in the specified file. */
		static void writeTimestamp(const Path& file);

		/**
		 * Checks all files in the specified folder for modifications compared to the time stored in the timestamp file. 
		 * Timestamp file must have been saved using writeTimestamp().
		 */
		static bool checkForModifications(const Path& folder, const Path& timeStampFile);
	};

	/** @} */
}