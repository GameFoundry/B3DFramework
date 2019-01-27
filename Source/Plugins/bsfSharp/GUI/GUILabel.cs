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
    /// GUI element that displays text.
    /// </summary>
    public sealed class GUILabel : GUIElement
    {
        /// <summary>
        /// Creates a new label element.
        /// </summary>
        /// <param name="content">Content to display on the label.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUILabel(GUIContent content, string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, style, options);
        }

        /// <summary>
        /// Creates a new label element.
        /// </summary>
        /// <param name="content">Content to display on the label.</param>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUILabel(GUIContent content, string style = "")
        {
            Internal_CreateInstance(this, ref content, style, new GUIOption[0]);
        }

        /// <summary>
        /// Creates a new label element.
        /// </summary>
        /// <param name="content">Content to display on the label.</param>
        /// <param name="options">Options that allow you to control how is the element positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUILabel(GUIContent content, params GUIOption[] options)
        {
            Internal_CreateInstance(this, ref content, "", options);
        }

        /// <summary>
        /// Updates the contents display on the label.
        /// </summary>
        /// <param name="content">Content to display on the label.</param>
        public void SetContent(GUIContent content)
        {
            Internal_SetContent(mCachedPtr, ref content);
        }

        /// <summary>
        /// Colors the element with a specific tint.
        /// </summary>
        /// <param name="color">Tint to apply to the element.</param>
        public void SetTint(Color color)
        {
            Internal_SetTint(mCachedPtr, ref color);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUILabel instance, ref GUIContent content, string style, 
            GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetContent(IntPtr nativeInstance, ref GUIContent content);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
