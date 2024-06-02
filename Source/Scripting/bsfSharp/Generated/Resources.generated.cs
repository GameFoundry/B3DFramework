//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Resources
	 *  @{
	 */

	/// <summary>
	/// Manager for dealing with all engine resources. It allows you to save new resources and load existing ones.
	/// </summary>
	[ShowInInspector]
	public partial class Resources : ScriptObject
	{
		private Resources(bool __dummy0) { }
		protected Resources() { }

		/// <summary>Called when the resource has been successfully loaded.</summary>
		public static event Action<RRefBase> OnResourceLoaded;

		/// <summary>Called when the resource has been destroyed. Provides UUID of the destroyed resource.</summary>
		public static event Action<UUID> OnResourceDestroyed;

		/// <summary>Called when the internal resource the handle is pointing to has changed.</summary>
		public static event Action<RRefBase> OnResourceModified;

		/// <summary>
		/// Loads the resource from a given path. Returns an empty handle if resource can&apos;t be loaded. Resource is loaded 
		/// synchronously.
		/// </summary>
		/// <param name="filePath">
		/// File path to the resource to load. This can be absolute or relative to the working folder.
		/// </param>
		/// <param name="loadFlags">Flags used to control the load process.</param>
		public static Resource Load(string filePath, ResourceLoadFlag loadFlags = ResourceLoadFlag.Default)
		{
			return Internal_Load(filePath, loadFlags);
		}

		/// <summary>
		/// Loads the resource asynchronously. Initially returned resource handle will be invalid until resource loading is done. 
		/// Use ResourceHandle&lt;T&gt;::isLoaded to check if resource has been loaded, or 
		/// ResourceHandle&lt;T&gt;::blockUntilLoaded to wait until load completes.
		/// </summary>
		/// <param name="filePath">Full pathname of the file.</param>
		/// <param name="loadFlags">Flags used to control the load process.</param>
		public static RRefBase LoadAsync(string filePath, ResourceLoadFlag loadFlags = ResourceLoadFlag.Default)
		{
			return Internal_LoadAsync(filePath, loadFlags);
		}

		/// <summary>Loads the resource with the given UUID. Returns an empty handle if resource can&apos;t be loaded.</summary>
		/// <param name="uuid">UUID of the resource to load.</param>
		/// <param name="async">
		/// If true resource will be loaded asynchronously. Handle to non-loaded resource will be returned immediately while 
		/// loading will continue in the background.
		/// </param>
		/// <param name="loadFlags">Flags used to control the load process.</param>
		public static RRefBase LoadFromUuid(UUID uuid, bool async = false, ResourceLoadFlag loadFlags = ResourceLoadFlag.Default)
		{
			return Internal_LoadFromUuid(ref uuid, async, loadFlags);
		}

		/// <summary>
		/// Releases an internal reference to the resource held by the resources system. This allows the resource to be unloaded 
		/// when it goes out of scope, if the resource was loaded with <see cref="KeepInternalReference"/> option.
		///
		/// Alternatively you can also skip manually calling ReleaseInternalReference() and call UnloadAllUnused() which will 
		/// unload all resources that do not have any external references, but you lose the fine grained control of what will be 
		/// unloaded.
		/// </summary>
		public static void ReleaseInternalReference(RRefBase resource)
		{
			Internal_ReleaseInternalReference(resource);
		}

		/// <summary>Finds all resources that aren&apos;t being referenced outside of the resources system and unloads them.</summary>
		public static void UnloadAllUnused()
		{
			Internal_UnloadAllUnused();
		}

		/// <summary>Forces unload of all resources, whether they are being used or not.</summary>
		public static void UnloadAll()
		{
			Internal_UnloadAll();
		}

		/// <summary>Saves the resource at the specified location.</summary>
		/// <param name="resource">Handle to the resource.</param>
		/// <param name="filePath">Full pathname of the file to save as.</param>
		/// <param name="overwrite">If true, any existing resource at the specified location will be overwritten.</param>
		/// <param name="compress">
		/// Should the resource be compressed before saving. Some resources have data that is already compressed and this option 
		/// will be ignored for such resources.
		/// </param>
		public static void Save(Resource resource, string filePath, bool overwrite, bool compress = false)
		{
			Internal_Save(resource, filePath, overwrite, compress);
		}

		/// <summary>Saves an existing resource to its previous location.</summary>
		/// <param name="resource">Handle to the resource.</param>
		/// <param name="compress">
		/// Should the resource be compressed before saving. Some resources have data that is already compressed and this option 
		/// will be ignored for such resources.
		/// </param>
		public static void Save(Resource resource, bool compress = false)
		{
			Internal_Save0(resource, compress);
		}

		/// <summary>
		/// Returns a list of dependencies from the resources at the specified path. Resource will not be loaded or parsed, but 
		/// instead the saved list of dependencies will be read from the file and returned.
		/// </summary>
		/// <param name="filePath">Full path to the resource to get dependencies for.</param>
		/// <returns>List of dependencies represented as UUIDs.</returns>
		public static UUID[] GetDependencies(string filePath)
		{
			return Internal_GetDependencies(filePath);
		}

		/// <summary>Checks is the resource with the specified UUID loaded.</summary>
		/// <param name="uuid">UUID of the resource to check.</param>
		/// <param name="checkInProgress">
		/// Should this method also check resources that are in progress of being asynchronously loaded.
		/// </param>
		/// <returns>True if loaded or loading in progress, false otherwise.</returns>
		public static bool IsLoaded(UUID uuid, bool checkInProgress = true)
		{
			return Internal_IsLoaded(ref uuid, checkInProgress);
		}

		/// <summary>Returns the loading progress of a resource that&apos;s being asynchronously loaded.</summary>
		/// <param name="resource">Resource whose load progress to check.</param>
		/// <param name="includeDependencies">
		/// If false the progress will reflect the load progress only for this inidividual resource. If true the progress will 
		/// reflect load progress of this resource and all of its dependencies.
		/// </param>
		/// <returns>Load progress in range [0, 1].</returns>
		public static float GetLoadProgress(RRefBase resource, bool includeDependencies = true)
		{
			return Internal_GetLoadProgress(resource, includeDependencies);
		}

		/// <summary>Returns the loading progress of a resource that&apos;s being loaded</summary>
		/// <param name="resource">Resource whose load progress to check.</param>
		/// <returns>Load progress in range [0, 1].</returns>
		public static float GetLoadProgress2(RRefBase resource)
		{
			return Internal_GetLoadProgress2(resource);
		}

		/// <summary>
		/// Allows you to set a resource manifest containing UUID &lt;-&gt; file path mapping that is used when resolving 
		/// resource references.
		/// </summary>
		public static void RegisterResourceManifest(ResourceManifest manifest)
		{
			Internal_RegisterResourceManifest(manifest);
		}

		/// <summary>Unregisters a resource manifest previously registered with registerResourceManifest().</summary>
		public static void UnregisterResourceManifest(ResourceManifest manifest)
		{
			Internal_UnregisterResourceManifest(manifest);
		}

		/// <summary>
		/// Allows you to retrieve resource manifest containing UUID &lt;-&gt; file path mapping that is used when resolving 
		/// resource references.
		/// </summary>
		public static ResourceManifest GetResourceManifest(string name)
		{
			return Internal_GetResourceManifest(name);
		}

		/// <summary>
		/// Attempts to retrieve file path from the provided UUID. Returns true if successful, false otherwise.
		/// </summary>
		public static bool GetFilePathFromUuid(UUID uuid, out string filePath)
		{
			return Internal_GetFilePathFromUuid(ref uuid, out filePath);
		}

		/// <summary>
		/// Attempts to retrieve UUID from the provided physical file path. Returns true if successful, false otherwise.
		/// </summary>
		public static bool GetUUIDFromFilePath(string path, out UUID outUUID)
		{
			return Internal_GetUUIDFromFilePath(path, out outUUID);
		}

		/// <summary>
		/// Converts a potentially virtual path into a physical one. If the path is not virtual, returns the path as-is.
		/// </summary>
		public static string EnsurePhysicalPath(string path)
		{
			return Internal_EnsurePhysicalPath(path);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Resource Internal_Load(string filePath, ResourceLoadFlag loadFlags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRefBase Internal_LoadAsync(string filePath, ResourceLoadFlag loadFlags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRefBase Internal_LoadFromUuid(ref UUID uuid, bool async, ResourceLoadFlag loadFlags);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ReleaseInternalReference(RRefBase resource);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnloadAllUnused();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnloadAll();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Save(Resource resource, string filePath, bool overwrite, bool compress);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Save0(Resource resource, bool compress);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern UUID[] Internal_GetDependencies(string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsLoaded(ref UUID uuid, bool checkInProgress);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetLoadProgress(RRefBase resource, bool includeDependencies);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetLoadProgress2(RRefBase resource);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RegisterResourceManifest(ResourceManifest manifest);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnregisterResourceManifest(ResourceManifest manifest);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ResourceManifest Internal_GetResourceManifest(string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetFilePathFromUuid(ref UUID uuid, out string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_GetUUIDFromFilePath(string path, out UUID outUUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_EnsurePhysicalPath(string path);
		private static void Internal_OnResourceLoaded(RRefBase p0)
		{
			OnResourceLoaded?.Invoke(p0);
		}
		private static void Internal_OnResourceDestroyed(ref UUID p0)
		{
			OnResourceDestroyed?.Invoke(p0);
		}
		private static void Internal_OnResourceModified(RRefBase p0)
		{
			OnResourceModified?.Invoke(p0);
		}
	}

	/** @} */
#endif
}
