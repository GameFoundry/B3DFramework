//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// Holds a set of styles that control how are GUI element types positioned and displayed in the GUI. Each element type 
    /// can be assigned a specific style.
    /// </summary>
    public sealed class GUISkin : Resource
    {
        // Constructor for runtime use only (dummy parameter to differentiate from the normal constructor)
        private GUISkin(bool dummy)
        {  }

        /// <summary>
        /// Creates a new empty GUI skin.
        /// </summary>
        public GUISkin()
        {
            Internal_CreateInstance(this);
        }

        /// <summary>
        /// Returns names of all GUI element styles registered on this skin.
        /// </summary>
        public string[] StyleNames
        {
            get { return Internal_GetStyleNames(mCachedPtr); }
        }

        /// <summary>
        /// Checks if the style with the specified name exists.
        /// </summary>
        /// <param name="name">Name of the style to look for.</param>
        /// <returns>True if the style exists in this skin, false otherwise.</returns>
        public bool HasStyle(string name)
        {
            return Internal_HasStyle(mCachedPtr, name);
        }

        /// <summary>
        /// Returns a style for the specified GUI element type.
        /// </summary>
        /// <param name="name">Name of the style to look for.</param>
        /// <returns>Found element style, or null if a style with the specified name was not found.</returns>
        public GUIElementStyle GetStyle(string name)
        {
            return Internal_GetStyle(mCachedPtr, name);
        }

        /// <summary>
        /// Sets a style for the specified GUI element type.
        /// </summary>
        /// <param name="name">Name of the style to add/modify.</param>
        /// <param name="style">Style object containing style options.</param>
        public void SetStyle(string name, GUIElementStyle style)
        {
            IntPtr stylePtr = IntPtr.Zero;
            if (style != null)
                stylePtr = style.GetCachedPtr();

            Internal_SetStyle(mCachedPtr, name, stylePtr);
        }

        /// <summary>
        /// Removes a style from the skin.
        /// </summary>
        /// <param name="name">Name of the style to remove.</param>
        public void RemoveStyle(string name)
        {
            Internal_RemoveStyle(mCachedPtr, name);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUISkin instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_HasStyle(IntPtr thisPtr, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GUIElementStyle Internal_GetStyle(IntPtr thisPtr, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStyle(IntPtr thisPtr, string name, IntPtr style);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_RemoveStyle(IntPtr thisPtr, string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string[] Internal_GetStyleNames(IntPtr thisPtr);
    }

    /** @} */
}
