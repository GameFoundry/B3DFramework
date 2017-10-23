//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "Library/BsProjectLibrary.h"
#include "Wrappers/BsScriptResource.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ProjectLibrary. */
	class BS_SCR_BED_EXPORT ScriptProjectLibrary : public ScriptObject<ScriptProjectLibrary>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ProjectLibrary")

		/**	Initializes the project library callbacks. Must be called on library load. */
		void static startUp();

		/**	Cleans up project library callbacks. Must be called before library shutdown. */
		void static shutDown();

	private:
		ScriptProjectLibrary(MonoObject* instance);

		/**
		 * Triggered when a new entry has been added to the library.
		 *
		 * @param[in]	path	Absolute path to the new entry.
		 */
		static void onEntryAdded(const Path& path);

		/**
		 * Triggered when a new entry has been removed to the library.
		 *
		 * @param[in]	path	Absolute path to the removed entry.
		 */
		static void onEntryRemoved(const Path& path);

		/**
		 * Triggered when an entry was (re) imported in the library.
		 *
		 * @param[in]	path	Absolute path to the imported entry.
		 */
		static void onEntryImported(const Path& path);

		static HEvent mOnEntryAddedConn;
		static HEvent mOnEntryRemovedConn;
		static HEvent mOnEntryImportedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		typedef void(BS_THUNKCALL *OnEntryChangedThunkDef) (MonoString*, MonoException**);

		static OnEntryChangedThunkDef OnEntryAddedThunk;
		static OnEntryChangedThunkDef OnEntryRemovedThunk;
		static OnEntryChangedThunkDef OnEntryImportedThunk;

		static MonoArray* internal_Refresh(MonoString* path, bool import);
		static void internal_Create(MonoObject* resource, MonoString* path);
		static MonoObject* internal_Load(MonoString* path);
		static void internal_Save(MonoObject* resource);
		static MonoObject* internal_GetRoot();
		static void internal_Reimport(MonoString* path, MonoObject* options, bool force);
		static MonoObject* internal_GetEntry(MonoString* path);
		static bool internal_IsSubresource(MonoString* path);
		static MonoObject* internal_GetMeta(MonoString* path);
		static MonoString* internal_GetPathFromUUID(MonoString* uuid);
		static MonoString* internal_GetPath(MonoObject* resource);
		static MonoArray* internal_Search(MonoString* pattern, MonoArray* types);
		static void internal_Delete(MonoString* path);
		static void internal_CreateFolder(MonoString* path);
		static void internal_Rename(MonoString* path, MonoString* name, bool overwrite);
		static void internal_Move(MonoString* oldPath, MonoString* newPath, bool overwrite);
		static void internal_Copy(MonoString* source, MonoString* destination, bool overwrite);
		static MonoString* internal_GetResourceFolder();
		static void internal_SetIncludeInBuild(MonoString* path, bool include);
		static void internal_SetEditorData(MonoString* path, MonoObject* userData);
	};

	/**	Base class for C++/CLR interop objects used for wrapping LibraryEntry implementations. */
	class BS_SCR_BED_EXPORT ScriptLibraryEntryBase : public ScriptObjectBase
	{
	public:
		/**	Returns the asset path of the library entry. */
		const Path& getAssetPath() const { return mAssetPath; }

	protected:
		ScriptLibraryEntryBase(MonoObject* instance);
		virtual ~ScriptLibraryEntryBase() {}

		Path mAssetPath;
	};

	/**	Interop class between C++ & CLR for LibraryEntry. */
	class BS_SCR_BED_EXPORT ScriptLibraryEntry : public ScriptObject <ScriptLibraryEntry>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "LibraryEntry")

		ScriptLibraryEntry(MonoObject* instance);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* internal_GetPath(ScriptLibraryEntryBase* thisPtr);
		static MonoString* internal_GetName(ScriptLibraryEntryBase* thisPtr);
		static ProjectLibrary::LibraryEntryType internal_GetType(ScriptLibraryEntryBase* thisPtr);
		static MonoObject* internal_GetParent(ScriptLibraryEntryBase* thisPtr);
	};

	/**	Interop class between C++ & CLR for DirectoryEntry. */
	class BS_SCR_BED_EXPORT ScriptDirectoryEntry : public ScriptObject <ScriptDirectoryEntry, ScriptLibraryEntryBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "DirectoryEntry")

		ScriptDirectoryEntry(MonoObject* instance, const Path& assetPath);

		/** Creates a new interop object that wraps the provided native directory entry object. */
		static MonoObject* create(const ProjectLibrary::DirectoryEntry* entry);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoArray* internal_GetChildren(ScriptDirectoryEntry* thisPtr);
	};

	/**	Interop class between C++ & CLR for ResourceEntry. */
	class BS_SCR_BED_EXPORT ScriptFileEntry : public ScriptObject <ScriptFileEntry, ScriptLibraryEntryBase>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "FileEntry")

		ScriptFileEntry(MonoObject* instance, const Path& assetPath);

		/** Creates a new interop object that wraps the provided native resource entry object. */
		static MonoObject* create(const ProjectLibrary::FileEntry* entry);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_GetImportOptions(ScriptFileEntry* thisPtr);
		static MonoArray* internal_GetResourceMetas(ScriptFileEntry* thisPtr);
		static bool internal_GetIncludeInBuild(ScriptFileEntry* thisPtr);
	};

	/**	Interop class between C++ & CLR for ResourceMeta. */
	class BS_SCR_BED_EXPORT ScriptResourceMeta : public ScriptObject <ScriptResourceMeta>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "ResourceMeta")

		ScriptResourceMeta(MonoObject* instance, const SPtr<ProjectResourceMeta>& meta);

		/** Creates a new interop object that wraps the native resource meta object. */
		static MonoObject* create(const SPtr<ProjectResourceMeta>& meta);

	private:
		SPtr<ProjectResourceMeta> mMeta;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* internal_GetUUID(ScriptResourceMeta* thisPtr);
		static MonoString* internal_GetSubresourceName(ScriptResourceMeta* thisPtr);
		static MonoObject* internal_GetIcon(ScriptResourceMeta* thisPtr);
		static ScriptResourceType internal_GetResourceType(ScriptResourceMeta* thisPtr);
		static MonoObject* internal_GetEditorData(ScriptResourceMeta* thisPtr);
	};

	/** @} */
}