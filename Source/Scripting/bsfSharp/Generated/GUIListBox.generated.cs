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

	/// <summary>List box GUI element which when active opens a drop down selection with provided elements.</summary>
	[ShowInInspector]
	public partial class GUIListBox : GUIClickable
	{
		private GUIListBox(bool __dummy0) { }
		protected GUIListBox() { }

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIListBox(GUIListBoxContent contents, string styleClass, params GUIOption[] options)
		{
			Internal_Create(this, ref contents, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="contents">Structure describing the contents of the GUI element to create.</param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIListBox(GUIListBoxContent contents, params GUIOption[] options)
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
		public GUIListBox(string styleClass, params GUIOption[] options)
		{
			Internal_Create1(this, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIListBox(params GUIOption[] options)
		{
			Internal_Create2(this, options);
		}

		/// <summary>Checks whether the listbox supports multiple selected elements at once.</summary>
		[NativeWrapper]
		public bool IsMultiselect
		{
			get { return Internal_IsMultiselect(mCachedPtr); }
		}

		/// <summary>
		/// Sets states for all list box elements. Only valid for multi-select list boxes. Number of states must match number of 
		/// list box elements.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool[] ElementStates
		{
			get { return Internal_GetElementStates(mCachedPtr); }
			set { Internal_SetElementStates(mCachedPtr, value); }
		}

		/// <summary>
		/// Triggered whenever user selects or deselects an element in the list box. Returned index maps to the element in the 
		/// elements array that the list box was initialized with.
		/// </summary>
		public event Action<int, bool> OnSelectionToggled;

		/// <summary>Changes the list box elements.</summary>
		public void SetElements(LocString[] elements)
		{
			Internal_SetElements(mCachedPtr, elements);
		}

		/// <summary>Makes the element with the specified index selected.</summary>
		public void SelectElement(int index)
		{
			Internal_SelectElement(mCachedPtr, index);
		}

		/// <summary>Deselect element the element with the specified index. Only relevant for multi-select list boxes.</summary>
		public void DeselectElement(int index)
		{
			Internal_DeselectElement(mCachedPtr, index);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsMultiselect(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetElements(IntPtr thisPtr, LocString[] elements);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SelectElement(IntPtr thisPtr, int index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_DeselectElement(IntPtr thisPtr, int index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool[] Internal_GetElementStates(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetElementStates(IntPtr thisPtr, bool[] states);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(GUIListBox managedInstance, ref GUIListBoxContent contents, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(GUIListBox managedInstance, ref GUIListBoxContent contents, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create1(GUIListBox managedInstance, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create2(GUIListBox managedInstance, params GUIOption[] options);
		private void Internal_OnSelectionToggled(int p0, bool p1)
		{
			OnSelectionToggled?.Invoke(p0, p1);
		}
	}

	/** @} */
}
