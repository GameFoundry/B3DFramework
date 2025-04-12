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

	/// <summary>A GUI element container with support for vertical &amp; horizontal scrolling.</summary>
	[ShowInInspector]
	public partial class GUIScrollArea : GUIInteractable
	{
		private GUIScrollArea(bool __dummy0) { }
		protected GUIScrollArea() { }

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIScrollArea(GUIScrollAreaContent contents, string styleClass, params GUIOption[] options)
		{
			Internal_Create(this, ref contents, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIScrollArea(GUIScrollAreaContent contents, params GUIOption[] options)
		{
			Internal_Create0(this, ref contents, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIScrollArea(string styleClass, params GUIOption[] options)
		{
			Internal_Create1(this, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIScrollArea(params GUIOption[] options)
		{
			Internal_Create2(this, options);
		}

		/// <summary>Returns the scroll area layout that you may use to add elements inside the scroll area.</summary>
		[NativeWrapper]
		public GUILayout Layout
		{
			get { return Internal_GetLayout(mCachedPtr); }
		}

		/// <summary>
		/// Scrolls the contents to the specified position (0 meaning top-most part of the content is visible, and 1 meaning 
		/// bottom-most part is visible).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float VerticalScroll
		{
			get { return Internal_GetVerticalScroll(mCachedPtr); }
			set { Internal_ScrollToVertical(mCachedPtr, value); }
		}

		/// <summary>
		/// Scrolls the contents to the specified position (0 meaning left-most part of the content is visible, and 1 meaning 
		/// right-most part is visible)
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HorizontalScroll
		{
			get { return Internal_GetHorizontalScroll(mCachedPtr); }
			set { Internal_ScrollToHorizontal(mCachedPtr, value); }
		}

		/// <summary>
		/// Returns the bounds of the scroll area not including the scroll bars (meaning only the portion that contains the 
		/// contents).
		/// </summary>
		[NativeWrapper]
		public TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> ContentBounds
		{
			get
			{
				TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> temp;
				Internal_GetContentBounds(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>
		/// Enables/disables culling of child elements. If culling is enabled all child elements that are fully outside of the 
		/// parent visible bounds will be marked as culled. Culled elements will never have their contents or mesh updated, their 
		/// absolute coordinate will not be updated and they wont be drawn This is useful for layouts with a large amount of 
		/// children, but comes with an overhead so it is disabled by default. Note this has no impact on layout update, which 
		/// may still be expensive with many elements.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool EnableCulling
		{
			set { Internal_SetEnableCulling(mCachedPtr, value); }
		}

		/// <summary>Returns the width or height of the scrollbar.</summary>
		[NativeWrapper]
		public TUnitValue<int,LogicalPixel> ScrollBarSize
		{
			get
			{
				TUnitValue<int,LogicalPixel> temp;
				Internal_GetScrollBarSize(mCachedPtr, out temp);
				return temp;
			}
		}

		/// <summary>Scrolls the area up by specified amount of pixels, if possible.</summary>
		public void ScrollUp(TUnitValue<int,PhysicalPixel> pixels)
		{
			Internal_ScrollUp(mCachedPtr, ref pixels);
		}

		/// <summary>Scrolls the area down by specified amount of pixels, if possible.</summary>
		public void ScrollDown(TUnitValue<int,PhysicalPixel> pixels)
		{
			Internal_ScrollDown(mCachedPtr, ref pixels);
		}

		/// <summary>Scrolls the area left by specified amount of pixels, if possible.</summary>
		public void ScrollLeft(TUnitValue<int,PhysicalPixel> pixels)
		{
			Internal_ScrollLeft(mCachedPtr, ref pixels);
		}

		/// <summary>Scrolls the area right by specified amount of pixels, if possible.</summary>
		public void ScrollRight(TUnitValue<int,PhysicalPixel> pixels)
		{
			Internal_ScrollRight(mCachedPtr, ref pixels);
		}

		/// <summary>Scrolls the area up by specified percentage (ranging [0, 1]), if possible.</summary>
		public void ScrollUp(float percent)
		{
			Internal_ScrollUp0(mCachedPtr, percent);
		}

		/// <summary>Scrolls the area down by specified percentage (ranging [0, 1]), if possible.</summary>
		public void ScrollDown(float percent)
		{
			Internal_ScrollDown0(mCachedPtr, percent);
		}

		/// <summary>Scrolls the area left by specified percentage (ranging [0, 1]), if possible.</summary>
		public void ScrollLeft(float percent)
		{
			Internal_ScrollLeft0(mCachedPtr, percent);
		}

		/// <summary>Scrolls the area right by specified percentage (ranging [0, 1]), if possible.</summary>
		public void ScrollRight(float percent)
		{
			Internal_ScrollRight0(mCachedPtr, percent);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUILayout Internal_GetLayout(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollUp(IntPtr thisPtr, ref TUnitValue<int,PhysicalPixel> pixels);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollDown(IntPtr thisPtr, ref TUnitValue<int,PhysicalPixel> pixels);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollLeft(IntPtr thisPtr, ref TUnitValue<int,PhysicalPixel> pixels);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollRight(IntPtr thisPtr, ref TUnitValue<int,PhysicalPixel> pixels);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollUp0(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollDown0(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollLeft0(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollRight0(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollToVertical(IntPtr thisPtr, float pct);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ScrollToHorizontal(IntPtr thisPtr, float pct);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetVerticalScroll(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHorizontalScroll(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetContentBounds(IntPtr thisPtr, out TArea2<TUnitValue<int,PhysicalPixel>,TUnitValue<int,PhysicalPixel>> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableCulling(IntPtr thisPtr, bool enable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetScrollBarSize(IntPtr thisPtr, out TUnitValue<int,LogicalPixel> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(GUIScrollArea managedInstance, ref GUIScrollAreaContent contents, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(GUIScrollArea managedInstance, ref GUIScrollAreaContent contents, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create1(GUIScrollArea managedInstance, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create2(GUIScrollArea managedInstance, params GUIOption[] options);
	}

	/** @} */
}
