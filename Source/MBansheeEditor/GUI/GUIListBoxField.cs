//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup GUI-Editor 
     *  @{
     */

    /// <summary>
    /// Editor GUI element that displays a list box with user-specified elements and an optional label. List box can be
    /// a standard list-box that allows a single element to be selected, or a multi-select list box where any number of
    /// elements can be selected at the same time.
    /// </summary>
    public sealed class GUIListBoxField : GUIElement
    {
        public delegate void OnSelectionChangedDelegate(int index);

        /// <summary>
        /// Triggered whenever user selects a new element in the list box. Returned index maps to the element in the
        /// elements array that the list box was initialized with.
        /// </summary>
        public event OnSelectionChangedDelegate OnSelectionChanged;

        /// <summary>
        /// Index of the list box entry currently selected. Returns -1 if nothing is selected.
        /// </summary>
        public int Index
        {
            get { return Internal_GetValue(mCachedPtr); }
            set { Internal_SetValue(mCachedPtr, value); }
        }

        /// <summary>
        /// States of all element in the list box (enabled or disabled).
        /// </summary>
        public bool[] States
        {
            get { return Internal_GetElementStates(mCachedPtr); }
            set { Internal_SetElementStates(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(LocString[] elements, bool multiselect, GUIContent title, int titleWidth, string style = "",
            params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, title, titleWidth, style, options, true);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(LocString[] elements, bool multiselect, GUIContent title, int titleWidth = 100, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, title, titleWidth, "", options, true);
        }

        /// <summary>
        /// Creates a new single-selection list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(LocString[] elements, GUIContent title, int titleWidth, string style = "", 
            params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, false, title, titleWidth, style, options, true);
        }

        /// <summary>
        /// Creates a new single-selection list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(LocString[] elements, GUIContent title, int titleWidth = 100, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, false, title, titleWidth, "", options, true);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and no label.
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
        public GUIListBoxField(LocString[] elements, bool multiselect = false, string style = "", params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, null, 0, style, options, false);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and no label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(LocString[] elements, bool multiselect = false, params GUIOption[] options)
        {
            Internal_CreateInstance(this, elements, multiselect, null, 0, "", options, false);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(string[] elements, bool multiselect, GUIContent title, int titleWidth, string style = "",
            params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), multiselect, title, titleWidth, style, options, true);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(string[] elements, bool multiselect, GUIContent title, int titleWidth = 100, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), multiselect, title, titleWidth, "", options, true);
        }

        /// <summary>
        /// Creates a new single-selection list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(string[] elements, GUIContent title, int titleWidth, string style = "",
            params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), false, title, titleWidth, style, options, true);
        }

        /// <summary>
        /// Creates a new single-selection list box with the specified elements and a label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="title">Content to display on the label.</param>
        /// <param name="titleWidth">Width of the title label in pixels.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(string[] elements, GUIContent title, int titleWidth = 100, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), false, title, titleWidth, "", options, true);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and no label.
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
        public GUIListBoxField(string[] elements, bool multiselect = false, string style = "", params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), multiselect, null, 0, style, options, false);
        }

        /// <summary>
        /// Creates a new list box with the specified elements and no label.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        /// <param name="multiselect">Determines should the listbox allow multiple elements to be selected or just one.
        ///                           </param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIListBoxField(string[] elements, bool multiselect = false, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ToLocalizedElements(elements), multiselect, null, 0, "", options, false);
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
        /// Updates the list box with a new set of elements.
        /// </summary>
        /// <param name="elements">Array of elements to display in the list box. Elements will be displayed in the same 
        ///                        order as in the array.</param>
        public void SetElements(string[] elements)
        {
            Internal_SetElements(mCachedPtr, ToLocalizedElements(elements));
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
        /// Converts a set of normal strings to a set of localized strings.
        /// </summary>
        /// <param name="elements">Strings to convert.</param>
        /// <returns>Localized strings created from input strings.</returns>
        private LocString[] ToLocalizedElements(string[] elements)
        {
            if (elements == null)
                return null;

            LocString[] locElements = new LocString[elements.Length];
            for (int i = 0; i < locElements.Length; i++)
                locElements[i] = elements[i];

            return locElements;
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
        private static extern void Internal_CreateInstance(GUIListBoxField instance, LocString[] entries, bool multiselect,
            GUIContent title, int titleWidth, string style, GUIOption[] options, bool withTitle);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetElements(IntPtr nativeInstance, LocString[] elements);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int Internal_GetValue(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, int value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool[] Internal_GetElementStates(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetElementStates(IntPtr nativeInstance, bool[] states);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SelectElement(IntPtr nativeInstance, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_DeselectElement(IntPtr nativeInstance, int idx);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
