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
    /// Base class for all GUI elements. Every GUI element can at least be positioned in it's parent layout/panel and be
    /// hidden/visible, focused/unfocused and assigned a context menu.
    /// </summary>
    public abstract class GUIElement : ScriptObject
    {
        /// <summary>
        /// Returns bounds of the GUI element, relative to the parent GUI widget. Absolute values represent the final
        /// position and size of the GUI element, affected by DPI scale, parent scale and self scale. The values are
        /// provided in physical pixel units.
        ///
        /// Always returns value calculated by last layout update. This means out of date value may be returned if the
        /// layout has been dirtied since then. // TODO - This sentence is not true until below is implemented
        /// </summary>
        public GUIPhysicalArea AbsoluteBounds // TODO - Refactor this to always return cached bounds, add CalculateAbsoluteBounds for other use cases
        {
            get
            {
                GUIPhysicalArea bounds;
                Internal_CalculateAbsoluteBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Calculates bounds of the GUI element in screen space.
        /// <remarks>This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.</remarks>
        /// </summary>
        public Area2I ScreenBounds
        {
            get
            {
                Area2I bounds;
                Internal_GetScreenBounds(mCachedPtr, out bounds);
                return bounds;
            }
        }

        /// <summary>
        /// Returns the position of the GUI element relative to the first parent GUI panel. Values are provided in logical pixel units.
        /// </summary>
        /// <remarks>This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.</remarks>
        public GUILogicalPoint LayoutCalculatedPositionRelativeToParentPanel => CalculatePositionRelativeTo();

        /// <summary>
        /// Returns width/height of the GUI element. This will be the fixed width/height if set by the user, or automatically
        /// determined by the layout update pass if not fixed. Size is provided in logical pixel units.
        /// 
        /// Always returns value calculated by last layout update. This means out of date value may be returned if the
        /// layout has been dirtied since then.
        /// </summary>
        public GUILogicalSize LayoutCalculatedSize 
        {
            get
            {
                GUILogicalSize size;
                Internal_GetLayoutCalculatedSize(mCachedPtr, out size);
                return size;
            }
        }

        /// <summary>
        /// Makes the element hidden or visible. This will not affect the layout as the room for the element will still
        /// be reserved in the parent layout, use <see cref="Active"/> if you need to affect the layout as well.
        /// </summary>
        public bool Hidden
        {
            set { Internal_SetHidden(mCachedPtr, value); }
            get { return Internal_GetHidden(mCachedPtr); }
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
        /// Returns the layout this element belongs to, if any.
        /// </summary>
        public GUILayout Parent
        {
            get { return Internal_GetParent(mCachedPtr); }
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
        /// <param name="x">X position of the element in logical pixel units, relative to parent GUI panel.</param>
        /// <param name="y">Y position of the element in logical pixel units, relative to parent GUI panel.</param>
        /// <remarks>
        /// Be aware that this value will be ignored if GUI element is part of a layout because the layout controls placement of child elements.
        /// </remarks>
        public void SetPosition(GUILogicalUnit x, GUILogicalUnit y)
        {
            GUILogicalPoint point = new GUILogicalPoint(x, y);
            Internal_SetPosition2(mCachedPtr, ref point);
        }

        /// <summary>
        /// Sets element position relative to parent GUI panel.
        /// </summary>
        /// <param name="position">X/Y position of the element in logical pixel units, relative to parent GUI panel.</param>
        /// <remarks>
        /// Be aware that this value will be ignored if GUI element is part of a layout because the layout controls placement of child elements.
        /// </remarks>
        public void SetPosition(GUILogicalPoint position)
        {
            Internal_SetPosition2(mCachedPtr, ref position);
        }

        /// <summary>
        /// Sets a fixed element width.
        /// </summary>
        /// <param name="width">Width in logical pixel units.</param>
        public void SetWidth(GUILogicalUnit width)
        {
            Internal_SetWidth2(mCachedPtr, ref width);
        }

        /// <summary>
        /// Sets a flexible element width. Element will be resized according to its contents and parent layout but will
        /// always stay within the provided range.
        /// </summary>
        /// <param name="minWidth">Minimum width in logical pixel units. Element will never be smaller than this width.</param>
        /// <param name="maxWidth">Maximum width in logical pixel units. Element will never be larger than this width. Specify zero for
        ///                        unlimited width.</param>
        public void SetFlexibleWidth(int minWidth, int maxWidth)
        {
            Internal_SetFlexibleWidth(mCachedPtr, minWidth, maxWidth);
        }

        /// <summary>
        /// Sets a fixed element height.
        /// </summary>
        /// <param name="height">Height in logical pixel units.</param>
        public void SetHeight(GUILogicalUnit height)
        {
            Internal_SetHeight2(mCachedPtr, ref height);
        }

        /// <summary>
        /// Sets a flexible element height. Element will be resized according to its contents and parent layout but will
        /// always stay within the provided range.
        /// </summary>
        /// <param name="minHeight">Minimum height in logical pixel units. Element will never be smaller than this height.</param>
        /// <param name="maxHeight">Maximum height in logical pixel units. Element will never be larger than this height. Specify zero
        ///                         for unlimited height.</param>
        public void SetFlexibleHeight(int minHeight, int maxHeight)
        {
            Internal_SetFlexibleHeight(mCachedPtr, minHeight, maxHeight);
        }

        /// <summary>
        /// Sets a fixed element width & height.
        /// </summary>
        /// <param name="size">Width/height in logical pixel units.</param>
        public void SetSize(GUILogicalSize size)
        {
            Internal_SetSize2(mCachedPtr, ref size);
        }

        /// <summary>
        /// Resets element size constraints to their initial values dictated by the element's style.
        /// </summary>
        public void ResetSizeConstraints()
        {
            Internal_ResetSizeConstraints(mCachedPtr);
        }

        /// <summary>
        /// Calculates bounds of the GUI element, relative to the provided parent layout (or parent panel if null). Absolute values represent
        /// the final position and size of the GUI element, affected by DPI scale, parent scale and self scale. 
        /// </summary>
        /// <param name="relativeTo">
        /// Parent layout of the provided element relative to which to return the bounds. If null the bounds relative to parent panel are
        /// returned. Behavior is undefined if provided parent is not a parent of the element.
        /// </param>
        /// <returns>Bounds of a GUI element relative to the provided GUI layout.</returns>
        /// <remarks>This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.</remarks>
        public Area2I CalculateAbsoluteBoundsRelativeTo(GUILayout relativeTo = null)
        {
            IntPtr relativeToNative = IntPtr.Zero;
            if (relativeTo != null)
                relativeToNative = relativeTo.GetCachedPtr();

            Area2I output;
            Internal_CalculateAbsoluteBoundsRelativeTo(GetCachedPtr(), relativeToNative, out output);
            return output;
        }

        /// <summary>
        /// Calculates position of the GUI element, relative to the provided parent layout (or parent panel if null).
        /// The value is provided in logical pixel units.
        /// </summary>
        /// <param name="relativeTo">
        /// Parent layout of the provided element relative to which to return the position. If null the position relative to parent panel is
        /// returned. Behavior is undefined if provided parent is not a parent of the element.
        /// </param>
        /// <returns>Position of a GUI element relative to the provided GUI layout.</returns>
        /// <remarks>This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.</remarks>
        public GUILogicalPoint CalculatePositionRelativeTo(GUILayout relativeTo = null)
        {
            IntPtr relativeToNative = IntPtr.Zero;
            if (relativeTo != null)
                relativeToNative = relativeTo.GetCachedPtr();

            GUILogicalPoint output;
            Internal_CalculatePositionRelativeTo(GetCachedPtr(), relativeToNative, out output);
            return output;
        }

        /// <summary>
        /// Converts a point relative to the parent widget, into a point relative to this element.
        /// </summary>
        public GUILogicalPoint WidgetToElementSpace(GUIPhysicalPoint position)
        {
            GUILogicalPoint output;
            Internal_WidgetToElementSpace0(GetCachedPtr(), ref position, out output);

            return output;
        }

        /// <summary>
        /// Converts a point relative to this element, into a point relative to the parent widget.
        /// </summary>
        public GUIPhysicalPoint ElementToWidgetSpace(GUILogicalPoint position)
        {
            GUIPhysicalPoint output;
            Internal_ElementToWidgetSpace0(GetCachedPtr(), ref position, out output);

            return output;
        }

        /// <summary>
        /// Converts an area relative to the parent widget, into an area relative to this element.
        /// </summary>
        public GUILogicalArea WidgetToElementSpace(GUIPhysicalArea area)
        {
            GUILogicalArea output;
            Internal_WidgetToElementSpace1(GetCachedPtr(), ref area, out output);

            return output;
        }

        /// <summary>
        /// Converts an area relative to this element, into an area relative to the parent widget.
        /// </summary>
        public GUIPhysicalArea ElementToWidgetSpace(GUILogicalArea area)
        {
            GUIPhysicalArea output;
            Internal_ElementToWidgetSpace1(GetCachedPtr(), ref area, out output);

            return output;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetHidden(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHidden(IntPtr nativeInstance, bool visible);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetActive(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetActive(IntPtr nativeInstance, bool enabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Internal_GetDisabled(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetDisabled(IntPtr nativeInstance, bool disabled);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetPosition2(IntPtr nativeInstance, ref GUILogicalPoint position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetWidth2(IntPtr nativeInstance, ref GUILogicalUnit width);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFlexibleWidth(IntPtr nativeInstance, int minWidth, int maxWidth);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetHeight2(IntPtr nativeInstance, ref GUILogicalUnit height);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetFlexibleHeight(IntPtr nativeInstance, int minHeight, int maxHeight);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetSize2(IntPtr nativeInstance, ref GUILogicalSize size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ResetSizeConstraints(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CalculateAbsoluteBounds(IntPtr nativeInstance, out GUIPhysicalArea value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetLayoutCalculatedSize(IntPtr nativeInstance, out GUILogicalSize value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_GetScreenBounds(IntPtr nativeInstance, out Area2I value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern GUILayout Internal_GetParent(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_Destroy(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CalculateAbsoluteBoundsRelativeTo(IntPtr nativeInstance, IntPtr relativeTo, out Area2I output);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_CalculatePositionRelativeTo(IntPtr nativeInstance, IntPtr relativeTo, out TVector2<TUnitValue<int, LogicalPixel>> output);

        [MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WidgetToElementSpace0(IntPtr nativeInstance, ref TVector2<TUnitValue<int, PhysicalPixel>> position, out TVector2<TUnitValue<int, LogicalPixel>> outPosition);

        [MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ElementToWidgetSpace0(IntPtr nativeInstance, ref TVector2<TUnitValue<int, LogicalPixel>> position, out TVector2<TUnitValue<int, PhysicalPixel>> outPosition);

        [MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WidgetToElementSpace1(IntPtr nativeInstance, ref TArea2<TUnitValue<int, PhysicalPixel>, TUnitValue<int, PhysicalPixel>> area, out TArea2<TUnitValue<int, LogicalPixel>, TUnitValue<int, LogicalPixel>> outArea);

        [MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ElementToWidgetSpace1(IntPtr nativeInstance, ref TArea2<TUnitValue<int, LogicalPixel>, TUnitValue<int, LogicalPixel>> area, out TArea2<TUnitValue<int, PhysicalPixel>, TUnitValue<int, PhysicalPixel>> outArea);
    }

    /** @} */
}
