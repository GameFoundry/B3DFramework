//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
#if !IS_B3D
	/** @addtogroup Resources
	 *  @{
	 */

	/// <summary>Serializable class that contains UUID <-> file path mapping for resources.</summary>
	[ShowInInspector]
	public partial class ResourceManifest : ScriptObject
	{
		private ResourceManifest(bool __dummy0) { }
		protected ResourceManifest() { }

		/// <summary>Creates a new empty resource manifest. Provided name should be unique among manifests.</summary>
		public ResourceManifest(string name)
		{
			Internal_create(this, name);
		}

		/// <summary>Returns an unique name of the resource manifest.</summary>
		[NativeWrapper]
		public string Name
		{
			get { return Internal_getName(mCachedPtr); }
		}

		/// <summary>Registers a new resource in the manifest.</summary>
		public void RegisterResource(UUID uuid, string filePath)
		{
			Internal_registerResource(mCachedPtr, ref uuid, filePath);
		}

		/// <summary>Removes a resource from the manifest.</summary>
		public void UnregisterResource(UUID uuid)
		{
			Internal_unregisterResource(mCachedPtr, ref uuid);
		}

		/// <summary>
		/// Attempts to find a resource with the provided UUID and outputs the path to the resource if found. Returns true if 
		/// UUID was found, false otherwise.
		/// </summary>
		public bool UuidToFilePath(UUID uuid, out string filePath)
		{
			return Internal_uuidToFilePath(mCachedPtr, ref uuid, out filePath);
		}

		/// <summary>
		/// Attempts to find a resource with the provided path and outputs the UUID to the resource if found. Returns true if 
		/// path was found, false otherwise.
		/// </summary>
		public bool FilePathToUUID(string filePath, out UUID outUUID)
		{
			return Internal_filePathToUUID(mCachedPtr, filePath, out outUUID);
		}

		/// <summary>Checks if provided UUID exists in the manifest.</summary>
		public bool UuidExists(UUID uuid)
		{
			return Internal_uuidExists(mCachedPtr, ref uuid);
		}

		/// <summary>Checks if the provided path exists in the manifest.</summary>
		public bool FilePathExists(string filePath)
		{
			return Internal_filePathExists(mCachedPtr, filePath);
		}

		/// <summary>Saves the resource manifest to the specified location.</summary>
		/// <param name="manifest">Manifest to save.</param>
		/// <param name="path">Full pathname of the file to save the manifest in.</param>
		/// <param name="relativePath">
		/// If not empty, all pathnames in the manifest will be stored as if relative to this  path.
		/// </param>
		public static void Save(ResourceManifest manifest, string path, string relativePath)
		{
			Internal_save(manifest, path, relativePath);
		}

		/// <summary>Loads the resource manifest from the specified location.</summary>
		/// <param name="path">Full pathname of the file to load the manifest from.</param>
		/// <param name="relativePath">If not empty, all loaded pathnames will have this path prepended.</param>
		public static ResourceManifest Load(string path, string relativePath)
		{
			return Internal_load(path, relativePath);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_getName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_registerResource(IntPtr thisPtr, ref UUID uuid, string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_unregisterResource(IntPtr thisPtr, ref UUID uuid);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_uuidToFilePath(IntPtr thisPtr, ref UUID uuid, out string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_filePathToUUID(IntPtr thisPtr, string filePath, out UUID outUUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_uuidExists(IntPtr thisPtr, ref UUID uuid);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_filePathExists(IntPtr thisPtr, string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_save(ResourceManifest manifest, string path, string relativePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ResourceManifest Internal_load(string path, string relativePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ResourceManifest managedInstance, string name);
	}

	/** @} */
#endif
}
