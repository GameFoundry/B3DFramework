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
    /// Provides access to a global <see cref="GUIPanel"/> that renders GUI on the main viewport. Use 
    /// <see cref="GUIWidget"/> if you need more control over the placement of GUI, or require it to be rendered to a 
    /// different viewport.
    /// </summary>
    public static class GUI
    {
        private static GUISkin skin;
        private static GUIPanel panel; // Populated by runtime

        /// <summary>
        /// Skin used for rendering all the GUI elements.
        /// </summary>
        public static GUISkin Skin
        {
            get { return skin; }
            set
            {
                skin = value;

                IntPtr skinPtr = IntPtr.Zero;
                if (value != null)
                    skinPtr = value.GetCachedPtr();

                Internal_SetSkin(skinPtr);
            }
        }

        /// <summary>
        /// Container into which all GUI elements should be placed.
        /// </summary>
        public static GUIPanel Panel
        {
            get { return panel; }
        }

        /// <summary>
        /// Used by the runtime to set the primary panel.
        /// </summary>
        /// <param name="panel">Primary panel of the widget.</param>
        private static void SetPanel(GUIPanel panel)
        {
            // We can't set this directly through the field because there is an issue with Mono and static fields
            GUI.panel = panel;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetSkin(IntPtr skin);
    }

    /** @} */
}
