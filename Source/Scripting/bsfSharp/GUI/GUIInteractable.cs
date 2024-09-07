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
    /// Base class for all GUI elements that can be interacted with (e.g. respond to input). 
    /// </summary>
    public abstract class GUIInteractable : GUIElement
    {
        /// <summary>
        /// Triggered when a GUI element receives keyboard focus.
        /// </summary>
        public Action OnFocusGained;

        /// <summary>
        /// Triggered when a GUI element loses keyboard focus.
        /// </summary>
        public Action OnFocusLost;

        /// <summary>
        /// Name of the style that determines the appearance of this GUI element.
        /// </summary>
        public string Style
        {
            get { return Internal_GetStyle(mCachedPtr); }
            set { Internal_SetStyle(mCachedPtr, value); }
        }


        /// <summary>
        /// Assigns or removes keyboard focus on this element.
        /// </summary>
        public bool Focus
        {
            set { Internal_SetFocus(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines will this element block elements underneath it from receiving events like pointer click, hover
        /// on/off or be able to gain focus. True by default.
        /// </summary>
        public bool Blocking
        {
            get { return Internal_GetBlocking(mCachedPtr); }
            set { Internal_SetBlocking(mCachedPtr, value); }
        }

        /// <summary>
        /// Determines if the element can be navigated to by using keys/buttons (e.g. the 'Tab' button on the keyboard.
        /// </summary>
        public bool AcceptsKeyFocus
        {
            get { return Internal_GetAcceptsKeyFocus(mCachedPtr); }
            set { Internal_SetAcceptsKeyFocus(mCachedPtr, value); }
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
        /// Assigns a new context menu that will be opened when the element is right clicked.
        /// </summary>
        /// <param name="menu">Object containing context menu contents. Can be null if no menu is wanted.</param>
        public void SetContextMenu(ContextMenu menu)
        {
            IntPtr menuPtr = IntPtr.Zero;
            if (menu != null)
                menuPtr = menu.GetCachedPtr();

            Internal_SetContextMenu(mCachedPtr, menuPtr);
        }

        /// <summary>
        /// Triggered by the native interop object when the element gains keyboard focus.
        /// </summary>
        private void Internal_OnFocusGained()
        {
            if (OnFocusGained != null)
                OnFocusGained();
        }

        /// <summary>
        /// Triggered by the native interop object when the element loses keyboard focus.
        /// </summary>
        private void Internal_OnFocusLost()
        {
            if (OnFocusLost != null)
                OnFocusLost();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFocus(IntPtr nativeInstance, bool focus);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetBlocking(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetBlocking(IntPtr nativeInstance, bool blocking);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetAcceptsKeyFocus(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetAcceptsKeyFocus(IntPtr nativeInstance, bool accepts);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetContextMenu(IntPtr nativeInstance, IntPtr contextMenu);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetStyle(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStyle(IntPtr nativeInstance, string style);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetTint(IntPtr nativeInstance, ref Color tint);
    }

    /** @} */
}
