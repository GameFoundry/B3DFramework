//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System.IO;

namespace BansheeEngine
{
    /** @addtogroup Utility
     *  @{
     */

    /// <summary>
    /// Contains various methods that provide handling for files not provided by System.File type.
    /// </summary>
    public static class FileEx
    {
        /// <summary>
        /// Moves a file from one path to another, while creating any parent directories if they don't already exist.
        /// </summary>
        /// <param name="source">Path to the file to move.</param>
        /// <param name="destination">New location and/or name of the file.</param>
        public static void Move(string source, string destination)
        {
            string destParent = PathEx.GetParent(destination);
            if (!string.IsNullOrEmpty(destParent))
            {
                if (!Directory.Exists(destParent))
                    Directory.CreateDirectory(destParent);
            }

            File.Move(source, destination);
        }

        /// <summary>
        /// Copies a file from one path to another, while creating any parent directories if they don't already exist.
        /// </summary>
        /// <param name="source">Path to the file to copy.</param>
        /// <param name="destination">Path to the copied file.</param>
        public static void Copy(string source, string destination)
        {
            string destParent = PathEx.GetParent(destination);
            if (!string.IsNullOrEmpty(destParent))
            {
                if (!Directory.Exists(destParent))
                    Directory.CreateDirectory(destParent);
            }

            File.Copy(source, destination);
        }
    }

    /** @} */
}
