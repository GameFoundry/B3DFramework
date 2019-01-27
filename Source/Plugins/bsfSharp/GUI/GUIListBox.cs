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
    /// List box GUI element which opens a drop down selection with provided elements when active.
    /// </summary>
    public sealed class GUIListBox : GUIElement
    {
        public delegate void OnSelectionChangedDelegate(int index);

        /// <summary>
        /// Triggered whenever user selects a new element in the list box. Returned index maps to the element in the 
        /// elements array that the list box was initialized with.
        /// </summary>
        public event OnSelectionChangedDelegate OnSelectionChanged;

        /// <summary>
        /// States of all element in the list box (enabled or disabled).
        /// </summary>
        public bool[] States
        {
            get { return Internal_GetElementStates(mCachedPtr); }
            set { Internal_SetElementStates(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new list box with the specified elements.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBox(LocString[] elements, bool multiselect = false, string style = "", params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, style, options);
        }

        /// <summary>
        /// Creates a new list box with the specified elements.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBox(LocString[] elements, bool multiselect = false, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, "", options);
        }

        /// <summary>
        /// Updates the list box with a new set of elements.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        public void SetElements(LocString[] elements)
        {
            Internal_SetElements(mCachedPtr, elements);
        }

        /// <summary>
        /// Makes the element with the specified index selected.
        /// </summary>
        /// <param name="idx">Sequential index in the elements array provided on construction.</param>
        public void SelectElement(int idx)
        {
            Internal_SelectElement(mCachedPtr, idx);
        }

        /// <summary>
        /// Deselect element the element with the specified index. Only relevant for multi-select list boxes.
        /// </summary>
        /// <param name="idx">Sequential index in the elements array provided on construction.</param>
        public void DeselectElement(int idx)
        {
            Internal_DeselectElement(mCachedPtr, idx);
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        /// <summary>
        /// Triggered by the native interop object when a user selects an object in the list.
        /// </summary>
        private void DoOnSelectionChanged(int index)
        {
            if (OnSelectionChanged != null)
                OnSelectionChanged(index);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIListBox instance, LocString[] elements, bool multiselect,
            string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetElements(IntPtr nativeInstance, LocString[] elements);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SelectElement(IntPtr nativeInstance, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DeselectElement(IntPtr nativeInstance, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool[] Internal_GetElementStates(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetElementStates(IntPtr nativeInstance, bool[] states);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
