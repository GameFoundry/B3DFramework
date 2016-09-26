//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    /// <summary>
    /// Drop down window that displays options used by the library window.
    /// </summary>
    [DefaultSize(150, 30)]
    internal class LibraryDropDown : DropDownWindow
    {
        private LibraryWindow parent;

        /// <summary>
        /// Initializes the drop down window by creating the necessary GUI. Must be called after construction and before
        /// use.
        /// </summary>
        /// <param name="parent">Libary window that this drop down window is a part of.</param>
        internal void Initialize(LibraryWindow parent)
        {
            this.parent = parent;

            GUIToggleGroup group = new GUIToggleGroup();

            GUIToggle list16 = new GUIToggle(new LocEdString("16"), group, EditorStyles.Button, GUIOption.FixedWidth(30));
            GUIToggle grid32 = new GUIToggle(new LocEdString("32"), group, EditorStyles.Button, GUIOption.FixedWidth(30));
            GUIToggle grid48 = new GUIToggle(new LocEdString("48"), group, EditorStyles.Button, GUIOption.FixedWidth(30));
            GUIToggle grid64 = new GUIToggle(new LocEdString("64"), group, EditorStyles.Button, GUIOption.FixedWidth(30));

            ProjectViewType activeType = parent.ViewType;
            switch (activeType)
            {
                case ProjectViewType.List16:
                    list16.Value = true;
                    break;
                case ProjectViewType.Grid32:
                    grid32.Value = true;
                    break;
                case ProjectViewType.Grid48:
                    grid48.Value = true;
                    break;
                case ProjectViewType.Grid64:
                    grid64.Value = true;
                    break;
            }

            list16.OnToggled += (active) =>
            {
                if (active)
                    ChangeViewType(ProjectViewType.List16);
            };

            grid32.OnToggled += (active) =>
            {
                if (active)
                    ChangeViewType(ProjectViewType.Grid32);
            };

            grid48.OnToggled += (active) =>
            {
                if (active)
                    ChangeViewType(ProjectViewType.Grid48);
            };

            grid64.OnToggled += (active) =>
            {
                if (active)
                    ChangeViewType(ProjectViewType.Grid64);
            };

            GUILayoutY vertLayout = GUI.AddLayoutY();

            vertLayout.AddFlexibleSpace();
            GUILayoutX contentLayout = vertLayout.AddLayoutX();
            contentLayout.AddFlexibleSpace();
            contentLayout.AddElement(list16);
            contentLayout.AddElement(grid32);
            contentLayout.AddElement(grid48);
            contentLayout.AddElement(grid64);
            contentLayout.AddFlexibleSpace();
            vertLayout.AddFlexibleSpace();
        }

        /// <summary>
        /// Changes the icon view type in the library window.
        /// </summary>
        /// <param name="viewType">Type of icons to display in the library window.</param>
        private void ChangeViewType(ProjectViewType viewType)
        {
            parent.ViewType = viewType;
        }
    }

    /** @} */
}
