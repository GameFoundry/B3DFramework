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
    /// GUI element containing a background image and a fill image that is scaled depending on the percentage set by the 
    /// caller.
    /// </summary>
    public sealed class GUIProgressBar : GUIElement
    {
        /// <summary>
        /// Value that controls the width of the progress bar fill image. Range [0, 1].
        /// </summary>
        public float Percent
        {
            get { return Internal_GetPercent(mCachedPtr); }
            set { Internal_SetPercent(mCachedPtr, value); }
        }

        /// <summary>
        /// Creates a new progress bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        /// <param name="options">Options that allow you to control how is the element  positioned and sized. This will 
        ///                       override any similar options set by style.</param>
        public GUIProgressBar(string style, params GUIOption[] options)
        {
            Internal_CreateInstance(this, style, options);
        }

        /// <summary>
        /// Creates a new progress bar element.
        /// </summary>
        /// <param name="style">Optional style to use for the element. Style controls the look of the element, as well as 
        ///                     default layout options. Style will be retrieved from the active GUISkin. If not specified 
        ///                     default element style is used.</param>
        public GUIProgressBar(string style = "")
        {
            Internal_CreateInstance(this, style, new GUIOption[0]);
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
        private static extern void Internal_CreateInstance(GUIProgressBar instance, string style, GUIOption[] options);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetPercent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPercent(IntPtr nativeInstance, float percent);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color color);
    }

    /** @} */
}
