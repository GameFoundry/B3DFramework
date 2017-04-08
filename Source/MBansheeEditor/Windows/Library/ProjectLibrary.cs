//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// The primary location for interacting with all the resources in the current project. A complete hierarchy of 
    /// resources is provided which can be interacted with by importing new ones, deleting them, moving, renaming and similar.
    /// </summary>
    public sealed class ProjectLibrary : ScriptObject
    {
        /// <summary>
        /// Root entry of the project library, referencing the top level resources folder.
        /// </summary>
        public static DirectoryEntry Root { get { return Internal_GetRoot(); } }

        /// <summary>
        /// Absolute path to the current project's project library resource folder.
        /// </summary>
        public static string ResourceFolder { get { return Internal_GetResourceFolder(); } }

        /// <summary>
        /// Triggered when a new entry is added to the project library. Provided path relative to the project library 
        /// resources folder.
        /// </summary>
        public static event Action<string> OnEntryAdded;

        /// <summary>
        /// Triggered when an entry is removed from the project library. Provided path relative to the project library 
        /// resources folder.
        /// </summary>
        public static event Action<string> OnEntryRemoved;

        /// <summary>
        /// Triggered when an entry is (re)imported in the project library. Provided path relative to the project library 
        /// resources folder.
        /// </summary>
        public static event Action<string> OnEntryImported;

        /// <summary>
        /// Checks wheher an asset import is currently in progress.
        /// </summary>
        internal static bool ImportInProgress { get { return importInProgress; } }

        private static HashSet<string> queuedForImport = new HashSet<string>();
        private static int numImportedFiles;
        private static int totalFilesToImport;
        private static bool importInProgress;

        private const float TIME_SLICE_SECONDS = 0.030f;

        /// <summary>
        /// Checks the project library folder for any modifications and reimports the required resources.
        /// </summary>
        /// <param name="synchronous">If true this method will block until the project library has done refreshing, 
        ///                           otherwise the refresh will happen over the course of this and next frames.</param>
        public static void Refresh(bool synchronous = false)
        {
            string[] modifiedPaths = Internal_Refresh(ResourceFolder, synchronous);

            if (!synchronous)
            {
                foreach (var modifiedPath in modifiedPaths)
                {
                    if (queuedForImport.Add(modifiedPath))
                        totalFilesToImport++;
                }
            }
            else
            {
                foreach (var path in queuedForImport)
                    Internal_Refresh(path, true);

                queuedForImport.Clear();
                numImportedFiles = 0;
                totalFilesToImport = 0;
            }
        }

        /// <summary>
        /// Checks the specified folder for any modifications and reimports the required resources.
        /// </summary>
        /// <param name="path">Path to a file or folder to refresh. Relative to the project library resources folder or 
        ///                    absolute.</param>
        public static void Refresh(string path)
        {
            string[] modifiedPaths = Internal_Refresh(path, false);
            foreach (var modifiedPath in modifiedPaths)
            {
                if (queuedForImport.Add(modifiedPath))
                    totalFilesToImport++;
            }
        }

        /// <summary>
        /// Registers a new resource in the library.
        /// </summary>
        /// <param name="resource">Resource instance to add to the library. A copy of the resource will be saved at the 
        ///                        provided path.</param>
        /// <param name="path">Path where where to store the resource. Absolute or relative to the resources folder.</param>
        public static void Create(Resource resource, string path)
        {
            Internal_Create(resource, path);
        }

        /// <summary>
        /// Updates a resource that is already in the library.
        /// </summary>
        /// <param name="resource">Resource to save.</param>
        public static void Save(Resource resource)
        {
            Internal_Save(resource);
        }

        /// <summary>
        /// Loads a resource from the project library.
        /// </summary>
        /// <typeparam name="T">Type of the resource to load.</typeparam>
        /// <param name="path">Path of the resource to load. Absolute or relative to the resources folder. If a 
        ///                    sub-resource within a file is needed, append the name of the subresource to the path ( 
        ///                    for example mymesh.fbx/my_animation).</param>
        /// <returns>Instance of the loaded resource, or null if not found.</returns>
        public static T Load<T>(string path) where T : Resource
        {
            return (T) Internal_Load(path);
        }

        /// <summary>
        /// Triggers a reimport of a resource using the provided import options, if needed.
        /// </summary>
        /// <param name="path">Path to the resource to reimport, absolute or relative to resources folder.</param>
        /// <param name="options">ptional import options to use when importing the resource. Caller must ensure the import
        ///                       options are of the correct type for the resource in question. If null is provided default 
        ///                       import options are used.</param>
        /// <param name="force">Should the resource be reimported even if no changes are detected.</param>
        public static void Reimport(string path, ImportOptions options = null, bool force = false)
        {
            Internal_Reimport(path, options, force);
        }

        /// <summary>
        /// Checks does the project library contain a file or folder at the specified path.
        /// </summary>
        /// <param name="path">Path to the file/folder to check, absolute or relative to resources folder.</param>
        /// <returns>True if the element exists, false otherwise.</returns>
        public static bool Exists(string path)
        {
            return GetEntry(path) != null;
        }

        /// <summary>
        /// Attempts to locate a library entry that describes a file or a folder in the project library.
        /// </summary>
        /// <param name="path">Path to the entry to retrieve, absolute or relative to resources folder.</param>
        /// <returns>Library entry if found, null otherwise. This object can become invalid on the next library refresh
        ///          and you are not meant to hold a permanent reference to it.</returns>
        public static LibraryEntry GetEntry(string path)
        {
            return Internal_GetEntry(path);
        }

        /// <summary>
        /// Checks whether the provided path points to a sub-resource. Sub-resource is any resource that is not the
        /// primary resource in the file.
        /// </summary>
        /// <param name="path">Path to the entry, absolute or relative to resources folder.</param>
        /// <returns>True if the path is a sub-resource, false otherwise.</returns>
        public static bool IsSubresource(string path)
        {
            return Internal_IsSubresource(path);
        }

        /// <summary>
        /// Attempts to locate meta-data for a resource at the specified path.
        /// </summary>
        /// <param name="path">Path to the entry to retrieve, absolute or relative to resources folder. If a sub-resource 
        ///                    within a file is needed, append the name of the subresource to the path (for example
        ///                    mymesh.fbx/my_animation).</param>
        /// <returns>Resource meta-data if the resource was found, null otherwise.</returns>
        public static ResourceMeta GetMeta(string path)
        {
            return Internal_GetMeta(path);
        }

        /// <summary>
        /// Searches the library for a pattern and returns all entries matching it.
        /// </summary>
        /// <param name="pattern">Pattern to search for. Use wildcard * to match any character(s).</param>
        /// <param name="types">Type of resources to search for. If null all entries will be searched.</param>
        /// <returns>A set of entries matching the pattern. These objects can become invalid on the next library refresh
        ///          and you are not meant to hold a permanent reference to them.</returns>
        public static LibraryEntry[] Search(string pattern, ResourceType[] types = null)
        {
            return Internal_Search(pattern, types);
        }

        /// <summary>
        /// Returns a path to a resource stored in the project library.
        /// </summary>
        /// <param name="resource">Resource to find the path for.</param>
        /// <returns>Path to relative to the project library resources folder if resource was found, null otherwise.
        ///          </returns>
        public static string GetPath(Resource resource)
        {
            return Internal_GetPath(resource);
        }

        /// <summary>
        /// Returns a path to a resource with the specified UUID stored in the project library.
        /// </summary>
        /// <param name="uuid">Unique identifier of the resources to retrieve the path of.</param>
        /// <returns>Path to relative to the project library resources folder if resource was found, null otherwise.
        ///          </returns>
        public static string GetPath(string uuid)
        {
            return Internal_GetPathFromUUID(uuid);
        }

        /// <summary>
        /// Deletes a resource in the project library.
        /// </summary>
        /// <param name="path">Path to the entry to delete, absolute or relative to resources folder.</param>
        public static void Delete(string path)
        {
            Internal_Delete(path);
        }

        /// <summary>
        /// Creates a new folder in the library.
        /// </summary>
        /// <param name="path">Path of the folder to create. Absolute or relative to the resources folder.</param>
        public static void CreateFolder(string path)
        {
            Internal_CreateFolder(path);
        }

        /// <summary>
        /// Renames an entry in the project library.
        /// </summary>
        /// <param name="path">Path of the entry to rename, absolute or relative to resources folder.</param>
        /// <param name="name">New name of the entry with an extension.</param>
        /// <param name="overwrite">Determines should the entry be deleted if one with the provided name already exists. If
        ///                         this is false and an entry already exists, no rename operation will be performed.</param>
        public static void Rename(string path, string name, bool overwrite = false)
        {
            Internal_Rename(path, name, false);
        }

        /// <summary>
        /// Moves an entry in the project library from one path to another.
        /// </summary>
        /// <param name="oldPath">Source path of the entry, absolute or relative to resources folder.</param>
        /// <param name="newPath">Destination path of the entry, absolute or relative to resources folder.</param>
        /// <param name="overwrite">Determines should the entry be deleted if one at the destination path already exists. If
        ///                         this is false and an entry already exists, no move operation will be performed.</param>
        public static void Move(string oldPath, string newPath, bool overwrite = false)
        {
            Internal_Move(oldPath, newPath, overwrite);
        }

        /// <summary>
        /// Copies an entry in the project library from one path to another.
        /// </summary>
        /// <param name="source">Source path of the entry, absolute or relative to resources folder.</param>
        /// <param name="destination">Destination path of the entry, absolute or relative to resources folder.</param>
        /// <param name="overwrite">Determines should the entry be deleted if one at the destination path already exists. If
        ///                         this is false and an entry already exists, no copy operation will be performed.</param>
        public static void Copy(string source, string destination, bool overwrite = false)
        {
            Internal_Copy(source, destination, overwrite);
        }

        /// <summary>
        /// Controls should a resource be included an a build. All dependant resources will also be included.
        /// </summary>
        /// <param name="path">Path of the resource to include, absolute or relative to resources folder.</param>
        /// <param name="include">True if it should be included, false otherwise.</param>
        public static void SetIncludeInBuild(string path, bool include)
        {
            Internal_SetIncludeInBuild(path, include);
        }

        /// <summary>
        /// Assigns a non-specific editor data object to the resource at the specified path. 
        /// </summary>
        /// <param name="path">Path to the resource to modify, absolute or relative to resources folder.</param>
        /// <param name="userData">Data to assign to the resource, which can later be retrieved from the resource's
        /// meta-data as needed. </param>
        public static void SetEditorData(string path, object userData)
        {
            Internal_SetEditorData(path, userData);
        }

        /// <summary>
        /// Triggers reimport for queued resource. Should be called once per frame.
        /// </summary>
        internal static void Update()
        {
            if (queuedForImport.Count > 0)
            {
                // Skip first frame to get the progress bar a chance to show up
                if (importInProgress)
                {
                    UInt64 start = Time.Precise;
                    List<string> toRemove = new List<string>();

                    foreach (var entry in queuedForImport)
                    {
                        Internal_Refresh(entry, true);
                        toRemove.Add(entry);
                        numImportedFiles++;

                        UInt64 end = Time.Precise;
                        UInt64 elapsed = end - start;

                        float elapsedSeconds = elapsed * Time.MicroToSecond;
                        if (elapsedSeconds > TIME_SLICE_SECONDS)
                            break;
                    }

                    foreach (var entry in toRemove)
                        queuedForImport.Remove(entry);
                }

                if (queuedForImport.Count == 0)
                {
                    numImportedFiles = 0;
                    totalFilesToImport = 0;

                    ProgressBar.Hide();
                }
                else
                {
                    IEnumerator<string> enumerator = queuedForImport.GetEnumerator();
                    enumerator.MoveNext();

                    string displayName = enumerator.Current;
                    displayName = displayName.Replace("\\", "\\\\");

                    if (displayName.Length > 60)
                    {
                        displayName = displayName.Remove(0, displayName.Length - 60);
                        displayName = "..." + displayName;
                    }

                    float pct = numImportedFiles / (float)totalFilesToImport;
                    ProgressBar.Show("Importing (" + numImportedFiles + "/" + totalFilesToImport + ")", displayName, pct);
                }

                importInProgress = true;
            }
            else
                importInProgress = false;
        }

        /// <summary>
        /// Triggered internally by the runtime when a new entry is added to the project library.
        /// </summary>
        /// <param name="path">Path relative to the project library resources folder.</param>
        private static void Internal_DoOnEntryAdded(string path)
        {
            if (OnEntryAdded != null)
                OnEntryAdded(path);
        }

        /// <summary>
        /// Triggered internally by the runtime when an entry is removed from the project library.
        /// </summary>
        /// <param name="path">Path relative to the project library resources folder.</param>
        private static void Internal_DoOnEntryRemoved(string path)
        {
            if (OnEntryRemoved != null)
                OnEntryRemoved(path);
        }

        /// <summary>
        /// Triggered internally by the runtime when an entry is (re)imported in the project library.
        /// </summary>
        /// <param name="path">Path relative to the project library resources folder.</param>
        private static void Internal_DoOnEntryImported(string path)
        {
            if (OnEntryImported != null)
                OnEntryImported(path);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string[] Internal_Refresh(string path, bool import);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Create(Resource resource, string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Resource Internal_Load(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Save(Resource resource);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern DirectoryEntry Internal_GetRoot();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsSubresource(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Reimport(string path, ImportOptions options, bool force);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern LibraryEntry Internal_GetEntry(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ResourceMeta Internal_GetMeta(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern LibraryEntry[] Internal_Search(string path, ResourceType[] types);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetPath(Resource resource);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetPathFromUUID(string uuid);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Delete(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateFolder(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Rename(string path, string name, bool overwrite);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Move(string oldPath, string newPath, bool overwrite);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Copy(string source, string destination, bool overwrite);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetResourceFolder();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetIncludeInBuild(string path, bool force);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetEditorData(string path, object userData);
    }

    /// <summary>
    /// Type of project library entries.
    /// </summary>
    public enum LibraryEntryType // Note: Must match the C++ enum ProjectLibrary::LibraryEntryType
    {
        File, Directory
    }

    /// <summary>
    /// Type of resources supported by the project library.
    /// </summary>
    public enum ResourceType // Note: Must match the C++ enum ScriptResourceType
    {
        Texture, SpriteTexture, Mesh, Font, Shader, ShaderInclude, Material, Prefab, PlainText, 
        ScriptCode, StringTable, GUISkin, PhysicsMaterial, PhysicsMesh, AudioClip, AnimationClip, Undefined
    }

    /// <summary>
    /// A generic project library entry that may be a file or a folder.
    /// </summary>
    public class LibraryEntry : ScriptObject
    {
        /// <summary>
        /// Path of the library entry, relative to the project library resources folder.
        /// </summary>
        public string Path { get { return Internal_GetPath(mCachedPtr); } }

        /// <summary>
        /// Name of the library entry.
        /// </summary>
        public string Name { get { return Internal_GetName(mCachedPtr); } }

        /// <summary>
        /// Type of the library entry.
        /// </summary>
        public LibraryEntryType Type { get { return Internal_GetType(mCachedPtr); } }

        /// <summary>
        /// Directory entry that contains this entry. This may be null for the root entry.
        /// </summary>
        public DirectoryEntry Parent { get { return Internal_GetParent(mCachedPtr); } }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetPath(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetName(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern LibraryEntryType Internal_GetType(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern DirectoryEntry Internal_GetParent(IntPtr thisPtr);
    }

    /// <summary>
    /// A project library entry representing a directory that contains other entries.
    /// </summary>
    public class DirectoryEntry : LibraryEntry
    {
        /// <summary>
        /// A set of entries contained in this entry.
        /// </summary>
        public LibraryEntry[] Children { get { return Internal_GetChildren(mCachedPtr); } }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern LibraryEntry[] Internal_GetChildren(IntPtr thisPtr);
    }

    /// <summary>
    /// A library entry representing a file.
    /// </summary>
    public class FileEntry : LibraryEntry
    {
        /// <summary>
        /// Import options used for importing the resources in the file.
        /// </summary>
        public ImportOptions Options { get { return Internal_GetImportOptions(mCachedPtr); } }

        /// <summary>
        /// Returns meta-data for all resources part of the file represented by this object.
        /// </summary>
        public ResourceMeta[] ResourceMetas { get { return Internal_GetResourceMetas(mCachedPtr); } }

        /// <summary>
        /// Determines will the resources in the file be included in the project build.
        /// </summary>
        public bool IncludeInBuild { get { return Internal_GetIncludeInBuild(mCachedPtr); } }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ImportOptions Internal_GetImportOptions(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ResourceMeta[] Internal_GetResourceMetas(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetIncludeInBuild(IntPtr thisPtr);
    }

    /// <summary>
    /// Contains meta-data for a resource in the ProjectLibrary.
    /// </summary>
    public class ResourceMeta : ScriptObject
    {
        /// <summary>
        /// Unique identifier of the resource.
        /// </summary>
        public string UUID { get { return Internal_GetUUID(mCachedPtr); } }

        /// <summary>
        /// Returns a name of the subresources. Each resource within a file has a unique name.
        /// </summary>
        public string SubresourceName { get { return Internal_GetSubresourceName(mCachedPtr); } }

        /// <summary>
        /// Custom icon for the resource to display in the editor, if the resource has one.
        /// </summary>
        public Texture Icon { get { return Internal_GetIcon(mCachedPtr); } }

        /// <summary>
        /// Type of the resource referenced by this entry.
        /// </summary>
        public ResourceType ResType { get { return Internal_GetResourceType(mCachedPtr); } }

        /// <summary>
        /// Non-specific data assigned to the resource, available in editor only.
        /// </summary>
        public object EditorData { get { return Internal_GetEditorData(mCachedPtr); } }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetUUID(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetSubresourceName(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Texture Internal_GetIcon(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ResourceType Internal_GetResourceType(IntPtr thisPtr);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_GetEditorData(IntPtr thisPtr);
    }

    /** @} */
}
