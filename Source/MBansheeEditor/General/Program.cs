//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup BansheeEditor
     *  Scripting API available in editor only, mainly used for extending the editor in various ways.
     *  @{
     */

    /** @defgroup General General 
     *  Various functionality that doesn't fit into any other category.
     */

    /** @defgroup GUI-Editor GUI 
     *  Editor specific GUI elements.
     */

    /** @defgroup Input-Editor Input 
     *  %Input handling for editor only (not affected by game pauses).
     */

    /** @defgroup Inspectors Inspectors 
     *  Implementations of inspectors for various engine types.
     */

    /** @defgroup Settings Settings 
     *  Editor and project settings, and related window.
     */

    /** @defgroup Script Script 
     *  Script code management and compilation.
     */

    /** @defgroup Tests Tests 
     *  Editor only unit tests
     */

    /** @defgroup Utility-Editor Utility 
     *  Various utility functionality.
     */

    /** @defgroup Window Window 
     *  Interface and tools for creating custom editor windows.
     */

    /** @defgroup Windows Windows 
     *  Implementations of various editor windows (for example Scene, Game, Library).
     *  @{
     */

    /** @defgroup AnimationEditor Animation Editor 
     *  Animation editor window and related functionality.
     */

    /** @defgroup Build Build 
     *  Build editor window and build manager.
     */

    /** @defgroup Inspector Inspector 
     *  Interface and utilities needed for implementing custom inspectors.
     */

    /** @defgroup Library Library 
     *  Managment of resources in the project (loading, saving, creating, querying, etc.).
     */

    /** @defgroup Scene-Editor Scene 
     *  Scene editor window and related functionality.
     *  @{
     */

    /** @defgroup Gizmos Gizmos 
     *  Interface for creating custom gizmos.
     */

    /** @defgroup Handles Handles 
     *  Interface for creating custom handles (2D/3D tools).
     */

    /** @} */
    /** @} */
    /** @} */

    /** @cond Interop */

    /// <summary>
    /// Entry point to the editor.
    /// </summary>
    class Program
    {
        private static EditorApplication app;

        /// <summary>
        /// Called by the runtime whenever the editor assembly is loaded. This means initially when editor is started
        /// and every time assembly refresh occurs.
        /// </summary>
        static void OnInitialize()
        {
            app = new EditorApplication();
        }

        /// <summary>
        /// Called by the runtime when the editor is first started. Called after <see cref="OnInitialize"/>.
        /// </summary>
        static void OnEditorStartUp()
        {
            if (EditorSettings.AutoLoadLastProject)
            {
                string projectPath = EditorSettings.LastOpenProject;
                if (EditorApplication.IsValidProject(projectPath))
                    EditorApplication.LoadProject(projectPath);
                else
                    ProjectWindow.Open();
            }
            else
                ProjectWindow.Open();

            CodeEditorType activeCodeEditor = (CodeEditorType)EditorSettings.GetInt(SettingsWindow.ActiveCodeEditorKey, (int) CodeEditorType.None);
            CodeEditorType[] availableEditors = CodeEditor.AvailableEditors;
            if (Array.Exists(availableEditors, x => x == activeCodeEditor))
                CodeEditor.ActiveEditor = activeCodeEditor;
            else
            {
                if (availableEditors.Length > 0)
                    CodeEditor.ActiveEditor = availableEditors[0];
            }
        }

        /// <summary>
        /// Called 60 times per second by the runtime.
        /// </summary>
        static void OnEditorUpdate()
        {
            app.OnEditorUpdate();
        }

        /// <summary>
        /// Attempts to save the current scene, and keeps retrying if failed or until user cancels.
        /// </summary>
        static void TrySaveSceneOnQuit()
        {
            Action success = () =>
            {
                EditorApplication.SaveProject();
                EditorApplication.Quit();
            };

            EditorApplication.SaveScene(success, TrySaveSceneOnQuit);
        }

        /// <summary>
        /// Called when the user requests that the editor shuts down. You must manually close the editor from this
        /// method if you choose to accept the users request.
        /// </summary>
        static void OnEditorQuitRequested()
        {
            Action<DialogBox.ResultType> dialogCallback =
            (result) =>
            {
                if (result == DialogBox.ResultType.Yes)
                    TrySaveSceneOnQuit();
                else if (result == DialogBox.ResultType.No)
                {
                    EditorApplication.SaveProject();
                    EditorApplication.Quit();
                }
            };

            if (EditorApplication.IsSceneModified())
            {
                DialogBox.Open("Warning", "You current scene has modifications. Do you wish to save them first?",
                    DialogBox.Type.YesNoCancel, dialogCallback);
            }
            else
            {
                EditorApplication.SaveProject();
                EditorApplication.Quit();
            }
        }
    }

    /** @endcond */
}
