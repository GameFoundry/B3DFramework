//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup GUI_Engine
     *  @{
     */

    /// <summary>
    /// Base class for all GUI elements. Every GUI element can at least be positioned in it's parent layout/panel and be 
    /// hidden/visible, focused/unfocused and assigned a context menu.
    /// </summary>
    public abstract class GUIElement : ScriptObject
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
        /// Returns the layout this element belongs to, if any.
        /// </summary>
        public GUILayout Parent
        {
            get { return Internal_GetParent(mCachedPtr); }
        }

        /// <summary>
        /// Name of the style that determines the appearance of this GUI element.
        /// </summary>
        public string Style
        {
            get { return Internal_GetStyle(mCachedPtr); }
            set { Internal_SetStyle(mCachedPtr, value); }
        }

        /// <summary>
        /// Gets or sets non-clipped bounds of the GUI element. Relative to a parent GUI panel.
        /// </summary>
        public Rect2I Bounds
        {
            get
            {
                Rect2I bounds;
                Internal_GetBounds(mCachedPtr, out bounds);
                return bounds;
            }
            set { Internal_SetBounds(mCachedPtr, ref value); }
        }

        /// <summary>
        /// Returns the non-clipped bounds of the GUI element in screen space.
        /// </summary>
        public Rect2I ScreenBounds
        {
            get
            {
                Rect2I bounds;
                Internal_GetScreenBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Returns the non-clipped bounds of the GUI element including the margins. Relative to a parent GUI panel.
        /// </summary>
        public Rect2I VisualBounds
        {
            get
            {
                Rect2I bounds;
                Internal_GetVisualBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Makes the element hidden or visible. This will not affect the layout as the room for the element will still
        /// be reserved in the parent layout, use <see cref="Active"/> if you need to affect the layout as well.
        /// </summary>
        public bool Visible
        {
            set { Internal_SetVisible(mCachedPtr, value); }
            get { return Internal_GetVisible(mCachedPtr); }
        }

        /// <summary>
        /// Activates or deactivates the element, making it hidden or visible. When disabled it is essentially removed from 
        /// the parent achieving the similar effect as if the element was destroyed.
        /// </summary>
        public bool Active
        {
            set { Internal_SetActive(mCachedPtr, value); }
            get { return Internal_GetActive(mCachedPtr); }
        }

        /// <summary>
        /// Disables or enables the element. Disabled elements cannot be interacted with and have a faded out appearance.
        /// </summary>
        public bool Disabled
        {
            set { Internal_SetDisabled(mCachedPtr, value); }
            get { return Internal_GetDisabled(mCachedPtr); }
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
        /// Destroys this element and all its children. Removes the element from parent layout/panel.
        /// </summary>
        /// <remarks>
        /// Calling methods on a destroyed element is a no-operation. Destroyed elements aren't allowed to be assigned as 
        /// children of other elements, or be parents of other elements.
        /// </remarks>
        public virtual void Destroy()
        {
            Internal_Destroy(mCachedPtr);
        }

        /// <summary>
        /// Sets element position relative to parent GUI panel.
        /// </summary>
        /// <param name="x">X position of the element in pixels, relative to parent GUI panel.</param>
        /// <param name="y">Y position of the element in pixels, relative to parent GUI panel.</param>
        /// <remarks>
        /// Be aware that this value will be ignored if GUI element is part of a layout because the layout controls placement 
        /// of child elements.
        /// </remarks>
        public void SetPosition(int x, int y)
        {
            Internal_SetPosition(mCachedPtr, x, y);
        }

        /// <summary>
        /// Sets a fixed element width.
        /// </summary>
        /// <param name="width">Width in pixels.</param>
        public void SetWidth(int width)
        {
            Internal_SetWidth(mCachedPtr, width);
        }

        /// <summary>
        /// Sets a flexible element width. Element will be resized according to its contents and parent layout but will 
        /// always stay within the provided range.
        /// </summary>
        /// <param name="minWidth">Minimum width in pixels. Element will never be smaller than this width.</param>
        /// <param name="maxWidth">Maximum width in pixels. Element will never be larger than this width. Specify zero for 
        ///                        unlimited width.</param>
        public void SetFlexibleWidth(int minWidth, int maxWidth)
        {
            Internal_SetFlexibleWidth(mCachedPtr, minWidth, maxWidth);
        }

        /// <summary>
        /// Sets a fixed element height.
        /// </summary>
        /// <param name="height">Height in pixels.</param>
        public void SetHeight(int height)
        {
            Internal_SetHeight(mCachedPtr, height);
        }

        /// <summary>
        /// Sets a flexible element height. Element will be resized according to its contents and parent layout but will 
        /// always stay within the provided range.
        /// </summary>
        /// <param name="minHeight">Minimum height in pixels. Element will never be smaller than this height.</param>
        /// <param name="maxHeight">Maximum height in pixels. Element will never be larger than this height. Specify zero 
        ///                         for unlimited height.</param>
        public void SetFlexibleHeight(int minHeight, int maxHeight)
        {
            Internal_SetFlexibleHeight(mCachedPtr, minHeight, maxHeight);
        }

        /// <summary>
        /// Resets element bounds to their initial values dictated by the element's style.
        /// </summary>
        public void ResetDimensions()
        {
            Internal_ResetDimensions(mCachedPtr);
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
        private static extern bool Internal_GetVisible(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetVisible(IntPtr nativeInstance, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetActive(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetActive(IntPtr nativeInstance, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetDisabled(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDisabled(IntPtr nativeInstance, bool disabled);

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
        private static extern GUILayout Internal_GetParent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition(IntPtr nativeInstance, int x, int y);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetWidth(IntPtr nativeInstance, int width);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFlexibleWidth(IntPtr nativeInstance, int minWidth, int maxWidth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHeight(IntPtr nativeInstance, int height);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFlexibleHeight(IntPtr nativeInstance, int minHeight, int maxHeight);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ResetDimensions(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetBounds(IntPtr nativeInstance, out Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetBounds(IntPtr nativeInstance, ref Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetVisualBounds(IntPtr nativeInstance, out Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScreenBounds(IntPtr nativeInstance, out Rect2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetContextMenu(IntPtr nativeInstance, IntPtr contextMenu);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string Internal_GetStyle(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetStyle(IntPtr nativeInstance, string style);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr nativeInstance);
    }

    /** @} */
}
