//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Utility-Editor
     *  @{
     */

    /// <summary>
    /// Types of icons that may be displayed on the tool bar.
    /// </summary>
    public enum ToolbarIcon // Note: Must match C++ enum ToolbarIcon
	{
        NewCamera, NewRenderable, NewPointLight, NewDirLight, NewSpotLight, NewSceneObject, NewCube, NewSphere, NewCone, 
        NewQuad, NewMat, NewCSScript, NewShader, NewSpriteTex, Pause, Play, Step, Undo, Redo, OpenProject, SaveProject, 
        SaveScene
	}

    /// <summary>
    /// Types of icons that may be displayed in the scene window.
    /// </summary>
    public enum SceneWindowIcon // Note: Must match C++ enum SceneWindowIcon
	{
		View, Move, Rotate, Scale, Pivot, Center, Local, World, MoveSnap, RotateSnap
	}

    /// <summary>
    /// Types of icons that may be displayed in the library window.
    /// </summary>
    public enum LibraryWindowIcon // Note: Must match C++ enum LibraryWindowIcon
	{
		Home, Up, Clear, Options
	}

    /// <summary>
    /// Types of icons that may be displayed in the inspector window.
    /// </summary>
	public enum InspectorWindowIcon  // Note: Must match C++ enum InspectorWindowIcon
	{
		Create, Clone, Clear, Resize, Delete, MoveUp, MoveDown, Edit, Apply, Add, Cancel
	}

    /// <summary>
    /// Types of icons that may be displayed for resources in the library window.
    /// </summary>
    public enum LibraryItemIcon // Note: Must match C++ enum ProjectIcon
	{
		Folder, Mesh, Font, Texture, PlainText, ScriptCode, SpriteTexture, Shader, ShaderInclude, Material, Prefab, GUISkin,
        PhysicsMaterial, PhysicsMesh, AudioClip, AnimationClip
    }

    /// <summary>
    /// Types of icons that may be displayed in the animation editor window.
    /// </summary>
    public enum AnimationWindowIcon // Note: Must match C++ enum AnimationWindowIcon
    {
        Play, Record, FrameForward, FrameBack, AddKeyframe, AddEvent, Keyframe, Event
    };

    /// <summary>
    /// Types of icons that can be used for displaying types of log messages.
    /// </summary>
    public enum LogIcon // Note: Must match C++ enum LogMessageIcon
    {
        Info, Warning, Error
    }

    /// <summary>
    /// Types of icons used in various areas throughout the editor. 
    /// </summary>
    public enum EditorIcon // Note: Must match C++ enum EditorIcon
    {
        X, Component, SceneObject
    }

    /// <summary>
    /// Contains various editor-specific resources that are always available.
    /// </summary>
    public static class EditorBuiltin
    {
        /// <summary>Returns text contained in the default "empty" shader.</summary>
        public static string EmptyShaderCode { get { return Internal_GetEmptyShaderCode(); } }

        /// <summary>Returns text contained in the default "empty" C# script.</summary>
        public static string EmptyCSScriptCode { get { return Internal_GetEmptyCSScriptCode(); } }

        /// <summary>
        /// Returns the default Font used in the editor.
        /// </summary>
        public static Font DefaultFont
        {
            get { return Internal_GetDefaultFont(); }
        }

        /// <summary>
        /// Returns the default GUI skin used in the editor.
        /// </summary>
        public static GUISkin GUISkin
        {
            get { return Internal_GetGUISkin(); }
        }

        /// <summary>
        /// Retrieves an icon used for displaying an entry in the library window.
        /// </summary>
        /// <param name="icon">Type of the icon to retrieve.</param>
        /// <param name="size">Size of the icon to retrieve in pixels. This will be rounded
        ///                    to nearest available size.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetLibraryItemIcon(LibraryItemIcon icon, int size)
        {
            return Internal_GetLibraryItemIcon(icon, size);
        }

        /// <summary>
        /// Retrieves an icon that may be displayed on the main window's toolbar.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetToolbarIcon(ToolbarIcon icon)
        {
            return Internal_GetToolbarIcon(icon);
        }

        /// <summary>
        /// Retrieves an icon that may be displayed on the library window.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetLibraryWindowIcon(LibraryWindowIcon icon)
        {
            return Internal_GetLibraryWindowIcon(icon);
        }

        /// <summary>
        /// Retrieves an icon that may be displayed on the inspector window.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetInspectorWindowIcon(InspectorWindowIcon icon)
        {
            return Internal_GetInspectorWindowIcon(icon);
        }

        /// <summary>
        /// Retrieves an icon that may be displayed on the scene window.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite textures of the icon.</returns>
        public static GUIContentImages GetSceneWindowIcon(SceneWindowIcon icon)
        {
            return Internal_GetSceneWindowIcon(icon);
        }

        /// <summary>
        /// Retrieves an icon that may be displayed on the animation window.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite textures of the icon.</returns>
        public static GUIContentImages GetAnimationWindowIcon(AnimationWindowIcon icon)
        {
            return Internal_GetAnimationWindowIcon(icon);
        }

        /// <summary>
        /// Retrieves an icon that represents different types of log entries.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <param name="size">Size of the icon in pixels. Nearest available size will be returned.</param>
        /// <param name="dark">Controls should the dark or light version of the icon be returned.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetLogIcon(LogIcon icon, int size, bool dark)
        {
            return Internal_GetLogIcon(icon, size, dark);
        }

        /// <summary>
        /// Retrieves an icon used throughout the editor.
        /// </summary>
        /// <param name="icon">Type of icon to retrieve.</param>
        /// <returns>Sprite texture of the icon.</returns>
        public static SpriteTexture GetEditorIcon(EditorIcon icon)
        {
            return Internal_GetEditorIcon(icon);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetLibraryItemIcon(LibraryItemIcon icon, int size);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetEmptyShaderCode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetEmptyCSScriptCode();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetToolbarIcon(ToolbarIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetLibraryWindowIcon(LibraryWindowIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetInspectorWindowIcon(InspectorWindowIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GUIContentImages Internal_GetSceneWindowIcon(SceneWindowIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GUIContentImages Internal_GetAnimationWindowIcon(AnimationWindowIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetLogIcon(LogIcon icon, int size, bool dark);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SpriteTexture Internal_GetEditorIcon(EditorIcon icon);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern Font Internal_GetDefaultFont();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GUISkin Internal_GetGUISkin();
    }

    /** @} */
}
