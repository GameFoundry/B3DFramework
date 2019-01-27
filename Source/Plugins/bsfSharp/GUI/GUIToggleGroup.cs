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
    /// Object that allows you to group multiple GUI toggle buttons. Only one button among the grouped ones can be active.
    /// </summary>
    public sealed class GUIToggleGroup : ScriptObject
    {
        /// <summary>
        /// Creates a new toggle group.
        /// </summary>
        /// <param name="allowAllOff">If true all of the toggle buttons can be turned off, if false one will always be
        ///                           turned on.</param>
        public GUIToggleGroup(bool allowAllOff = false)
        {
            Internal_CreateInstance(this, allowAllOff);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CreateInstance(GUIToggleGroup instance, bool allowAllOff);
    }

    /** @} */
}
