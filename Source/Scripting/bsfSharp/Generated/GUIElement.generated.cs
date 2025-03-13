//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>
	/// Base class for all GUI elements. Provides general functionality such as element size/position, as well as handling 
	/// child/parent relationships.
	/// </summary>
	[ShowInInspector]
	public partial class GUIElement : ScriptObject
	{
		private GUIElement(bool __dummy0) { }
		protected GUIElement() { }

		/// <summary>
		/// Hides or shows this element and recursively applies the same state to all the child elements. This will not remove 
		/// the element from the layout, the room for it will still be reserved but it just won&apos;t be visible.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Hidden
		{
			get { return Internal_IsHidden(mCachedPtr); }
			set { Internal_SetHidden(mCachedPtr, value); }
		}

		/// <summary>
		/// Activates or deactives this element and recursively applies the same state to all the child elements. This has the 
		/// same effect as setVisible(), but when disabled it will also remove the element from the layout, essentially having 
		/// the same effect is if you destroyed the element.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Active
		{
			get { return Internal_IsActive(mCachedPtr); }
			set { Internal_SetActive(mCachedPtr, value); }
		}

		/// <summary>
		/// Disables or enables the element. Disabled elements cannot be interacted with and have a faded out appearance.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Disabled
		{
			get { return Internal_IsDisabled(mCachedPtr); }
			set { Internal_SetDisabled(mCachedPtr, value); }
		}

		/// <summary>
		/// Returns width/height of the GUI element. This will be the fixed width/height if set by the user, or automatically 
		/// determined by the layout update pass if not fixed. Size is provided in logical pixel units.
		/// </summary>
		[NativeWrapper]
		public TSize2<TUnitValue<int,LogicalPixel>> LayoutCalculatedSize
		{
			get
			{
				TSize2<TUnitValue<int,LogicalPixel>> temp;
				Internal_CalculateSizeInLayout(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Calculates bounds of the GUI element, relative to the parent GUI widget, with scaling applied.  The values are 
		/// provided in physical pixel units.
		/// </summary>
		[NativeWrapper]
		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> AbsoluteBounds
		{
			get
			{
				TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
				Internal_CalculateAbsoluteBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Calculates bounds of the GUI element in screen space.</summary>
		[NativeWrapper]
		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> ScreenBounds
		{
			get
			{
				TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
				Internal_CalculateScreenBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Returns parent GUI base element.</summary>
		[NativeWrapper]
		public GUIElement Parent
		{
			get { return Internal_GetParent(mCachedPtr); }
		}

		/// <summary>
		/// Sets element position relative to parent GUI panel. Values should be provided in logical pixel units.
		///
		/// Be aware that this value will be ignored if GUI element is part of a layout since then the layout controls its 
		/// placement.
		/// </summary>
		public void SetPosition(TUnitValue<int,LogicalPixel> x, TUnitValue<int,LogicalPixel> y)
		{
			Internal_SetPosition(mCachedPtr, ref x, ref y);
		}

		/// <summary>
		/// Sets element position relative to parent GUI panel. Values should be provided in logical pixel units.
		///
		/// Be aware that this value will be ignored if GUI element is part of a layout since then the layout controls its 
		/// placement.
		/// </summary>
		public void SetPosition(TVector2<TUnitValue<int,LogicalPixel>> position)
		{
			Internal_SetPosition0(mCachedPtr, ref position);
		}

		/// <summary>Sets fixed element width. Value should be in logical pixel units.</summary>
		public void SetWidth(TUnitValue<int,LogicalPixel> width)
		{
			Internal_SetWidth(mCachedPtr, ref width);
		}

		/// <summary>
		/// Sets flexible element width. Element will be resized according to its contents and parent layout but will always stay 
		/// within the provided range. If maximum width is zero, the element is allowed to expand as much as it needs. Values 
		/// should be in logical pixel units.
		/// </summary>
		public void SetFlexibleWidth(TUnitValue<int,LogicalPixel> minWidth, TUnitValue<int,LogicalPixel> maxWidth)
		{
			Internal_SetFlexibleWidth(mCachedPtr, ref minWidth, ref maxWidth);
		}

		/// <summary>Sets fixed element height. Value should be in logical pixel units.</summary>
		public void SetHeight(TUnitValue<int,LogicalPixel> height)
		{
			Internal_SetHeight(mCachedPtr, ref height);
		}

		/// <summary>
		/// Sets flexible element height. Element will be resized according to its contents and parent layout but will always 
		/// stay within the provided range. If maximum height is zero, the element is allowed to expand as much as it needs. 
		/// Values provided should be in logical pixel units.
		/// </summary>
		public void SetFlexibleHeight(TUnitValue<int,LogicalPixel> minHeight, TUnitValue<int,LogicalPixel> maxHeight)
		{
			Internal_SetFlexibleHeight(mCachedPtr, ref minHeight, ref maxHeight);
		}

		/// <summary>Sets fixed width and height of a GUI element. Values provided should be in logical pixel units.</summary>
		public void SetSize(TSize2<TUnitValue<int,LogicalPixel>> size)
		{
			Internal_SetSize(mCachedPtr, ref size);
		}

		/// <summary>Resets element size constraints to their initial values dictated by the element&apos;s style.</summary>
		public void ResetSizeConstraints()
		{
			Internal_ResetSizeConstraints(mCachedPtr);
		}

		/// <summary>
		/// Calculates position of the GUI element, relative to the provided parent element (or parent panel if null). The value 
		/// is provided in logical pixel units.
		/// </summary>
		/// <param name="relativeTo">
		/// Parent element of the provided element relative to which to return the position. If null the position relative to 
		/// parent panel is returned. Behavior is undefined if provided parent is not a parent of the element.
		/// </param>
		public TVector2<TUnitValue<int,LogicalPixel>> CalculatePositionRelativeTo(GUIElement relativeTo)
		{
			TVector2<TUnitValue<int,LogicalPixel>> temp;
			Internal_CalculatePositionRelativeTo(mCachedPtr, relativeTo, out temp);
			return temp;
		}

		/// <summary>
		/// Calculates bounds of the GUI element, relative to the provided parent element (or parent panel if null), with scaling 
		/// applied. The values are provided in physical pixel units.
		/// </summary>
		/// <param name="relativeTo">
		/// Parent element of the provided element relative to which to return the bounds. If null the bounds relative to parent 
		/// panel are returned. Behavior is undefined if provided parent is not a parent of the element.
		/// </param>
		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> CalculateAbsoluteBoundsRelativeTo(GUIElement relativeTo)
		{
			TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
			Internal_CalculateAbsoluteBoundsRelativeTo(mCachedPtr, relativeTo, out temp);
			return temp;
		}

		/// <summary>Converts a point relative to the parent widget, into a point relative to this element.</summary>
		public TVector2<TUnitValue<int,LogicalPixel>> WidgetToElementSpace(TVector2<TUnitValue<int,PhysicalPixel>> point)
		{
			TVector2<TUnitValue<int,LogicalPixel>> temp;
			Internal_WidgetToElementSpace(mCachedPtr, ref point, out temp);
			return temp;
		}

		/// <summary>Converts a point relative to this element, into a point relative to the parent widget.</summary>
		public TVector2<TUnitValue<int,PhysicalPixel>> ElementToWidgetSpace(TVector2<TUnitValue<int,LogicalPixel>> point)
		{
			TVector2<TUnitValue<int,PhysicalPixel>> temp;
			Internal_ElementToWidgetSpace(mCachedPtr, ref point, out temp);
			return temp;
		}

		/// <summary>Converts an area relative to the parent widget, into an area relative to this element.</summary>
		public TArea2<TUnitValue<int,LogicalPixel>,TUnitValue<int,LogicalPixel>> WidgetToElementSpace(TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> area)
		{
			TArea2<TUnitValue<int,LogicalPixel>,TUnitValue<int,LogicalPixel>> temp;
			Internal_WidgetToElementSpace0(mCachedPtr, ref area, out temp);
			return temp;
		}

		/// <summary>Converts an area relative to this element, into an area relative to the parent widget.</summary>
		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> ElementToWidgetSpace(TArea2<TUnitValue<int,LogicalPixel>,TUnitValue<int,LogicalPixel>> area)
		{
			TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
			Internal_ElementToWidgetSpace0(mCachedPtr, ref area, out temp);
			return temp;
		}

		/// <summary>
		/// Destroy the element. Removes it from parent and widget, and queues it for deletion. Element memory will be released 
		/// delayed, next frame.
		/// </summary>
		public void Destroy()
		{
			Internal_Destroy(mCachedPtr);
		}

		/// <summary>
		/// Sets flexible element width. Element will be resized according to its contents and parent layout but will always stay 
		/// within the provided range. If maximum width is zero, the element is allowed to expand as much as it needs. Values 
		/// should be in logical pixel units.
		/// </summary>
		public void SetFlexibleWidth(TUnitValue<int,LogicalPixel> minWidth)
		{
			TUnitValue<int,LogicalPixel> maxWidth = new TUnitValue<int,LogicalPixel>(0);
			Internal_SetFlexibleWidth(mCachedPtr, ref minWidth, ref maxWidth);
		}

		/// <summary>
		/// Sets flexible element width. Element will be resized according to its contents and parent layout but will always stay 
		/// within the provided range. If maximum width is zero, the element is allowed to expand as much as it needs. Values 
		/// should be in logical pixel units.
		/// </summary>
		public void SetFlexibleWidth()
		{
			TUnitValue<int,LogicalPixel> minWidth = new TUnitValue<int,LogicalPixel>(0);
			TUnitValue<int,LogicalPixel> maxWidth = new TUnitValue<int,LogicalPixel>(0);
			Internal_SetFlexibleWidth(mCachedPtr, ref minWidth, ref maxWidth);
		}

		/// <summary>
		/// Sets flexible element height. Element will be resized according to its contents and parent layout but will always 
		/// stay within the provided range. If maximum height is zero, the element is allowed to expand as much as it needs. 
		/// Values provided should be in logical pixel units.
		/// </summary>
		public void SetFlexibleHeight(TUnitValue<int,LogicalPixel> minHeight)
		{
			TUnitValue<int,LogicalPixel> maxHeight = new TUnitValue<int,LogicalPixel>(0);
			Internal_SetFlexibleHeight(mCachedPtr, ref minHeight, ref maxHeight);
		}

		/// <summary>
		/// Sets flexible element height. Element will be resized according to its contents and parent layout but will always 
		/// stay within the provided range. If maximum height is zero, the element is allowed to expand as much as it needs. 
		/// Values provided should be in logical pixel units.
		/// </summary>
		public void SetFlexibleHeight()
		{
			TUnitValue<int,LogicalPixel> minHeight = new TUnitValue<int,LogicalPixel>(0);
			TUnitValue<int,LogicalPixel> maxHeight = new TUnitValue<int,LogicalPixel>(0);
			Internal_SetFlexibleHeight(mCachedPtr, ref minHeight, ref maxHeight);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPosition(IntPtr thisPtr, ref TUnitValue<int,LogicalPixel> x, ref TUnitValue<int,LogicalPixel> y);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPosition0(IntPtr thisPtr, ref TVector2<TUnitValue<int,LogicalPixel>> position);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetWidth(IntPtr thisPtr, ref TUnitValue<int,LogicalPixel> width);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlexibleWidth(IntPtr thisPtr, ref TUnitValue<int,LogicalPixel> minWidth, ref TUnitValue<int,LogicalPixel> maxWidth);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHeight(IntPtr thisPtr, ref TUnitValue<int,LogicalPixel> height);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFlexibleHeight(IntPtr thisPtr, ref TUnitValue<int,LogicalPixel> minHeight, ref TUnitValue<int,LogicalPixel> maxHeight);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetSize(IntPtr thisPtr, ref TSize2<TUnitValue<int,LogicalPixel>> size);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ResetSizeConstraints(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHidden(IntPtr thisPtr, bool hidden);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetActive(IntPtr thisPtr, bool active);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDisabled(IntPtr thisPtr, bool disabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateSizeInLayout(IntPtr thisPtr, out TSize2<TUnitValue<int,LogicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculatePositionRelativeTo(IntPtr thisPtr, GUIElement relativeTo, out TVector2<TUnitValue<int,LogicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateAbsoluteBoundsRelativeTo(IntPtr thisPtr, GUIElement relativeTo, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateAbsoluteBounds(IntPtr thisPtr, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateScreenBounds(IntPtr thisPtr, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WidgetToElementSpace(IntPtr thisPtr, ref TVector2<TUnitValue<int,PhysicalPixel>> point, out TVector2<TUnitValue<int,LogicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ElementToWidgetSpace(IntPtr thisPtr, ref TVector2<TUnitValue<int,LogicalPixel>> point, out TVector2<TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_WidgetToElementSpace0(IntPtr thisPtr, ref TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> area, out TArea2<TUnitValue<int,LogicalPixel>,TUnitValue<int,LogicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ElementToWidgetSpace0(IntPtr thisPtr, ref TArea2<TUnitValue<int,LogicalPixel>,TUnitValue<int,LogicalPixel>> area, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Destroy(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIElement Internal_GetParent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsHidden(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsActive(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsDisabled(IntPtr thisPtr);
	}

	/** @} */
}
