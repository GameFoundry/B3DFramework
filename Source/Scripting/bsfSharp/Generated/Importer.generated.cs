//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
#if !IS_B3D
	/** @addtogroup Importer
	 *  @{
	 */

	/// <summary>
	/// Module responsible for importing various asset types and converting them to types usable by the engine.
	/// </summary>
	[ShowInInspector]
	public partial class Importer : ScriptObject
	{
		private Importer(bool __dummy0) { }
		protected Importer() { }

		/// <summary>
		/// Imports a resource at the specified location, and returns the loaded data. If file contains more than one resource 
		/// only the primary resource is imported (for example an FBX a mesh would be imported, but animations ignored).
		/// </summary>
		/// <param name="inputFilePath">Pathname of the input file.</param>
		/// <param name="importOptions">
		/// (optional) Options for controlling the import. Caller must ensure import options  actually match the type of the 
		/// importer used for the file type.
		/// </param>
		/// <param name="UUID">
		/// Specific UUID to assign to the resource. If not specified a randomly generated UUID will be assigned.
		/// </param>
		/// <returns>Imported resource.</returns>
		public static Resource Import(string inputFilePath, ImportOptions importOptions, UUID UUID)
		{
			return Internal_import(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Same as import(), except it imports a resource without blocking the main thread. The resulting resource will be 
		/// placed in the returned AsyncOp object when the import ends.
		/// </summary>
		public static AsyncOp<RRefBase> ImportAsync(string inputFilePath, ImportOptions importOptions, UUID UUID)
		{
			return Internal_importAsync(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Imports a resource at the specified location, and returns the loaded data. This method returns all imported 
		/// resources, which is relevant for files that can contain multiple resources (for example an FBX which may contain both 
		/// a mesh and animations).
		/// </summary>
		/// <param name="inputFilePath">Pathname of the input file.</param>
		/// <param name="importOptions">
		/// (optional) Options for controlling the import. Caller must ensure import options  actually match the type of the 
		/// importer used for the file type.
		/// </param>
		/// <returns>A list of all imported resources. The primary resource is always the first returned resource.</returns>
		public static MultiResource ImportAll(string inputFilePath, ImportOptions importOptions)
		{
			return Internal_importAll(inputFilePath, importOptions);
		}

		/// <summary>
		/// Same as importAll(), except it imports a resource without blocking the main thread. The returned AsyncOp will contain 
		/// a list of the imported resources, after the import ends.
		/// </summary>
		public static AsyncOp<MultiResource> ImportAllAsync(string inputFilePath, ImportOptions importOptions)
		{
			return Internal_importAllAsync(inputFilePath, importOptions);
		}

		/// <summary>Checks if we can import a file with the specified extension.</summary>
		/// <param name="extension">The extension without the leading dot.</param>
		public static bool SupportsFileType(string extension)
		{
			return Internal_supportsFileType(extension);
		}

		/// <summary>
		/// Imports a resource at the specified location, and returns the loaded data. If file contains more than one resource 
		/// only the primary resource is imported (for example an FBX a mesh would be imported, but animations ignored).
		/// </summary>
		/// <param name="inputFilePath">Pathname of the input file.</param>
		/// <param name="importOptions">
		/// (optional) Options for controlling the import. Caller must ensure import options  actually match the type of the 
		/// importer used for the file type.
		/// </param>
		/// <returns>Imported resource.</returns>
		public static Resource Import(string inputFilePath, ImportOptions importOptions)
		{
			UUID UUID = new UUID();
			return Internal_import(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Imports a resource at the specified location, and returns the loaded data. If file contains more than one resource 
		/// only the primary resource is imported (for example an FBX a mesh would be imported, but animations ignored).
		/// </summary>
		/// <param name="inputFilePath">Pathname of the input file.</param>
		/// <returns>Imported resource.</returns>
		public static Resource Import(string inputFilePath)
		{
			ImportOptions importOptions = null;
			UUID UUID = new UUID();
			return Internal_import(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Same as import(), except it imports a resource without blocking the main thread. The resulting resource will be 
		/// placed in the returned AsyncOp object when the import ends.
		/// </summary>
		public static AsyncOp<RRefBase> ImportAsync(string inputFilePath, ImportOptions importOptions)
		{
			UUID UUID = new UUID();
			return Internal_importAsync(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Same as import(), except it imports a resource without blocking the main thread. The resulting resource will be 
		/// placed in the returned AsyncOp object when the import ends.
		/// </summary>
		public static AsyncOp<RRefBase> ImportAsync(string inputFilePath)
		{
			ImportOptions importOptions = null;
			UUID UUID = new UUID();
			return Internal_importAsync(inputFilePath, importOptions, ref UUID);
		}

		/// <summary>
		/// Imports a resource at the specified location, and returns the loaded data. This method returns all imported 
		/// resources, which is relevant for files that can contain multiple resources (for example an FBX which may contain both 
		/// a mesh and animations).
		/// </summary>
		/// <param name="inputFilePath">Pathname of the input file.</param>
		/// <returns>A list of all imported resources. The primary resource is always the first returned resource.</returns>
		public static MultiResource ImportAll(string inputFilePath)
		{
			ImportOptions importOptions = null;
			return Internal_importAll(inputFilePath, importOptions);
		}

		/// <summary>
		/// Same as importAll(), except it imports a resource without blocking the main thread. The returned AsyncOp will contain 
		/// a list of the imported resources, after the import ends.
		/// </summary>
		public static AsyncOp<MultiResource> ImportAllAsync(string inputFilePath)
		{
			ImportOptions importOptions = null;
			return Internal_importAllAsync(inputFilePath, importOptions);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Resource Internal_import(string inputFilePath, ImportOptions importOptions, ref UUID UUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AsyncOp<RRefBase> Internal_importAsync(string inputFilePath, ImportOptions importOptions, ref UUID UUID);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MultiResource Internal_importAll(string inputFilePath, ImportOptions importOptions);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AsyncOp<MultiResource> Internal_importAllAsync(string inputFilePath, ImportOptions importOptions);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_supportsFileType(string extension);
	}

	/** @} */
#endif
}
