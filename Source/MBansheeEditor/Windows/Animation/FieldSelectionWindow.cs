//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Library
     *  @{
     */

    [DefaultSize(250, 350)]
    internal class FieldSelectionWindow : DropDownWindow
    {
        private GUIFieldSelector guiFieldSelector;

        public Action<string, SerializableProperty.FieldType> OnFieldSelected;

        /// <summary>
        /// Constructs the drop down window.
        /// </summary>
        public FieldSelectionWindow()
        {
            GUILayout layoutY = GUI.AddLayoutY();
            layoutY.AddSpace(5);
            GUILayout layoutX = layoutY.AddLayoutX();
            layoutY.AddSpace(5);

            layoutX.AddSpace(5);
            GUILayout selectorLayout = layoutX.AddLayoutY();
            layoutX.AddSpace(5);

            guiFieldSelector = new GUIFieldSelector(selectorLayout, Selection.SceneObject);
            guiFieldSelector.OnElementSelected += (so, comp, path, type) =>
            {
                OnFieldSelected?.Invoke(path, type);
            };
        }
    }

    /** @} */
}
