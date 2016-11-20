//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsSpecificImporter.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** 
	 * Contains a resource that was imported from a file that contains multiple resources (for example an animation from an
	 * FBX file). 
	 */
	struct SubResource
	{
		WString name; /**< Unique name of the sub-resource. */
		HResource value; /**< Contents of the sub-resource. */
	};

	/** Module responsible for importing various asset types and converting them to types usable by the engine. */
	class BS_CORE_EXPORT Importer : public Module<Importer>
	{
	public:
		Importer(); 
		~Importer(); 

		/**
		 * Imports a resource at the specified location, and returns the loaded data. If file contains more than one
		 * resource only the primary resource is imported (for example an FBX a mesh would be imported, but animations
		 * ignored).
		 *
		 * @param[in]	inputFilePath	Pathname of the input file.
		 * @param[in]	importOptions	(optional) Options for controlling the import. Caller must ensure import options 
		 *								actually match the type of the importer used for the file type.
		 * @param[in]	UUID			Specific UUID to assign to the resource. If not specified a randomly generated
		 *								UUID will be assigned.
		 * @return						Imported resource.
		 *
		 * @see		createImportOptions
		 */
		HResource import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, 
			const String& UUID = StringUtil::BLANK);

		/** @copydoc import */
		template <class T>
		ResourceHandle<T> import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, 
			const String& UUID = StringUtil::BLANK)
		{
			return static_resource_cast<T>(import(inputFilePath, importOptions, UUID));
		}

		/**
		 * Imports a resource at the specified location, and returns the loaded data. This method returns all imported
		 * resources, which is relevant for files that can contain multiple resources (for example an FBX which may contain
		 * both a mesh and animations). 
		 *
		 * @param[in]	inputFilePath	Pathname of the input file.
		 * @param[in]	importOptions	(optional) Options for controlling the import. Caller must ensure import options 
		 *								actually match the type of the importer used for the file type.
		 * @return						A list of all imported resources. The primary resource is always the first returned
		 *								resource.
		 *
		 * @see		createImportOptions
		 */
		Vector<SubResource> importAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/**
		 * Imports a resource and replaces the contents of the provided existing resource with new imported data.
		 *
		 * @param[in]	existingResource	Resource whose contents to replace.
		 * @param[in]	inputFilePath		Pathname of the input file.
		 * @param[in]	importOptions		(optional) Options for controlling the import. Caller must ensure import options 
		 *									actually match the type of the importer used for the file type. 
		 *
		 * @see		createImportOptions
		 */
		void reimport(HResource& existingResource, const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/**
		 * Automatically detects the importer needed for the provided file and returns valid type of import options for 
		 * that importer.
		 *
		 * @param[in]	inputFilePath	Pathname of the input file.
		 *
		 * @return						The new import options. Null is returned if the file path is not valid, or if a 
		 *								valid importer cannot be found for the specified file.
		 * 			
		 * @note	
		 * You will need to type cast the importer options to a valid type, taking into consideration exact importer you 
		 * expect to be used for this file type. If you don't use a proper import options type, an exception will be thrown 
		 * during import.
		 */
		SPtr<ImportOptions> createImportOptions(const Path& inputFilePath);

		/** @copydoc createImportOptions */
		template<class T>
		SPtr<T> createImportOptions(const Path& inputFilePath)
		{
			return std::static_pointer_cast<T>(createImportOptions(inputFilePath));
		}

		/**
		 * Checks if we can import a file with the specified extension.
		 *
		 * @param[in]	extension	The extension without the leading dot.
		 */
		bool supportsFileType(const WString& extension) const;

		/**
		 * Checks if we can import a file with the specified magic number.
		 *
		 * @param[in]	magicNumber 	The buffer containing the magic number.
		 * @param[in]	magicNumSize	Size of the magic number buffer.
		 */
		bool supportsFileType(const UINT8* magicNumber, UINT32 magicNumSize) const;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Registers a new asset importer for a specific set of extensions (as determined by the implementation). If an
		 * asset importer for one or multiple extensions already exists, it is removed and replaced with this one.
		 *
		 * @param[in]	importer	The importer that is able to handle import of certain type of files. 
		 *
		 * @note	This method should only be called by asset importers themselves on startup. Importer takes ownership
		 *			of the provided pointer and will release it. Assumes it is allocated using the general allocator.
		 */
		void _registerAssetImporter(SpecificImporter* importer);

		/** Alternative to importAll() which doesn't create resource handles, but instead returns raw resource pointers. */
		Vector<SubResourceRaw> _importAllRaw(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/** @} */
	private:
		/** 
		 * Searches available importers and attempts to find one that can import the file of the provided type. Returns null
		 * if one cannot be found.
		 */
		SpecificImporter* getImporterForFile(const Path& inputFilePath) const;

		Vector<SpecificImporter*> mAssetImporters;
	};

	/** Provides easier access to Importer. */
	BS_CORE_EXPORT Importer& gImporter();

	/** @} */
}