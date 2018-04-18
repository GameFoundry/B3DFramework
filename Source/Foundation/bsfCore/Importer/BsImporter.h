//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Importer/BsSpecificImporter.h"
#include "Threading/BsAsyncOp.h"

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
		String name; /**< Unique name of the sub-resource. */
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
			const UUID& UUID = UUID::EMPTY);

		/** @copydoc import */
		template <class T>
		ResourceHandle<T> import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, 
			const UUID& UUID = UUID::EMPTY)
		{
			return static_resource_cast<T>(import(inputFilePath, importOptions, UUID));
		}

		/** 
		 * Same as import(), except it imports a resource without blocking the main thread. The resulting resource will be
		 * placed in the returned AsyncOp object when the import ends. If @p handle is true, the returned object will be
		 * a resource handle, otherwise it will be a SPtr to the resource.
		 */
		AsyncOp importAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, 
			const UUID& UUID = UUID::EMPTY, bool handle = true);

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
		 * Same as importAll(), except it imports a resource without blocking the main thread. The returned AsyncOp will
		 * contain a Vector<SubResource> containing the imported resources, after the import ends. If @p handle is true, 
		 * the returned object will be a resource handle, otherwise it will be a SPtr to the resource.
		 */
		AsyncOp importAllAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, 
			bool handle = true);

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
		bool supportsFileType(const String& extension) const;

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

		/** Alternative to import() which doesn't create a resource handle, but instead returns a raw resource pointer. */
		SPtr<Resource> _import(const Path& inputFilePath, 
			SPtr<const ImportOptions> importOptions = nullptr) const;

		/** Alternative to importAll() which doesn't create resource handles, but instead returns raw resource pointers. */
		Vector<SubResourceRaw> _importAll(const Path& inputFilePath, 
			SPtr<const ImportOptions> importOptions = nullptr) const;

		/** @} */
	private:
		/** Information about a single queued import operation. */
		struct QueuedOperation
		{
			QueuedOperation() = default;

			QueuedOperation(SpecificImporter* importer, const Path& filePath, SPtr<const ImportOptions> importOptions, 
				bool importAll, const UUID& uuid, bool handle, const AsyncOp& op)
				: importer(importer), filePath(filePath), importOptions(importOptions), importAll(importAll), uuid(uuid)
				, handle(handle), op(op)
			{ }

			SpecificImporter* importer;
			Path filePath;
			SPtr<const ImportOptions> importOptions;
			bool importAll;
			UUID uuid;
			bool handle;

			AsyncOp op;
		};

		/** 
		 * Searches available importers and attempts to find one that can import the file of the provided type. Returns null
		 * if one cannot be found.
		 */
		SpecificImporter* getImporterForFile(const Path& inputFilePath) const;

		/** 
		 * Queues resource for import on a secondary thread. The system will execute the import as soon as possible
		 * and write the resulting resource to the provided @p op object. 
		 */
		void queueForImport(SpecificImporter* importer, const Path& inputFilePath, SPtr<const ImportOptions> importOptions, 
			bool importAll, const UUID& uuid, bool handle, AsyncOp& op);

		/**
		 * Prepares for import of a file at the specified path. Returns the type of importer the file can be imported with,
		 * or null if the file isn't valid or is of unsupported type. Also creates the default set of import options unless
		 * already provided.
		 */
		SpecificImporter* prepareForImport(const Path& filePath, SPtr<const ImportOptions>& importOptions) const;

		/**
		 * Checks is the specific importer currently importing something asynchronously. If the importer doesn't support
		 * multiple threads then the method will wait until async. import completes.
		 */
		void waitForAsync(SpecificImporter* importer) const;

		Vector<SpecificImporter*> mAssetImporters;

		mutable Mutex mLastTaskMutex;
		mutable Signal mTaskCompleted;
		UINT64 mTaskId = 0;

		/** Information about a task queued for a specific import operation. */
		struct QueuedTask
		{
			QueuedTask() = default;

			QueuedTask(SPtr<Task> task, UINT64 id)
				:task(std::move(task)), id(id)
			{ }

			SPtr<Task> task;
			UINT64 id;
		};

		UnorderedMap<SpecificImporter*, QueuedTask> mLastQueuedTask;
	};

	/** Provides easier access to Importer. */
	BS_CORE_EXPORT Importer& gImporter();

	/** @} */
}