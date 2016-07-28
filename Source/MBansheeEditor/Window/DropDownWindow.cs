//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Window
     *  @{
     */

    /// <summary>
    /// Base class for all drop down window implementations. Drop down windows are temporary windows that open above all
    /// other GUI, and close as soon user clicks outside of them.
    /// </summary>
    public class DropDownWindow : ScriptObject
    {
        /// <summary>
        /// Width of the window in pixels.
        /// </summary>
        public int Width
        {
            get { return Internal_GetWidth(mCachedPtr); }
            set { Internal_SetWidth(mCachedPtr, value); }
        }

        /// <summary>
        /// Height of the window in pixels.
        /// </summary>
        public int Height
        {
            get { return Internal_GetHeight(mCachedPtr); }
            set { Internal_SetHeight(mCachedPtr, value); }
        }

        protected GUIPanel GUI;

        /// <summary>
        /// Opens a new drop down window at the specified location.
        /// </summary>
        /// <typeparam name="T">Type of the drop down window to open.</typeparam>
        /// <param name="parent">Parent editor window to open the drop down window in.</param>
        /// <param name="position">Position relative to the parent editor window at which to open the drop down window.
        ///                        </param>
        /// <returns>Instance of the opened drop down window.</returns>
        public static T Open<T>(EditorWindow parent, Vector2I position) where T : DropDownWindow, new()
        {
            IntPtr parentPtr = IntPtr.Zero;
            if (parent != null)
                parentPtr = parent.GetCachedPtr();

            T window = (T)Internal_CreateInstance(typeof(T).Namespace, typeof(T).Name, parentPtr, ref position);

            return window;
        }

        /// <summary>
        /// Constructs a new drop down window.
        /// </summary>
        protected DropDownWindow()
        { }

        /// <summary>
        /// Converts coordinates in screen space to coordinates relative to the drop down window.
        /// </summary>
        /// <param name="screenPos">Coordinates in screen space.</param>
        /// <returns>Coordinates relative to the drop down window.</returns>
        protected Vector2I ScreenToWindowPos(Vector2I screenPos)
        {
            Vector2I windowPos;
            Internal_ScreenToWindowPos(mCachedPtr, ref screenPos, out windowPos);
            return windowPos;
        }

        /// <summary>
        /// Converts coordinates relative to the drop down window to screen space to coordinates.
        /// </summary>
        /// <param name="windowPos">Coordinates relative to the drop down window.</param>
        /// <returns>Coordinates in screen space.</returns>
        protected Vector2I WindowToScreenPos(Vector2I windowPos)
        {
            Vector2I screenPos;
            Internal_WindowToScreenPos(mCachedPtr, ref windowPos, out screenPos);
            return screenPos;
        }

        /// <summary>
        /// Closes the drop down window.
        /// </summary>
        protected void Close()
        {
            Internal_Close(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern DropDownWindow Internal_CreateInstance(string ns, string typeName, IntPtr parentWindow, ref Vector2I position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Close(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetWidth(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetWidth(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetHeight(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHeight(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ScreenToWindowPos(IntPtr nativeInstance, ref Vector2I position, out Vector2I windowPos);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_WindowToScreenPos(IntPtr nativeInstance, ref Vector2I position, out Vector2I screenPos);
    }

    /** @} */
}
