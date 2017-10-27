//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.IO;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// Contains various helper methods for dealing with the project library.
    /// </summary>
    public static class LibraryUtility
    {
        /// <summary>
        /// Creates a new folder with in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the new folder in.</param>
        public static void CreateFolder(string folder)
        {
            string path = Path.Combine(folder, "New Folder");
            path = GetUniquePath(path);

            ProjectLibrary.CreateFolder(path);
        }

        /// <summary>
        /// Creates a new material with the default shader in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the material in.</param>
        public static void CreateEmptyMaterial(string folder)
        {
            string path = Path.Combine(folder, "New Material.asset");
            path = GetUniquePath(path);

            Material material = new Material(Builtin.GetShader(BuiltinShader.Standard));
            ProjectLibrary.Create(material, path);
        }

        /// <summary>
        /// Creates a new empty sprite texture in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the sprite texture in.</param>
        public static void CreateEmptySpriteTexture(string folder)
        {
            string path = Path.Combine(folder, "New Sprite Texture.asset");
            path = GetUniquePath(path);

            SpriteTexture spriteTexture = new SpriteTexture(null);
            ProjectLibrary.Create(spriteTexture, path);
        }

        /// <summary>
        /// Creates a new empty string table in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the string table in.</param>
        public static void CreateEmptyStringTable(string folder)
        {
            string path = Path.Combine(folder, "New String Table.asset");
            path = GetUniquePath(path);

            StringTable stringTable = new StringTable();
            ProjectLibrary.Create(stringTable, path);
        }

        /// <summary>
        /// Creates a new empty GUI skin in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the GUI skin in.</param>
        public static void CreateEmptyGUISkin(string folder)
        {
            string path = Path.Combine(folder, "New GUI Skin.asset");
            path = GetUniquePath(path);

            GUISkin guiSkin = new GUISkin();
            ProjectLibrary.Create(guiSkin, path);
        }

        /// <summary>
        /// Creates a new shader containing a rough code outline in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the shader in.</param>
        public static void CreateEmptyShader(string folder)
        {
            string path = Path.Combine(folder, "New Shader.bsl");
            path = Path.Combine(ProjectLibrary.ResourceFolder, path);
            path = GetUniquePath(path);

            File.WriteAllText(path, EditorBuiltin.EmptyShaderCode);
            ProjectLibrary.Refresh(path);
        }

        /// <summary>
        /// Creates a new C# script containing a rough code outline in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the C# script in.</param>
        public static void CreateEmptyCSScript(string folder)
        {
            string path = Path.Combine(folder, "NewScript.cs");
            path = Path.Combine(ProjectLibrary.ResourceFolder, path);
            path = GetUniquePath(path);

            File.WriteAllText(path, EditorBuiltin.EmptyCSScriptCode);
            ProjectLibrary.Refresh(path);
        }

        /// <summary>
        /// Creates a new physics material with the default properties in the specified folder.
        /// </summary>
        /// <param name="folder">Folder relative to project library to create the material in.</param>
        public static void CreateEmptyPhysicsMaterial(string folder)
        {
            string path = Path.Combine(folder, "New Physics Material.asset");
            path = GetUniquePath(path);

            PhysicsMaterial material = new PhysicsMaterial();
            ProjectLibrary.Create(material, path);
        }

        /// <summary>
        /// Checks if a file or folder at the specified path exists in the library, and if it does generates a new unique 
        /// name for the file or folder.
        /// </summary>
        /// <param name="path">Path to the file or folder to generate a unique name.</param>
        /// <returns>New path with the unique name. This will be unchanged from input path if the input path doesn't
        ///          already exist.</returns>
        public static string GetUniquePath(string path)
        {
            string extension = Path.GetExtension(path);
            string pathClean = path;
            if (!String.IsNullOrEmpty(extension))
                pathClean = path.Remove(path.Length - extension.Length);

            int idx = 0;
            int separatorIdx = pathClean.LastIndexOf('_');
            if (separatorIdx != -1)
            {
                string numberString = pathClean.Substring(separatorIdx + 1, pathClean.Length - (separatorIdx + 1));
                if (int.TryParse(numberString, out idx))
                {
                    pathClean = pathClean.Substring(0, separatorIdx);
                    idx++;
                }
            }
           
            string destination = path;
            while (ProjectLibrary.Exists(destination))
            {
                destination = pathClean + "_" + idx + extension;
                idx++;
            }

            return destination;
        }
    }

    /** @} */
}
