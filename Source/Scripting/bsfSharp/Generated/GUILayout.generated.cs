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
	/// Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles 
	/// and layout options.
	/// </summary>
	[ShowInInspector]
	public partial class GUILayout : GUIElement
	{
		private GUILayout(bool __dummy0) { }
		protected GUILayout() { }

		/// <summary>Returns the number of children in the layout.</summary>
		[NativeWrapper]
		public int ChildCount
		{
			get { return Internal_GetChildCount(mCachedPtr); }
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

		/// <summary>Adds a new element to the layout after all existing elements.</summary>
		public void AddElement(GUIElement element)
		{
			Internal_AddElement(mCachedPtr, element);
		}

		/// <summary>Removes the specified element from the layout.</summary>
		public void RemoveElement(GUIElement element)
		{
			Internal_RemoveElement(mCachedPtr, element);
		}

		/// <summary>Removes a child element at the specified index.</summary>
		public void RemoveElementAt(int index)
		{
			Internal_RemoveElementAt(mCachedPtr, index);
		}

		/// <summary>Inserts a GUI element before the element at the specified index.</summary>
		public void InsertElement(int index, GUIElement element)
		{
			Internal_InsertElement(mCachedPtr, index, element);
		}

		/// <summary>Returns a child element at the specified index, or null if the index is not valid.</summary>
		public GUIElement GetChild(int index)
		{
			return Internal_GetChild(mCachedPtr, index);
		}

		/// <summary>Removes all child elements and destroys them.</summary>
		public void Clear()
		{
			Internal_Clear(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddElement(IntPtr thisPtr, GUIElement element);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveElement(IntPtr thisPtr, GUIElement element);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveElementAt(IntPtr thisPtr, int index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_InsertElement(IntPtr thisPtr, int index, GUIElement element);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetChildCount(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIElement Internal_GetChild(IntPtr thisPtr, int index);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Clear(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetEnableCulling(IntPtr thisPtr, bool enable);
	}

	/** @} */
}
