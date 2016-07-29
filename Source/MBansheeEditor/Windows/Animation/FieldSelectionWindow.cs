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
            guiFieldSelector = new GUIFieldSelector(GUI, Selection.SceneObject, Width, Height);
            guiFieldSelector.OnElementSelected += (so, comp, path, type) =>
            {
                OnFieldSelected?.Invoke(path, type);
                Close();
            };
        }
    }

    /** @} */
}
