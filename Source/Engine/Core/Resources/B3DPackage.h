//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DCompression.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Threading/B3DSignal.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	class PackageResourceUserMetaData;

	/** Flags that can be set per-resource package meta-data. */
	enum class B3D_SCRIPT_EXPORT() PackageResourceFlag
	{
		None = 0,
		Folder = 1 << 0, /**< Resource entry represents a folder and has no associated resource data. */
	};

	using PackageResourceFlags = Flags<PackageResourceFlag>;
	B3D_FLAGS_OPERATORS(PackageResourceFlag);

	/** Contains meta-data for a resource stored in a Package. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() PackageResourceMetaData final : public IReflectable, public IScriptExportable
	{
	public:
		PackageResourceMetaData() = default;
		PackageResourceMetaData(Path path, const UUID& id, Vector<UUID> dependencies, CompressionType compressionMethod, u32 typeId) :
		Path(std::move(path)), Id(id), Dependencies(std::move(dependencies)), CompressionType(compressionMethod), TypeId(typeId)
		{ }

		/** Returns the name of the resource. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ResourceName))
		String GetResourceName() const
		{
			return Path.IsFile() ? Path.GetFilename(false) : Path.GetTail();
		}

		B3D_SCRIPT_EXPORT()
		Path Path; /**< Path to the resource within the package. */

		B3D_SCRIPT_EXPORT()
		UUID Id; /**< Unique ID of the resource. */

		B3D_SCRIPT_EXPORT()
		u32 TypeId = 0; /**< RTTI type ID of the resource contained. */

		B3D_SCRIPT_EXPORT()
		Vector<UUID> Dependencies; /**< IDs of other resource that this resource depends on. */

		B3D_SCRIPT_EXPORT()
		CompressionType CompressionType = CompressionType::Uncompressed; /**< Type of compression used on the serialized resource data. */

		B3D_SCRIPT_EXPORT()
		PackageResourceFlags Flags = PackageResourceFlag::None; /**< Flags to provide additional information about the resource. */

		B3D_SCRIPT_EXPORT()
		TShared<PackageResourceUserMetaData> AdditionalMetaData; /**< Optional additional meta-data set explicitly by the user. This can be anything, but should be kept small. */

		B3D_SCRIPT_EXPORT()
		TShared<ResourceMetaData> ResourceMetaData; /**< Meta-data that is inherited from the Resource object. */

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PackageResourceMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains user-specified meta-data for a resource stored in a Package. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() PackageResourceUserMetaData : public IReflectable, public IScriptExportable
	{
	public:
		PackageResourceUserMetaData() = default;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PackageResourceUserMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains meta-data for the entirety of a Package (not individual resources). */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() PackageMetaData : public IReflectable, public IScriptExportable
	{
	public:
		PackageMetaData() = default;

		/**
		 * Hint to the package loader that the name of the package filename should be added as a parent path to the
		 * virtual path of all the resources in the package.
		 */
		bool IncludePackageNameInVirtualPath = false;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PackageMetaDataRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Contains a hierarchy of folders and resources within a package. */
	struct B3D_EXPORT PackageHierarchy final : INonCopyable
	{
		/** Type of entry within the package hierarchy. */
		enum class EntryType
		{
			Resource,
			Folder
		};

		/** Resource or folder entry in the package hierarchy. */
		struct Entry final
		{
			Entry(EntryType type):
			Type(type)
			{ }

			Entry(EntryType type, String name, Path path, TShared<PackageResourceMetaData> resourceMetaData = nullptr) :
			Name(std::move(name)), Path(std::move(path)), Type(type), ResourceMetaData(std::move(resourceMetaData))
			{ }

			~Entry();

			String Name; /**< Name of the entry. */
			Path Path; /**< Path of the entry in the package. */
			EntryType Type; /**< Type of the entry. */
			Vector<Entry*> Children; /**< Child entries, only relevant for folders. */
			TShared<PackageResourceMetaData> ResourceMetaData; /**< Resource meta-data. Only relevant for resources. */

		};

		PackageHierarchy() = default;

		~PackageHierarchy()
		{
			if (Root != nullptr)
				B3DPoolDelete(Root);
		}

		PackageHierarchy(PackageHierarchy&& other) :
		Root(std::exchange(other.Root, nullptr))
		{
		}

		PackageHierarchy& operator=(PackageHierarchy&& other)
		{
			if(&other != this)
			{
				if(Root != nullptr)
					B3DPoolDelete(Root);

				Root = std::exchange(other.Root, nullptr);
			}

			return *this;
		}

		/**
		 * Searches for an entry at the specified path.
		 *
		 * @param	path		Path to search for, relative to the current entry. Case sensitive.
		 * @return				Found entry, or null if not found.
		 */
		Entry* FindEntry(const Path& path) const;

		Entry* Root = nullptr;
	};

	B3D_IMPLEMENT_GLOBAL_POOL(PackageHierarchy::Entry, 64)

	/** Load state of a resource within the package. */
	enum class PackageResourceLoadState
	{
		Unloaded,
		InProgress,
		Loaded
	};

	/** Options used for controlling the Package::Save operation. */
	struct SavePackageOptions
	{
		bool CompressResources = true; /**<	If true, resources will be compressed before being saved. */

		/**
		 * If true, the save process will attempt to write only package meta-data, without writing the resource data. This is only
		 * possible if the new meta-data takes equal or less space than previous meta-data. If new meta-data takes more space
		 * than existing meta-data, then the whole package will be resaved, as if this option is disabled.
		 *
		 * You can reduce the chance the meta-data takes more space than needed by adding MetaDataPaddingByteCount when saving the package.
		 */
		bool SaveMetaDataOnly = false;

		/**
		 * Number of empty space to insert after the meta-data in the package. This allows you to overwrite the meta-data with larger meta-data in the future,
		 * without having to re-write the entire package (See the SaveMetaDataOnly option). 
		 */
		u32 MetaDataPaddingByteCount = 0;
	};

	/**
	 * Contains Resources in a virtual file system, along with their associated meta-data. Provides mechanisms to easily (de)serialize the package
	 * and load the individual resources within independently.
	 *
	 * @note Thread safe.
	 */
	class B3D_EXPORT Package final : public IReflectable, public INonCopyable
	{
		/** Runtime information about a resource stored in the package. */
		struct ResourceInformation
		{
			TShared<PackageResourceMetaData> MetaData; /**< Meta-data for the resource. This will be loaded independently of the resource data, and is always available if a package is loaded. */
			TShared<Resource> LoadedResource; /**< Resource if loaded, or null otherwise. */
			bool IsLoadedResourceDirty = false; /**< True if the loaded resource was changed and requires to be re-serialized. */

			u64 OffsetInDataStream = 0; /**< Offset in bytes at which the resource data starts in the serialized data stream. */
			u64 SizeInDataStream = 0; /**< Size of the resource data in the serialized data stream, in bytes. */

			PackageResourceLoadState LoadState = PackageResourceLoadState::Unloaded; /**< Load state of the resource. */
			std::atomic<float> LoadProgress { 0 }; /**< State of resource load, in case resource loading is in progress. */
			mutable Signal LoadSignal; /**< Signal that triggers when resource load completes. */
		};

	public:
		static constexpr const char* kPackageExtension = ".b3d";

		Package(String name, const UUID& id);

		/**
		 * @name Package information
		 * @{
		 */

		/** Determines the name of the package for easier identification. */
		const String& GetPackageName() const { return mName; }

		/** @copydoc GetPackageName() */
		void SetPackageName(const String& name);

		/** Globally unique identifier of the package. */
		const UUID& GetPackageId() const { return mId; }

		/** @copydoc GetPackageId() */
		void SetPackageId(const UUID& id);

		/** User-settable meta-data for the package as a whole. */
		const TShared<PackageMetaData>& GetPackageMetaData() const;

		/** @copydoc GetPackageMetaData(). */
		void SetPackageMetaData(const TShared<PackageMetaData>& metaData);

		/** Returns the total number of resources in the package. */
		u32 GetResourceCount() const { return (u32)mResourceInformationByUUID.size(); }

		/** Returns true if the package contains no resources. */
		bool IsEmpty() const;

		/** Creates a list of unique identifiers for all resources within the package. */
		Vector<UUID> CreateResourceIdList() const;

		/** Creates an object containing the folder hierarchy of all the resources within the package. */
		PackageHierarchy CreateHierarchy() const;

		/**
		 * Breaks a combined path to a package and a resource in that package into a separate path pointing
		 * to the package, and a separate path for the resource. e.g. 'D:/path/to/package.b3d/path/to/resource' will
		 * be broken into 'D:/path/to/package.b3d' and '/path/to/resource'.
		 */
		static bool BreakCombinedPackagePath(const Path& combinedPath, Path& outPathToPackage, Path& outPathToResource);

		/** @} */

		/**
		 * @name Resource operations
		 * @{
		 */

		/** Returns true if the package contains a resource with the specified identifier. */
		bool Contains(const UUID& id) const;

		/** Returns true if the package contains a resource at the specified path. Paths are case sensitive. */
		bool Contains(const Path& path) const;

		/** Returns resource meta-data for resource with the specified identifier. Returns null if not found. */
		TShared<const PackageResourceMetaData> GetResourceMetaData(const UUID& id) const;

		/** Returns resource meta-data for resource at the specified path. Returns null if not found. Paths are case sensitive. */
		TShared<const PackageResourceMetaData> GetResourceMetaData(const Path& path) const;

		/** Assigns additional meta-data for the resource with the specified identifier. */
		void SetResourceMetaData(const UUID& id, const TShared<PackageResourceUserMetaData>& data);

		/** Assigns additional meta-data for the resource at the specified path. Paths are case sensitive. */
		void SetResourceMetaData(const Path& path, const TShared<PackageResourceUserMetaData>& data);

		/** Check if the resource is unloaded, loaded or in progress of being loaded. */
		PackageResourceLoadState GetResourceLoadState(const UUID& id) const;

		/** Returns the load progress of the resource. Only relevant of GetResourceLoadState returns the in-progress state. */
		float GetResourceLoadProgress(const UUID& id) const;

		/** Registers a new resource with the package at the specified path. Paths are case sensitive. Empty entries are treated as folders. */
		void AddResource(const Path& path, const HResource& resource);

		/** Registers a new resource with the package at the specified path. Paths are case sensitive. Empty entries are treated as folders. */
		void AddResource(const Path& path, const TShared<Resource>& resource);

		/** Removes one or multiple resources from the provided path. Paths are case sensitive. If the path represents a folder and @p recursive is true, all resources within the folder will be removed. */
		void RemoveResource(const Path& path, bool recursive);

		/** Removes a resource with the specified id. If the id represents a folder and @p recursive is true, all resources within the folder will be removed. */
		void RemoveResource(const UUID& id, bool recursive);

		/**
		 * Changes a resource instance associated with a particular ID. Resource must have already been registered with the package.
		 * 
		 * @param	resource		Resource to assign. Id of the resource will be used for determining which resource to update.
		 * @param	markAsDirty		If true the resource will be serialized during the next SerializePackage call. Non-dirty resources will just have their source data copied without re-serializing.
		 */
		void SetResource( const TShared<Resource>& resource, bool markAsDirty = true);

		/**
		 * Changes the path of one or multiple resources within the package.
		 *
		 * @param	path		Existing path to rename. Paths are case sensitive.
		 * @param	newPath		New path to the resource. Paths are case sensitive.
		 * @param	recursive	If the path represents a folder containing multiple resources, all the resources will be renamed.
		 * @return				True if the path was successfully changed.
		 */
		bool SetResourcePath(const Path& path, const Path& newPath, bool recursive);

		/**
		 * Changes the path of one or multiple resources within the package.
		 *
		 * @param	id			Unique id of the resource.
		 * @param	newPath		New path to the resource. Paths are case sensitive.
		 * @param	recursive	If the id represents a folder containing multiple resources, all the resources will be renamed.
		 * @return				True if the path was successfully changed.
		 */
		bool SetResourcePath(const UUID& id, const Path& newPath, bool recursive);

		/**
		 * Loads the resource with the specified id into memory, or returns an already loaded resource if previously loaded. Resource will remain loaded for future calls, unless explicitly unloaded or destroyed.
		 *
		 * @param	id			Unique id of the resource.
		 * @return				Resource if successful, or null otherwise.
		 */
		TShared<Resource> LoadResource(const UUID& id);

		/**
		 * Loads the resource with the specified id into memory, or returns an already loaded resource if previously loaded. Resource will remain loaded for future calls, unless explicitly unloaded or destroyed.
		 *
		 * @param	path		Path to the resource. Paths are case sensitive.
		 * @return				Resource if successful, or null otherwise.
		 */
		TShared<Resource> LoadResource(const Path& path);

		/**
		 * Deserializes a new instance of the resource with the specified id. This is similar to LoadResource(), but it does not cache the loaded resource internally, instead it always returns a fresh instance.
		 *
		 * @param	id			Unique id of the resource.
		 * @return				Resource if successful, or null otherwise.
		 */
		TShared<Resource> DeserializeResource(const UUID& id) const;

		/**
		 * Deserializes a new instance of the resource with the specified path. This is similar to LoadResource(), but it does not cache the loaded resource internally, instead it always returns a fresh instance.
		 *
		 * @param	path		Path to the resource. Paths are case sensitive.
		 * @return				Resource if successful, or null otherwise.
		 */
		TShared<Resource> DeserializeResource(const Path& path) const;

		/**
		 * Returns a previously loaded resource.
		 *
		 * @param	id			Unique id of the resource.
		 * @return				Resource if present and loaded, null otherwise.
		 */
		TShared<Resource> GetResource(const UUID& id) const;

		/**
		 * Returns a previously loaded resource.
		 *
		 * @param	path		Path to the resource. Paths are case sensitive.
		 * @return				Resource if present and loaded, null otherwise.
		 */
		TShared<Resource> GetResource(const Path& path) const;

		/** Unloads the resource with specified id. */
		void UnloadResource(const UUID& id);

		/** Unloads all resources loaded with LoadResource(). */
		void UnloadAllResources();

		/**
		 * Returns the size of the resource when serialized to the data stream, in bytes. Returns 0 if resource has not yet been serialized, or if resource was not found.
		 * Might return an out of date value if runtime resource changed but hasn't been serialized yet.
		 */
		u64 GetResourceSizeInDataStream(const UUID& id) const;

		/** @} */

		/**
		 * @name Package serialization and creation
		 * @{
		 */

		/**
		 * Saves the package into the provided data stream.
		 *
		 * @param	stream		Stream into which to save the package.
		 * @param	options		Options controlling the save operation.
		 * @return 				True if successful, false otherwise. If saving meta-data only, if false it returned it means the meta-data doesn't fit
		 *						and you must attempt to re-save the entire package.
		 */
		bool Save(const TShared<DataStream>& stream, const SavePackageOptions& options);

		/** Creates a new empty package. */
		static TShared<Package> Create(const String& name = StringUtility::kBlank, const UUID& id = UUID::kEmpty);

		/** Loads the package from the provided path. */
		static TShared<Package> Load(const Path& path);

		/** Loads the package from the provided data stream. */
		static TShared<Package> Load(const TShared<DataStream>& strean);

		/** @} */

		/**
		 * @name Internal
		 */

		/** Associates a file with the package. Any resource load attempts will be done from this file. A file will be automatically associated with the package once the package is saved to a file, or when a package is loaded from a file. */
		void AssociateFileWithPackage(const Path& path);

		/**
		 * Creates a clone of the package, with all the meta-data deep copied into the new package. Loaded resources are not copied and the cloned
		 * package will still be referencing the original resources.
		 *
		 * As both packages are pointing to the same resources, and to the same file (if serialized), you should never keep multiple clones of the package persistently active.
		 *
		 * Generally you only wish to clone to update the package contents and potentially serialize it to some new location, while the original location can be used for regular
		 * load purposes (to avoid blocking users of the package). When the slow serialization operation completes, you will generally kill the original package and replace
		 * it with the clone.
		 *
		 * Note that in this scenario it's important to call CopyResourceLoadStatesFrom() before the package is replaced, as the original package's resource load states could have been modified.
		 */
		TShared<Package> Clone() const;

		/** Copies the resource load states from a clone of this package. Caller must ensure no in-progress loads are happening in @p otherPackage. See @p Clone(). */
		void CopyResourceLoadStatesFromClone(const Package& otherPackage);

		/** @} */
	private:
		/**
		 * Deserializes a resource from a specific location in the package data stream.
		 *
		 * @param	id						Id of the resource to deserialize.
		 * @param	offsetInStream			Offset into the stream at which the resource starts, in bytes.
		 * @param	sizeInStream			Size of the resources in the stream, in bytes.
		 * @param	compressionType			Compression method that was used to compress the data, if any.
		 * @param	outProgress				Parameter in which to report the load progress, ranging [0, 1].
		 * @return							Loaded resource if successful.
		 */
		TShared<Resource> LoadAndDeserializeResource(const UUID& id, u64 offsetInStream, u64 sizeInStream, CompressionType compressionType, std::atomic<float>& outProgress) const;

		/** Unloads the resource with the associated resource information. */
		void UnloadResource(ResourceInformation* resourceInfo);

		/**
		 * Returns package internal information about the resource with specified id.
		 *
		 * @param	id				Id of the resource to look for.
		 * @param	warnIfMissing	If true, a warning will be logged if the resource information cannot be found.
		 * @return					Resource information if found, or null otherwise.
		 */
		ResourceInformation* GetResourceInformation(const UUID& id, bool warnIfMissing = true) const;

		/**
		 * Returns package internal information about the resource with specified path.
		 *
		 * @param	path			Path of the resource to look for. Paths are case sensitive.
		 * @param	warnIfMissing	If true, a warning will be logged if the resource information cannot be found.
		 * @return					Resource information if found, or null otherwise.
		 */
		ResourceInformation* GetResourceInformation(const Path& path, bool warnIfMissing = true) const;

		String mName;
		UUID mId;
		Path mAssociatedPackageFilePath; /**< Path to the file in which the package data has been saved. Empty if package hasn't been saved yet. */
		TShared<PackageMetaData> mPackageMetaData;
		size_t mSerializedMetaDataEnd = 0;
		size_t mMetaDataPaddingByteCount = 0; /**< Extra empty bytes in the file after meta-data. Allows meta-data to grow without having to re-write the whole file. */

		UnorderedMap<Path, ResourceInformation*, PathHashFunction<true>, PathEqualsFunction<true>> mResourceInformationByPath;
		UnorderedMap<UUID, TUnique<ResourceInformation>> mResourceInformationByUUID;

		mutable Mutex mMetaDataMutex;
		mutable Mutex mPathMutex;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PackageRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		Package() = default;
	};

	/** @} */
} // namespace b3d
