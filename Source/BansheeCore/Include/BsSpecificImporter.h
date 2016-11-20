//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
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
		WString name; /**< Unique name of the sub-resource. */
		SPtr<Resource> value; /**< Contents of the sub-resource. */
	};

	/**
	 * Abstract class that is to be specialized for converting a certain asset type into an engine usable resource
	 * (for example a .png file into an engine usable texture).
	 * 			
	 * On initialization this class must register itself with the Importer module, which delegates asset import calls to a 
	 * specific importer.
	 */
	class BS_CORE_EXPORT SpecificImporter
	{
	public:
		SpecificImporter() {}
		virtual ~SpecificImporter() {}

		/**
		 * Check is the provided extension supported by this importer.
		 *
		 * @note	Provided extension should be without the leading dot.
		 */
		virtual bool isExtensionSupported(const WString& ext) const = 0;

		/** Check if the provided magic number is supported by this importer. */
		virtual bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const = 0; 

		/**
		 * Imports the given file. If file contains more than one resource only the primary resource is imported (for 
		 * example for an FBX a mesh would be imported, but animations ignored).
		 *
		 * @param[in]	filePath		Pathname of the file, with file extension.
		 * @param[in]	importOptions	Options that can control how is the resource imported.
		 * @return						null if it fails, otherwise the loaded object.
		 */
		virtual SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) = 0;

		/**
		 * Imports the given file. This method returns all imported resources, which is relevant for files that can contain
		 * multiple resources (for example an FBX which may contain both a mesh and animations). 
		 *
		 * @param[in]	filePath		Pathname of the file, with file extension.
		 * @param[in]	importOptions	Options that can control how are the resources imported.
		 * @return						Empty array if it fails, otherwise the loaded objects. First element is always the 
		 *								primary resource.
		 */
		virtual Vector<SubResourceRaw> importAll(const Path& filePath, SPtr<const ImportOptions> importOptions);

		/**
		 * Creates import options specific for this importer. Import options are provided when calling import() in order 
		 * to customize the import, and provide additional information.
		 */
		virtual SPtr<ImportOptions> createImportOptions() const;

		/**
		 * Gets the default import options.
		 *
		 * @return	The default import options.
		 */
		SPtr<const ImportOptions> getDefaultImportOptions() const;

	private:
		mutable SPtr<const ImportOptions> mDefaultImportOptions;
	};

	/** @} */
}