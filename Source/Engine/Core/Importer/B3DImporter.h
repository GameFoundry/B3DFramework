//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Importer/B3DSpecificImporter.h"
#include "Threading/B3DAsyncOp.h"
#include "Resources/B3DResource.h"
#include "Threading/B3DSchedulerTicket.h"

namespace b3d
{
	class SignalEvent;
	/** @addtogroup Importer
	 *  @{
	 */

	/**
	 * Contains a resource that was imported from a file that contains multiple resources (for example an animation from an
	 * FBX file).
	 */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), ExportAsStruct(true), API(Framework)) SubResource
	{
		String Name; /**< Unique name of the sub-resource. */
		B3D_NO_RREF HResource Value; /**< Contents of the sub-resource. */
	};

	/** Contains a group of resources imported from a single source file. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework)) MultiResource
	{
		B3D_SCRIPT_EXPORT()
		MultiResource() = default;

		B3D_SCRIPT_EXPORT()

		MultiResource(const Vector<SubResource>& entries)
			: Entries(entries)
		{}

		B3D_SCRIPT_EXPORT()
		Vector<SubResource> Entries;
	};

	/** Module responsible for importing various asset types and converting them to types usable by the engine. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), API(Framework)) Importer : public Module<Importer>
	{
	public:
		Importer();
		~Importer();

		/**
		 * Imports a resource at the specified location, and returns the loaded data. If file contains more than one
		 * resource only the primary resource is imported (for example an FBX a mesh would be imported, but animations
		 * ignored).
		 *
		 * @param	inputFilePath	Pathname of the input file.
		 * @param	importOptions	(optional) Options for controlling the import. Caller must ensure import options
		 *							actually match the type of the importer used for the file type.
		 * @param	UUID			Specific UUID to assign to the resource. If not specified a randomly generated
		 *							UUID will be assigned.
		 * @return					Imported resource.
		 *
		 * @see		CreateImportOptions
		 * @note	Thread safe.
		 */
		B3D_SCRIPT_EXPORT()
		B3D_NO_RREF HResource Import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, const UUID& UUID = UUID::kEmpty);

		/** @copydoc Import */
		template <class T>
		TResourceHandle<T> Import(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, const UUID& UUID = UUID::kEmpty)
		{
			return B3DStaticResourceCast<T>(Import(inputFilePath, importOptions, UUID));
		}

		/**
		 * Same as import(), except it imports a resource without blocking the main thread. The resulting resource will be
		 * placed in the returned AsyncOp object when the import ends.
		 */
		B3D_SCRIPT_EXPORT()
		TAsyncOp<HResource> ImportAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, const UUID& UUID = UUID::kEmpty);

		/**
		 * Imports a resource at the specified location, and returns the loaded data. This method returns all imported
		 * resources, which is relevant for files that can contain multiple resources (for example an FBX which may contain
		 * both a mesh and animations).
		 *
		 * @param	inputFilePath	Pathname of the input file.
		 * @param	importOptions	(optional) Options for controlling the import. Caller must ensure import options
		 *							actually match the type of the importer used for the file type.
		 * @return					A list of all imported resources. The primary resource is always the first returned
		 *							resource.
		 *
		 * @see		CreateImportOptions
		 * @note	Thread safe.
		 */
		B3D_SCRIPT_EXPORT()
		SPtr<MultiResource> ImportAll(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/**
		 * Same as importAll(), except it imports a resource without blocking the main thread. The returned AsyncOp will
		 * contain a list of the imported resources, after the import ends.
		 */
		B3D_SCRIPT_EXPORT()
		TAsyncOp<SPtr<MultiResource>> ImportAllAsync(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/**
		 * Automatically detects the importer needed for the provided file and returns valid type of import options for
		 * that importer.
		 *
		 * @param	inputFilePath	Pathname of the input file.
		 *
		 * @return					The new import options. Null is returned if the file path is not valid, or if a
		 *							valid importer cannot be found for the specified file.
		 *
		 * @note
		 * You will need to type cast the importer options to a valid type, taking into consideration exact importer you
		 * expect to be used for this file type. If you don't use a proper import options type, an exception will be thrown
		 * during import.
		 */
		SPtr<ImportOptions> CreateImportOptions(const Path& inputFilePath);

		/** @copydoc CreateImportOptions */
		template <class T>
		SPtr<T> CreateImportOptions(const Path& inputFilePath)
		{
			return std::static_pointer_cast<T>(CreateImportOptions(inputFilePath));
		}

		/**
		 * Checks if we can import a file with the specified extension.
		 *
		 * @param	extension	The extension without the leading dot.
		 */
		B3D_SCRIPT_EXPORT()
		bool SupportsFileType(const String& extension) const;

		/**
		 * Checks if we can import a file with the specified magic number.
		 *
		 * @param	magicNumber			The buffer containing the magic number.
		 * @param	magicNumberSize		Size of the magic number buffer.
		 */
		bool SupportsFileType(const u8* magicNumber, u32 magicNumberSize) const;

		/** @name Internal
		 *  @{
		 */

		/**
		 * Registers a new asset importer for a specific set of extensions (as determined by the implementation). If an
		 * asset importer for one or multiple extensions already exists, it is removed and replaced with this one.
		 *
		 * @param	importer	The importer that is able to handle import of certain type of files.
		 *
		 * @note	This method should only be called by asset importers themselves on startup. Importer takes ownership
		 *			of the provided pointer and will release it. Assumes it is allocated using the general allocator.
		 */
		void RegisterAssetImporter(SpecificImporter* importer);

		/** Alternative to import() which doesn't create a resource handle, but instead returns a raw resource pointer. */
		SPtr<Resource> ImportInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/** Alternative to ImportAll() which doesn't create resource handles, but instead returns raw resource pointers. */
		Vector<SubResourceRaw> ImportAllInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/** Alternative to ImportAllAsync() which doesn't create resource handles, but instead returns raw resource pointers. */
		TAsyncOp<Vector<SubResourceRaw>> ImportAllAsyncInternal(const Path& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

		/** @} */
	private:
		/**
		 * Searches available importers and attempts to find one that can import the file of the provided type. Returns null
		 * if one cannot be found.
		 */
		SpecificImporter* GetImporterForFile(const Path& inputFilePath) const;

		/**
		 * Queues resource for import on a secondary thread. The system will execute the import as soon as possible
		 * and write the resulting resource to the provided @p op object.
		 */
		template <class ReturnType>
		void QueueForImport(SpecificImporter* importer, const Path& inputFilePath, const SPtr<const ImportOptions>& importOptions, const UUID& uuid, TAsyncOp<ReturnType>& op);

		/**
		 * Prepares for import of a file at the specified path. Returns the type of importer the file can be imported with,
		 * or null if the file isn't valid or is of unsupported type. Also creates the default set of import options unless
		 * already provided.
		 */
		SpecificImporter* PrepareForImport(const Path& filePath, SPtr<const ImportOptions>& importOptions) const;

		/**
		 * Checks is the specific importer currently importing something asynchronously. If the importer doesn't support
		 * multiple threads then the method will wait until async. import completes.
		 */
		void WaitForAsync(SpecificImporter* importer);

		Vector<SpecificImporter*> mAssetImporters;
		UnorderedMap<SpecificImporter*, UPtr<SchedulerTicketQueue>> mPerImporterQueues; /**< Queues for importers having to run sequential tasks. */
		Mutex mPerImporterQueueMutex;
	};

	/** Provides easier access to Importer. */
	B3D_EXPORT Importer& GetImporter();

	/** @} */
} // namespace b3d
