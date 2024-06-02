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

	/// <summary>Serializable class that contains UUID &lt;-&gt; file path mapping for resources.</summary>
	[ShowInInspector]
	public partial class ResourceManifest : ScriptObject
	{
		private ResourceManifest(bool __dummy0) { }
		protected ResourceManifest() { }

		/// <summary>Creates a new empty resource manifest. Provided name should be unique among manifests.</summary>
		public ResourceManifest(string name)
		{
			Internal_Create(this, name);
		}

		/// <summary>Returns an unique name of the resource manifest.</summary>
		[NativeWrapper]
		public string Name
		{
			get { return Internal_GetName(mCachedPtr); }
		}

		/// <summary>Registers a new resource in the manifest.</summary>
		public void RegisterResource(UUID uuid, string filePath)
		{
			Internal_RegisterResource(mCachedPtr, ref uuid, filePath);
		}

		/// <summary>Removes a resource from the manifest.</summary>
		public void UnregisterResource(UUID uuid)
		{
			Internal_UnregisterResource(mCachedPtr, ref uuid);
		}

		/// <summary>
		/// Attempts to find a resource with the provided UUID and outputs the absolute physical path to the resource if found. 
		/// Returns true if UUID was found, false otherwise.
		/// </summary>
		public bool UUIDToPhysicalFilePath(UUID uuid, out string filePath)
		{
			return Internal_UUIDToPhysicalFilePath(mCachedPtr, ref uuid, out filePath);
		}

		/// <summary>
		/// Attempts to find a resource with the provided physical path and outputs the UUID to the resource if found. Returns 
		/// true if path was found, false otherwise.
		/// </summary>
		public bool PhysicalFilePathToUUID(string filePath, out UUID outUUID)
		{
			return Internal_PhysicalFilePathToUUID(mCachedPtr, filePath, out outUUID);
		}

		/// <summary>
		/// Attempts to find a resource with the provided virtual path and outputs the UUID to the resource if found. Returns 
		/// true if path was found, false otherwise.
		/// </summary>
		public bool VirtualFilePathToUUID(string filePath, out UUID outUUID)
		{
			return Internal_VirtualFilePathToUUID(mCachedPtr, filePath, out outUUID);
		}

		/// <summary>Checks if provided UUID exists in the manifest.</summary>
		public bool UuidExists(UUID uuid)
		{
			return Internal_UuidExists(mCachedPtr, ref uuid);
		}

		/// <summary>Checks if the provided path exists in the manifest.</summary>
		public bool FilePathExists(string filePath)
		{
			return Internal_FilePathExists(mCachedPtr, filePath);
		}

		/// <summary>
		/// Converts a virtual path to a physical one. Note the existance of the path in the manifest will not be checked, the 
		/// only requirement is that the virtual path begins with virtual path prefix. This means you can use this method for 
		/// translating virtual -&gt; physical paths before they are added to the manifest. Returns false if <paramref 
		/// name="virtualPath"/> doesn&apos;t start with the manifest&apos;s virtual path prefix.
		/// </summary>
		public bool VirtualToPhysicalPath(string virtualPath, out string outPhysicalPath)
		{
			return Internal_VirtualToPhysicalPath(mCachedPtr, virtualPath, out outPhysicalPath);
		}

		/// <summary>Saves the resource manifest to the specified location.</summary>
		/// <param name="manifest">Manifest to save.</param>
		/// <param name="path">Full pathname of the file to save the manifest in.</param>
		/// <param name="physicalPathPrefix">
		/// If not empty, all pathnames in the manifest will be stored as if relative to this path.
		/// </param>
		public static void Save(ResourceManifest manifest, string path, string physicalPathPrefix)
		{
			Internal_Save(manifest, path, physicalPathPrefix);
		}

		/// <summary>Loads the resource manifest from the specified location.</summary>
		/// <param name="path">Full pathname of the file to load the manifest from.</param>
		/// <param name="physicalPathPrefix">If not empty, all loaded path names will have this path prepended.</param>
		/// <param name="virtualPathPrefix">
		/// If not empty, adds an additional set of paths that the resource can be referenced from.
		/// </param>
		public static ResourceManifest Load(string path, string physicalPathPrefix, string virtualPathPrefix)
		{
			return Internal_Load(path, physicalPathPrefix, virtualPathPrefix);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetName(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RegisterResource(IntPtr thisPtr, ref UUID uuid, string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnregisterResource(IntPtr thisPtr, ref UUID uuid);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_UUIDToPhysicalFilePath(IntPtr thisPtr, ref UUID uuid, out string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_PhysicalFilePathToUUID(IntPtr thisPtr, string filePath, out UUID outUUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_VirtualFilePathToUUID(IntPtr thisPtr, string filePath, out UUID outUUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_UuidExists(IntPtr thisPtr, ref UUID uuid);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_FilePathExists(IntPtr thisPtr, string filePath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_VirtualToPhysicalPath(IntPtr thisPtr, string virtualPath, out string outPhysicalPath);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Save(ResourceManifest manifest, string path, string physicalPathPrefix);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ResourceManifest Internal_Load(string path, string physicalPathPrefix, string virtualPathPrefix);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ResourceManifest managedInstance, string name);
	}

	/** @} */
#endif
}
