//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Window
     *  @{
     */

    /// <summary>
    /// Base class for all editor windows. Editor window can be docked on the main editor window or float as a separate 
    /// window. User is allowed to reposition and resize the window as he wishes. Editor window GUI is fully customizable.
    /// </summary>
    public class EditorWindow : ScriptObject
    {
#pragma warning disable 649 // Value assigned by the runtime
        private UndoRedo undoRedo;
#pragma warning restore 649

        /// <summary>
        /// Width of the window in pixels.
        /// </summary>
        public int Width
        {
            get { return Internal_GetWidth(mCachedPtr); }
        }

        /// <summary>
        /// Height of the window in pixels.
        /// </summary>
        public int Height
        {
            get { return Internal_GetHeight(mCachedPtr); }
        }

        /// <summary>
        /// Screen space bounds of the window.
        /// </summary>
        public Rect2I Bounds
        {
            get
            {
                Rect2I bounds;
                Internal_GetBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Determines whether the editor window currently has keyboard focus (has been clicked on most recently). Window
        /// that isn't active (is not the active tab) cannot be put into focus without activating it first.
        /// </summary>
        public bool HasFocus
        {
            get { return Internal_HasFocus(mCachedPtr); }
            set { Internal_SetFocus(mCachedPtr, value); }
        }

        /// <summary>
        /// Returns the local undo/redo stack specific to this editor window. Only windows marked with [UndoRedoLocal]
        /// attribute will return a non-null value.
        /// </summary>
        public UndoRedo UndoRedo
        {
            get { return undoRedo; }
        }

        /// <summary>
        /// Determines is the mouse pointer currently hovering over the editor window.
        /// </summary>
        public bool IsPointerHovering { get { return Internal_IsPointerHovering(mCachedPtr); } }

        /// <summary>
        /// Checks if the window's tab is currently active. If the window is floating or not sharing space with any other
        /// windows (just a single tab), it is always considered active.
        /// </summary>
        public bool Active
        {
            get { return Internal_IsActive(mCachedPtr); }
        }

        /// <summary>
        /// GUI panel that you may use for adding GUI elements to the window.
        /// </summary>
        public GUIPanel GUI;

        /// <summary>
        /// Returns a list of all currently open editor windows.
        /// </summary>
        public static EditorWindow[] AllWindows
        {
            get { return Internal_GetAllWindows(); }
        }

        /// <summary>
        /// Opens an editor window. If window is already open it returns the existing instance.
        /// </summary>
        /// <typeparam name="T">Type of window to open.</typeparam>
        /// <returns>Instance of the open window.</returns>
        public static T OpenWindow<T>() where T : EditorWindow
        {
            return (T)Internal_CreateOrGetInstance(typeof(T).Namespace, typeof(T).Name);
        }

        /// <summary>
        /// Retrieves instance of an open window.
        /// </summary>
        /// <typeparam name="T">Type of window to retrieve the instance of.</typeparam>
        /// <returns>Instance of the winodow if it is open, null otherwise.</returns>
        public static T GetWindow<T>() where T : EditorWindow
        {
            return (T)Internal_GetInstance(typeof(T).Namespace, typeof(T).Name);
        }

        /// <summary>
        /// Converts coordinates in screen space to coordinates relative to the window.
        /// </summary>
        /// <param name="screenPos">Coordinates in screen space.</param>
        /// <returns>Coordinates relative to the window.</returns>
        public Vector2I ScreenToWindowPos(Vector2I screenPos)
        {
            Vector2I windowPos;
            Internal_ScreenToWindowPos(mCachedPtr, ref screenPos, out windowPos);
            return windowPos;
        }

        /// <summary>
        /// Converts coordinates relative to the window to screen space to coordinates.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to the window.</param>
        /// <returns>Coordinates in screen space.</returns>
        public Vector2I WindowToScreenPos(Vector2I windowPos)
        {
            Vector2I screenPos;
            Internal_WindowToScreenPos(mCachedPtr, ref windowPos, out screenPos);
            return screenPos;
        }

        /// <summary>
        /// Triggered whenever the window size ranges.
        /// </summary>
        /// <param name="width">New width of the window in pixels.</param>
        /// <param name="height">New height of the window in pixels.</param>
        protected virtual void WindowResized(int width, int height)
        {

        }

        /// <summary>
        /// Triggered whenever the window gains or loses focus.
        /// </summary>
        /// <param name="inFocus">True if focus was gained, false otherwise.</param>
        protected virtual void FocusChanged(bool inFocus)
        {
            
        }

        /// <summary>
        /// Name of the window to display in the title bar.
        /// </summary>
        /// <returns>Name of the window to display in the title bar.</returns>
        protected virtual LocString GetDisplayName()
        {
            return GetType().Name;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EditorWindow Internal_CreateOrGetInstance(string ns, string typeName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EditorWindow Internal_GetInstance(string ns, string typeName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern EditorWindow[] Internal_GetAllWindows();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetWidth(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetHeight(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_HasFocus(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFocus(IntPtr nativeInstance, bool focus);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsActive(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_IsPointerHovering(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetBounds(IntPtr nativeInstance, out Rect2I bounds);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ScreenToWindowPos(IntPtr nativeInstance, ref Vector2I position, out Vector2I windowPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_WindowToScreenPos(IntPtr nativeInstance, ref Vector2I position, out Vector2I screenPos);
    }

    /** @} */
}
