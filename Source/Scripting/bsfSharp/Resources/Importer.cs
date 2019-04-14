using System;
using System.Collections.Generic;

#if !IS_B3D
namespace bs
{
    partial class Importer
    {
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
        public static T Import<T>(string inputFilePath, ImportOptions importOptions, UUID UUID) where T: Resource
        {
            return Internal_import(inputFilePath, importOptions, ref UUID) as T;
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
        public static T Import<T>(string inputFilePath, ImportOptions importOptions) where T: Resource
        {
            UUID UUID = new UUID();
            return Internal_import(inputFilePath, importOptions, ref UUID) as T;
        }

        /// <summary>
        /// Imports a resource at the specified location, and returns the loaded data. If file contains more than one resource 
        /// only the primary resource is imported (for example an FBX a mesh would be imported, but animations ignored).
        /// </summary>
        /// <param name="inputFilePath">Pathname of the input file.</param>
        /// <returns>Imported resource.</returns>
        public static T Import<T>(string inputFilePath) where T: Resource
        {
            ImportOptions importOptions = null;
            UUID UUID = new UUID();
            return Internal_import(inputFilePath, importOptions, ref UUID) as T;
        }
    }
}
#endif
