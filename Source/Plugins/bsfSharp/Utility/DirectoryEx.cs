//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.IO;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Contains various methods that provide handling for directories not provided by System.Directory type.
    /// </summary>
    public static class DirectoryEx
    {
        /// <summary>
        /// Moves a directory from one path to another, while creating any parent directories if they don't already exist.
        /// </summary>
        /// <param name="source">Path to the directory to move.</param>
        /// <param name="destination">New location and/or name of the directory.</param>
        public static void Move(string source, string destination)
        {
            string destParent = PathEx.GetParent(destination);
            if (!string.IsNullOrEmpty(destParent))
            {
                if (!Directory.Exists(destParent))
                    Directory.CreateDirectory(destParent);
            }

            Directory.Move(source, destination);
        }

        /// <summary>
        /// Recursively copies a directory from one path to another, while creating any parent directories if they don't 
        /// already exist.
        /// </summary>
        /// <param name="source">Path to the directory to copy.</param>
        /// <param name="destination">Path determining where the directory copy will be placed.</param>
        public static void Copy(string source, string destination)
        {
            DirectoryInfo dir = new DirectoryInfo(source);
            DirectoryInfo[] dirs = dir.GetDirectories();

            if (!dir.Exists)
            {
                throw new DirectoryNotFoundException(
                    "Source directory does not exist or could not be found: "
                    + source);
            }

            if (!Directory.Exists(destination))
                Directory.CreateDirectory(destination);

            FileInfo[] files = dir.GetFiles();
            foreach (FileInfo file in files)
            {
                string temppath = Path.Combine(destination, file.Name);
                file.CopyTo(temppath, false);
            }

            foreach (DirectoryInfo subdir in dirs)
            {
                string temppath = Path.Combine(destination, subdir.Name);
                Copy(subdir.FullName, temppath);
            }
        }
    }

    /** @} */
}
