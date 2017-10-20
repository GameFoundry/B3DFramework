//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Script
     *  @{
     */

    /// <summary>
    /// Contains all supported external code editors
    /// </summary>
    public enum CodeEditorType // Note: Must match C++ CodeEditorType enum
    {
        VS2008,
        VS2010,
        VS2012,
        VS2013,
        VS2015,
        VS2017,
        MonoDevelop,
        None
    }

    /// <summary>
    /// Handles interaction with the external application used for editing scripts.
    /// </summary>
    public static class CodeEditor
    {
        /// <summary>
        /// Contains resource types that are relevant to the code editor.
        /// </summary>
        public static readonly ResourceType[] CodeTypes =
        {
            ResourceType.ScriptCode,
            ResourceType.PlainText,
            ResourceType.Shader,
            ResourceType.ShaderInclude
        };

        private static bool isSolutionDirty;

        /// <summary>
        /// Currently active code editor.
        /// </summary>
        public static CodeEditorType ActiveEditor
        {
            set { Internal_SetActiveEditor(value); }
            get { return Internal_GetActiveEditor(); }
        }

        /// <summary>
        /// Checks if the code editor's solution requires updating.
        /// </summary>
        internal static bool IsSolutionDirty
        {
            get { return isSolutionDirty; }
        }

        /// <summary>
        /// Returns a list of all code editors available on this machine.
        /// </summary>
        public static CodeEditorType[] AvailableEditors
        {
            get { return Internal_GetAvailableEditors(); }
        }

        /// <summary>
        /// Opens a script file in the currently active code editor.
        /// </summary>
        /// <param name="path">Path to the script file to open, either absolute or relative to the project resources folder.</param>
        /// <param name="line">Line in the file to focus the editor on.</param>
        public static void OpenFile(string path, int line)
        {
            if (IsSolutionDirty)
                SyncSolution();

            Internal_OpenFile(path, line);
        }

        /// <summary>
        /// Generates a solution file for the active editor, which includes all scripts in the project.
        /// </summary>
        public static void SyncSolution()
        {
            Internal_SyncSolution();
            isSolutionDirty = false;
        }

        /// <summary>
        /// Notifies the code editor that code file structure has changed and the solution needs to be rebuilt.
        /// </summary>
        internal static void MarkSolutionDirty()
        {
            isSolutionDirty = true;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_SetActiveEditor(CodeEditorType type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern CodeEditorType Internal_GetActiveEditor();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern CodeEditorType[] Internal_GetAvailableEditors();

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_OpenFile(string path, int line);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Internal_SyncSolution();
    }

    /** @} */
}
