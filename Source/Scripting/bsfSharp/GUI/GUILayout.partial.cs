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

    public partial class GUILayout 
    {
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
        public GUIFixedSpace AddSpace(GUILogicalUnit size)
        {
            GUIFixedSpace space = new GUIFixedSpace(size);
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
    }

    /** @} */
}
