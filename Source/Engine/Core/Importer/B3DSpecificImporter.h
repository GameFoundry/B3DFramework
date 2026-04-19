//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Importer-Internal
	 *  @{
	 */

	/**
	 * Contains a resource that was imported from a file that contains multiple resources (for example an animation from an
	 * FBX file).
	 */
	struct SubResourceRaw
	{
		String Name; /**< Unique name of the sub-resource. */
		SPtr<Resource> Value; /**< Contents of the sub-resource. */

		static constexpr const char* kPrimaryResourceName = "primary";
	};

	/** Modes signififying the level of asynchronous functionality provided by a SpecificImporter. */
	enum class ImporterAsyncMode
	{
		/** Asynchronous import is supported but only on a single thread. */
		Single,
		/** Asynchronous import for multiple simultaneous threads is supported. */
		Multi
	};

	/**
	 * Abstract class that is to be specialized for converting a certain asset type into an engine usable resource
	 * (for example a .png file into an engine usable texture).
	 *
	 * On initialization this class must register itself with the Importer module, which delegates asset import calls to a
	 * specific importer.
	 */
	class B3D_EXPORT SpecificImporter
	{
	public:
		SpecificImporter() {}

		virtual ~SpecificImporter() {}

		/**
		 * Check is the provided extension supported by this importer.
		 *
		 * @note	Provided extension should be without the leading dot.
		 */
		virtual bool IsExtensionSupported(const String& extension) const = 0;

		/** Check if the provided magic number is supported by this importer. */
		virtual bool IsMagicNumberSupported(const u8* magicNumber, u32 magicNumberSize) const = 0;

		/** Returns the level of asynchronous import supported by this importer. */
		virtual ImporterAsyncMode GetAsyncMode() const { return ImporterAsyncMode::Multi; }

		/**
		 * Imports the given file. If file contains more than one resource only the primary resource is imported (for
		 * example for an FBX a mesh would be imported, but animations ignored).
		 *
		 * @param	filePath			Pathname of the file, with file extension.
		 * @param	importOptions		Options that can control how is the resource imported.
		 * @return						null if it fails, otherwise the loaded object.
		 */
		virtual SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions) = 0;

		/**
		 * Imports the given file. This method returns all imported resources, which is relevant for files that can contain
		 * multiple resources (for example an FBX which may contain both a mesh and animations).
		 *
		 * @param	filePath			Pathname of the file, with file extension.
		 * @param	importOptions		Options that can control how are the resources imported.
		 * @return						Empty array if it fails, otherwise the loaded objects. First element is always the
		 *								primary resource.
		 */
		virtual Vector<SubResourceRaw> ImportAll(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/**
		 * Creates import options specific for this importer. Import options are provided when calling import() in order
		 * to customize the import, and provide additional information.
		 */
		virtual SPtr<ImportOptions> CreateImportOptions() const;

		/**
		 * Gets the default import options.
		 *
		 * @return	The default import options.
		 */
		SPtr<const ImportOptions> GetDefaultImportOptions() const;

	private:
		mutable SPtr<const ImportOptions> mDefaultImportOptions;
	};

	/** @} */
} // namespace b3d
