//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// Base class for GUI layout implementations. GUI layouts serve as containers
    /// for GUI elements that position and resize the elements automatically with
    /// respect to layout rules set on the elements.
    /// </summary>
    public abstract class GUILayout : GUIElement
    {
        /// <summary>
        /// Returns number of child elements in the layout.
        /// </summary>
        public int ChildCount
        {
            get { return Internal_GetChildCount(mCachedPtr); }
        }

        /// <summary>
        /// Adds a new element to the layout after all existing elements.
        /// </summary>
        /// <param name="element">GUI element to add.</param>
        public void AddElement(GUIElement element)
        {
            if(element != null)
                Internal_AddElement(mCachedPtr, element.mCachedPtr);
        }

        /// <summary>
        /// Inserts a GUI element before the element at the specified index.
        /// </summary>
        /// <param name="index">Index to insert the GUI element at. This must be in range [0, GetNumChildren()).</param>
        /// <param name="element">GUI element to insert.</param>
        public void InsertElement(int index, GUIElement element)
        {
            if(index < 0 || index > ChildCount)
                throw new ArgumentOutOfRangeException("index", index, "Index out of range.");

            if (element != null)
                Internal_InsertElement(mCachedPtr, index, element.mCachedPtr);
        }

        /// <summary>
        /// Gets a child elements at the specified index in the layout.
        /// </summary>
        /// <param name="index">Index of the element to retrieve. This must be in range [0, GetNumChildren()).</param>
        /// <returns>GUI element at the specified index, or null if the index is invalid.</returns>
        public GUIElement GetChild(int index)
        {
            return Internal_GetChild(mCachedPtr, index);
        }

        /// <summary>
        /// Adds a new horizontal layout as a child of this layout. Layout is inserted after all existing elements.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the layout positioned and sized.</param>
        /// <returns>Newly created horizontal layout.</returns>
        public GUILayoutX AddLayoutX(params GUIOption[] options)
        {
            GUILayoutX layout = new GUILayoutX(options);
            AddElement(layout);
            return layout;
        }

        /// <summary>
        /// Adds a new vertical layout as a child of this layout. Layout is inserted after all existing elements.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the layout positioned and sized.</param>
        /// <returns>Newly created vertical layout.</returns>
        public GUILayoutY AddLayoutY(params GUIOption[] options)
        {
            GUILayoutY layout = new GUILayoutY(options);
            AddElement(layout);
            return layout;
        }

        /// <summary>
        /// Adds a new GUI panel as a child of this layout. Panel is inserted after all existing elements.
        /// </summary>
        /// <param name="options">Options that allow you to control how is the panel positioned and sized.</param>
        /// <returns>Newly created GUI panel.</returns>
        public GUIPanel AddPanel(params GUIOption[] options)
        {
            GUIPanel layout = new GUIPanel(options);
            AddElement(layout);
            return layout;
        }

        /// <summary>
        /// Adds a new GUI panel as a child of this layout. Panel is inserted after all existing elements.
        /// </summary>
        /// <param name="depth">Depth at which to position the panel. Panels with lower depth will be displayed in front of
        ///                     panels with higher depth. Provided depth is relative to the depth of the parent GUI panel.
        ///                     The depth value will be clamped if outside of the depth range of the parent GUI panel.</param>
        /// <param name="depthRangeMin">Smallest depth offset allowed by any child GUI panels. If a child panel has a depth
        ///                             offset lower than this value it will be clamped.</param>
        /// <param name="depthRangeMax">Largest depth offset allowed by any child GUI panels. If a child panel has a depth
        ///                             offset higher than this value it will be clamped.</param>
        /// <param name="options">Options that allow you to control how is the panel positioned and sized.</param>
        /// <returns>Newly created GUI panel.</returns>
        public GUIPanel AddPanel(Int16 depth = 0, ushort depthRangeMin = ushort.MaxValue,
            ushort depthRangeMax = ushort.MaxValue, params GUIOption[] options)
        {
            GUIPanel layout = new GUIPanel(depth, depthRangeMin, depthRangeMax, options);
            AddElement(layout);
            return layout;
        }

        /// <summary>
        /// Adds a new flexible space as a child of this layout. Flexible space expands
        /// to fill all available space in the layout. Space is inserted after all existing elements.
        /// </summary>
        /// <returns>Newly created flexible space.</returns>
        public GUIFlexibleSpace AddFlexibleSpace()
        {
            GUIFlexibleSpace space = new GUIFlexibleSpace();
            AddElement(space);
            return space;
        }

        /// <summary>
        /// Adds a new fixed space object. Fixed space inserts a blank space with specific
        /// width or height (depending on layout type) in the layout. Space is inserted after all existing elements.
        /// </summary>
        /// <param name="size">Size of the space in pixels. This will represent either width or height depending whether the
        ///                    layout is vertical or horizontal.</param>
        /// <returns>Newly created fixed space.</returns>
        public GUIFixedSpace AddSpace(int size)
        {
            GUIFixedSpace space = new GUIFixedSpace(size);
            AddElement(space);
            return space;
        }

        /// <summary>
        /// Adds a new horizontal layout as a child of this layout. Layout is inserted
        /// before the element at the specified index.
        /// </summary>
        /// <param name="idx">Index to insert the layout at. This must be in range [0, GetNumChildren()).</param>
        /// <param name="options">Options that allow you to control how is the layout positioned and sized.</param>
        /// <returns>Newly created horizontal layout.</returns>
        public GUILayoutX InsertLayoutX(int idx, params GUIOption[] options)
        {
            GUILayoutX layout = new GUILayoutX(options);
            InsertElement(idx, layout);
            return layout;
        }

        /// <summary>
        /// Adds a new vertical layout as a child of this layout. Layout is inserted
        /// before the element at the specified index.
        /// </summary>
        /// <param name="idx">Index to insert the layout at. This must be in range [0, GetNumChildren()).</param>
        /// <param name="options">Options that allow you to control how is the layout positioned and sized.</param>
        /// <returns>Newly created vertical layout.</returns>
        public GUILayoutY InsertLayoutY(int idx, params GUIOption[] options)
        {
            GUILayoutY layout = new GUILayoutY(options);
            InsertElement(idx, layout);
            return layout;
        }

        /// <summary>
        /// Adds a new GUI panel as a child of this layout. Panel is inserted before the element at the specified index.
        /// </summary>
        /// <param name="idx">Index to insert the panel at. This must be in range [0, GetNumChildren()).</param>
        /// <param name="options">Options that allow you to control how is the panel positioned and sized.</param>
        /// <returns>Newly created GUI panel.</returns>
        public GUIPanel InsertPanel(int idx, params GUIOption[] options)
        {
            GUIPanel layout = new GUIPanel(options);
            InsertElement(idx, layout);
            return layout;
        }

        /// <summary>
        /// Adds a new GUI panel as a child of this layout. Panel is inserted before the element at the specified index.
        /// </summary>
        /// <param name="idx">Index at which to insert the panel.</param>
        /// <param name="depth">Depth at which to position the panel. Panels with lower depth will be displayed in front of
        ///                     panels with higher depth. Provided depth is relative to the depth of the parent GUI panel.
        ///                     The depth value will be clamped if outside of the depth range of the parent GUI panel.</param>
        /// <param name="depthRangeMin">Smallest depth offset allowed by any child GUI panels. If a child panel has a depth
        ///                             offset lower than this value it will be clamped.</param>
        /// <param name="depthRangeMax">Largest depth offset allowed by any child GUI panels. If a child panel has a depth
        ///                             offset higher than this value it will be clamped.</param>
        /// <param name="options">Options that allow you to control how is the panel positioned and sized.</param>
        /// <returns>Newly created GUI panel.</returns>
        public GUIPanel InsertPanel(int idx, Int16 depth = 0, ushort depthRangeMin = ushort.MaxValue,
            ushort depthRangeMax = ushort.MaxValue, params GUIOption[] options)
        {
            GUIPanel layout = new GUIPanel(depth, depthRangeMin, depthRangeMax, options);
            InsertElement(idx, layout);
            return layout;
        }

        /// <summary>
        /// Adds a new flexible space as a child of this layout. Flexible space expands to fill all available space in the
        /// layout. is inserted before the element at the specified index.
        /// </summary>
        /// <returns>Newly created flexible space.</returns>
        public GUIFlexibleSpace InsertFlexibleSpace(int idx)
        {
            GUIFlexibleSpace space = new GUIFlexibleSpace();
            InsertElement(idx, space);
            return space;
        }

        /// <summary>
        /// Adds a new fixed space object. Fixed space inserts a blank space with specific width or height (depending on
        /// layout type) in the layout. Space is inserted after all existing elements.
        /// </summary>
        /// <param name="idx">Index at which to insert the space.</param>
        /// <param name="size">Size of the space in pixels. This will represent either width or height depending whether the
        /// layout is vertical or horizontal.</param>
        /// <returns>Newly created fixed space.</returns>
        public GUIFixedSpace InsertSpace(int idx, int size)
        {
            GUIFixedSpace space = new GUIFixedSpace(size);
            InsertElement(idx, space);
            return space;
        }

        /// <summary>
        /// Destroys all children of this layout.
        /// </summary>
        public void Clear()
        {
            Internal_Clear(mCachedPtr);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_CreateInstanceYFromScrollArea(GUILayout instance, GUIScrollArea parentArea);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_CreateInstanceX(GUILayout instance, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_CreateInstanceY(GUILayout instance, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_CreateInstancePanel(GUILayout instance, Int16 depth, ushort depthRangeMin,
            ushort depthRangeMax, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_AddElement(IntPtr instance, IntPtr element);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_InsertElement(IntPtr instance, int index, IntPtr element);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern int Internal_GetChildCount(IntPtr instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern GUIElement Internal_GetChild(IntPtr instance, int index);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void Internal_Clear(IntPtr instance);
    }

    /** @} */
}
