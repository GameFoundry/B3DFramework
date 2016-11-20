//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Library
	 *  @{
	 */

	/**
	 * Project library is the primary location for interacting with all the resources in the current project. A complete
	 * hierarchy of resources is provided which can be interacted with by importing new ones, deleting them, moving,
	 * renaming and similar.
	 */
	class BS_ED_EXPORT ProjectLibrary : public Module<ProjectLibrary>
	{
	public:
		struct LibraryEntry;
		struct FileEntry;
		struct DirectoryEntry;

		/** Types of elements in the library, either a file or a folder. */
		enum class LibraryEntryType
		{
			File,
			Directory
		};

		/**	A generic library entry that may be a file or a folder depending on its type. */
		struct LibraryEntry
		{
			LibraryEntry();
			LibraryEntry(const Path& path, const WString& name, DirectoryEntry* parent, LibraryEntryType type);

			LibraryEntryType type; /**< Specific type of this entry. */
			Path path; /**< Absolute path to the entry. */
			WString elementName; /**< Name of the entry. */

			DirectoryEntry* parent; /**< Folder this entry is located in. */
		};

		/**	A library entry representing a file. Each file can have one or multiple resources. */
		struct FileEntry : public LibraryEntry
		{
			FileEntry();
			FileEntry(const Path& path, const WString& name, DirectoryEntry* parent);

			SPtr<ProjectFileMeta> meta; /**< Meta file containing various information about the resource(s). */
			std::time_t lastUpdateTime; /**< Timestamp of when we last imported the resource. */
		};

		/**	A library entry representing a folder that contains other entries. */
		struct DirectoryEntry : public LibraryEntry
		{
			DirectoryEntry();
			DirectoryEntry(const Path& path, const WString& name, DirectoryEntry* parent);

			Vector<LibraryEntry*> mChildren; /**< Child files or folders. */
		};

	public:
		ProjectLibrary();
		~ProjectLibrary();

		/**
		 * Checks if any resources at the specified path have been modified, added or deleted, and updates the internal
		 * hierarchy accordingly. Automatically imports dirty resources.
		 *
		 * @param[in]	path	Absolute path of the file or folder to check. If a folder is provided all its children will
		 *						be checked recursively.
		 */
		void checkForModifications(const Path& path);

		/**
		 * Checks if any resources at the specified path have been modified, added or deleted, and updates the internal
		 * hierarchy accordingly. 
		 *
		 * @param[in]	path			Absolute path of the file or folder to check. If a folder is provided all its 
		 *								children will be checked recursively.
		 * @param[in]	import			Should the dirty resources be automatically reimported.
		 * @param[in]	dirtyResources	A list of resources that should be reimported.
		 */
		void checkForModifications(const Path& path, bool import, Vector<Path>& dirtyResources);

		/**	Returns the root library entry that references the entire library hierarchy. */
		const LibraryEntry* getRootEntry() const { return mRootEntry; }

		/**
		 * Attempts to a find a library entry at the specified path.
		 *
		 * @param[in]	path	Path to the entry, either absolute or relative to resources folder.
		 * @return				Found entry, or null if not found. Value returned by this method is transient, it may be
		 *						destroyed on any following ProjectLibrary call.
		 */
		LibraryEntry* findEntry(const Path& path) const;

		/** 
		 * Checks whether the provided path points to a sub-resource. Sub-resource is any resource that is not the primary
		 * resource in the file.
		 */
		bool isSubresource(const Path& path) const;

		/**
		 * Attempts to a find a meta information for a resource at the specified path.
		 *
		 * @param[in]	path	Path to the entry, either absolute or relative to resources folder. If a sub-resource within
		 *						a file is needed, append the name of the subresource to the path 
		 *						(for example mymesh.fbx/my_animation).
		 * @return				Found meta information for the resource, or null if not found. 
		 */
		SPtr<ProjectResourceMeta> findResourceMeta(const Path& path) const;

		/**
		 * Searches the library for a pattern and returns all entries matching it.
		 *
		 * @param[in]	pattern	Pattern to search for. Use wildcard * to match any character(s).
		 * @return		A list of entries matching the pattern. Values returned by this method are transient, they may be
		 *				destroyed on any following ProjectLibrary call.
		 */
		Vector<LibraryEntry*> search(const WString& pattern);

		/**
		 * Searches the library for a pattern, but only among specific resource types.
		 *
		 * @param[in]	pattern	Pattern to search for. Use wildcard * to match any character(s).
		 * @param[in]	typeIds	RTTI type IDs of the resource types we're interested in searching.
		 * @return		A list of entries matching the pattern. Values returned by this method are transient, they may be
		 *				destroyed on any following ProjectLibrary call.
		 */
		Vector<LibraryEntry*> search(const WString& pattern, const Vector<UINT32>& typeIds);

		/**
		 * Returns resource path based on its UUID.
		 *
		 * @param[in]	uuid	UUID of the resource to look for.
		 * @return				Absolute path to the resource.
		 */
		Path uuidToPath(const String& uuid) const;

		/**
		 * Registers a new resource in the library.
		 *
		 * @param[in]	resource	Resource instance to add to the library. A copy of the resource will be saved at the
		 *							provided path.
		 * @param[in]	path		Path where where to store the resource. Absolute or relative to the resources folder.
		 */
		void createEntry(const HResource& resource, const Path& path);

		/**
		 * Creates a new folder in the library.
		 *
		 * @param[in]	path		Path where where to store the folder. Absolute or relative to the resources folder.
		 */
		void createFolderEntry(const Path& path);

		/**	Updates a resource that is already in the library. */
		void saveEntry(const HResource& resource);

		/**
		 * Moves a library entry from one path to another.
		 *
		 * @param[in]	oldPath		Source path of the entry, absolute or relative to resources folder.
		 * @param[in]	newPath		Destination path of the entry, absolute or relative to resources folder.
		 * @param[in]	overwrite	If an entry already exists at the destination path, should it be overwritten.
		 */
		void moveEntry(const Path& oldPath, const Path& newPath, bool overwrite = true);

		/**
		 * Copies a library entry from one path to another.
		 *
		 * @param[in]	oldPath		Source path of the entry, absolute or relative to resources folder.
		 * @param[in]	newPath		Destination path of the entry, absolute or relative to resources folder.
		 * @param[in]	overwrite	If an entry already exists at the destination path, should it be overwritten.
		 */
		void copyEntry(const Path& oldPath, const Path& newPath, bool overwrite = true);

		/**
		 * Deletes an entry from the library.
		 *
		 * @param[in]	path		Path of the entry, absolute or relative to resources folder.
		 */
		void deleteEntry(const Path& path);

		/**
		 * Triggers a reimport of a resource using the provided import options, if needed.
		 *
		 * @param[in]	path			Path to the resource to reimport, absolute or relative to resources folder.
		 * @param[in]	importOptions	Optional import options to use when importing the resource. Caller must ensure the
		 *								import options are of the correct type for the resource in question. If null is
		 *								provided default import options are used.
		 * @param[in]	forceReimport	Should the resource be reimported even if no changes are detected. This should be
		 *								true if import options changed since last import.
		 */
		void reimport(const Path& path, const SPtr<ImportOptions>& importOptions = nullptr, bool forceReimport = false);

		/**
		 * Determines if this resource will always be included in the build, regardless if it's being referenced or not.
		 *
		 * @param[in]	path	Path to the resource to modify, absolute or relative to resources folder.
		 * @param[in]	force	True if we want the resource to be included in the build, false otherwise.
		 */
		void setIncludeInBuild(const Path& path, bool force);

		/** 
		 * Assigns a non-specific user data object to the resource at the specified path. 
		 * 
		 * @param[in]	path		Path to the resource to modify, absolute or relative to resources folder.
		 * @param[in]	userData	User data to assign to the resource, which can later be retrieved from the resource's
		 *							meta-data as needed. 
		 */
		void setUserData(const Path& path, const SPtr<IReflectable>& userData);

		/**
		 * Finds all top-level resource entries that should be included in a build. Values returned by this method are
		 * transient, they may be destroyed on any following ProjectLibrary call.
		 */
		Vector<FileEntry*> getResourcesForBuild() const;

		/**
		 * Loads a resource at the specified path, synchronously.
		 *
		 * @param[in]	path	Path of the resource, absolute or relative to resources folder. If a sub-resource within
		 *						a file is needed, append the name of the subresource to the path 
		 *						(for example mymesh.fbx/my_animation).
		 * @return				Loaded resource, or null handle if one is not found.
		 */
		HResource load(const Path& path);

		/** Returns the path to the project's resource folder where all the assets are stored. */
		const Path& getResourcesFolder() const { return mResourcesFolder; }

		/**
		 * Saves all the project library data so it may be restored later, at the default save location in the project
		 * folder. Project must be loaded when calling this.
		 */
		void saveLibrary();

		/**
		 * Loads previously saved project library data from the default save location in the project folder. Nothing is
		 * loaded if it doesn't exist.Project must be loaded when calling this.
		 */
		void loadLibrary();

		/**	Clears all library data. */
		void unloadLibrary();

		/** Triggered whenever an entry is removed from the library. Path provided is absolute. */
		Event<void(const Path&)> onEntryRemoved; 

		/** Triggered whenever an entry is added to the library. Path provided is absolute. */
		Event<void(const Path&)> onEntryAdded; 

		/** Triggered when a resource is being (re)imported. Path provided is absolute. */
		Event<void(const Path&)> onEntryImported; 

		/** @name Internal 
		 *  @{
		 */

		/**	Returns the resource manifest managed by the project library. */
		const SPtr<ResourceManifest>& _getManifest() const { return mResourceManifest; }

		/** @} */

		static const Path RESOURCES_DIR;
		static const Path INTERNAL_RESOURCES_DIR;
	private:
		/**
		 * Common code for adding a new resource entry to the library.
		 *
		 * @param[in]	parent			Parent of the new entry.
		 * @param[in]	filePath		Absolute path to the resource.
		 * @param[in]	importOptions	Optional import options to use when importing the resource. Caller must ensure the
		 *								import options are of the correct type for the resource in question. If null is 
		 *								provided default import options are used.
		 * @param[in]	forceReimport	Should the resource be reimported even if we detect no changes. This should be true
		 *								if import options changed since last import.
		 * @return						Newly added resource entry.
		 */
		FileEntry* addResourceInternal(DirectoryEntry* parent, const Path& filePath, 
			const SPtr<ImportOptions>& importOptions = nullptr, bool forceReimport = false);

		/**
		 * Common code for adding a new folder entry to the library.
		 *
		 * @param[in]	parent	Parent of the new entry.
		 * @param[in]	dirPath	Absolute path to the directory.
		 * @return		Newly added directory entry.
		 */
		DirectoryEntry* addDirectoryInternal(DirectoryEntry* parent, const Path& dirPath);

		/**
		 * Common code for deleting a resource from the library. This code only removes the library entry, not the actual
		 * resource file.
		 *
		 * @param[in]	resource	Entry to delete.
		 */
		void deleteResourceInternal(FileEntry* resource);

		/**
		 * Common code for deleting a directory from the library. This code only removes the library entry, not the actual
		 * directory.
		 *
		 * @param[in]	resource	Entry to delete.
		 */
		void deleteDirectoryInternal(DirectoryEntry* directory);

		/**
		 * Triggers a reimport of a resource using the provided import options, if needed. Doesn't import dependencies.
		 *
		 * @param[in]	path				Absolute Path to the resource to reimport.
		 * @param[in]	importOptions		Optional import options to use when importing the resource. Caller must ensure 
		 *									the import options are of the correct type for the resource in question. If null
		 *									is provided default import options are used.
		 * @param[in]	forceReimport		Should the resource be reimported even if we detect no changes. This should be 
		 *									true if import options changed since last import.
		 * @param[in]	pruneResourceMetas	Determines should resource meta data for resources that no longer exist within
		 *									the file be deleted. Default behaviour is to keep these alive so that if their
		 *									resources are eventually restored, references to them will remain valid. If you
		 *									feel that you need to clear this data, set this to true but be aware that you
		 *									might need to re-apply those references.
		 */
		void reimportResourceInternal(FileEntry* file, const SPtr<ImportOptions>& importOptions = nullptr, 
			bool forceReimport = false, bool pruneResourceMetas = false);

		/**
		 * Creates a full hierarchy of directory entries up to the provided directory, if any are needed.
		 *
		 * @param[in]	fullPath			Absolute path to a directory we are creating the hierarchy to.
		 * @param[in]	newHierarchyRoot	First directory entry that already existed in our hierarchy.
		 * @param[in]	newHierarchyLeaf	Leaf entry corresponding to the exact entry at \p path.
		 */
		void createInternalParentHierarchy(const Path& fullPath, DirectoryEntry** newHierarchyRoot, DirectoryEntry** newHierarchyLeaf);

		/**	Checks has a file been modified since the last import. */
		bool isUpToDate(FileEntry* file) const;

		/**	Checks is the resource a native engine resource that doesn't require importing. */
		bool isNative(const Path& path) const;

		/**
		 * Returns a path to a .meta file based on the resource path.
		 *
		 * @param[in]	path	Absolute path to the resource.
		 * @return				Path to the .meta file.
		 */
		Path getMetaPath(const Path& path) const;
		
		/**	Checks does the path represent a .meta file. */
		bool isMeta(const Path& fullPath) const;

		/**
		 * Returns a set of resource paths that are dependent on the provided resource entry. (for example a shader file
		 * might be dependent on shader include file).
		 */
		Vector<Path> getImportDependencies(const FileEntry* entry);

		/**	Registers any import dependencies for the specified resource. */
		void addDependencies(const FileEntry* entry);

		/**	Removes any import dependencies for the specified resource. */
		void removeDependencies(const FileEntry* entry);

		/**	Finds dependants resource for the specified resource entry and reimports them. */
		void reimportDependants(const Path& entryPath);

		/**	Makes all library entry paths relative to the current resources folder. */
		void makeEntriesRelative();

		/**
		 * Makes all library entry paths absolute by appending them to the current resources folder. Entries must have
		 * previously been made relative by calling makeEntriesRelative().
		 */
		void makeEntriesAbsolute();

		/** Deletes all library entries. */
		void clearEntries();

		static const WString LIBRARY_ENTRIES_FILENAME;
		static const WString RESOURCE_MANIFEST_FILENAME;

		SPtr<ResourceManifest> mResourceManifest;
		DirectoryEntry* mRootEntry;
		Path mProjectFolder;
		Path mResourcesFolder;
		bool mIsLoaded;

		UnorderedMap<Path, Vector<Path>> mDependencies;
		UnorderedMap<String, Path> mUUIDToPath;
	};

	/**	Provides easy access to ProjectLibrary. */
	BS_ED_EXPORT ProjectLibrary& gProjectLibrary();

	/** @} */
}