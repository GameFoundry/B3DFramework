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
	/// Represents a GUI element that can be rendered (i.e. has a visual representation). Renderable element can have a 
	/// particular style, and provides one or multiple render elements to be drawn.
	/// </summary>
	[ShowInInspector]
	public partial class GUIRenderable : GUIElement
	{
		private GUIRenderable(bool __dummy0) { }
		protected GUIRenderable() { }

		/// <summary>Sets new style class to be used by the element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public string StyleSheetClass
		{
			get { return Internal_GetStyleSheetClass(mCachedPtr); }
			set { Internal_SetStyleSheetClass(mCachedPtr, value); }
		}

		/// <summary>Sets the tint of the GUI element.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Color Tint
		{
			get
			{
				Color temp;
				Internal_GetTint(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetTint(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern string Internal_GetStyleSheetClass(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetStyleSheetClass(IntPtr thisPtr, string styleClass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetTint(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetTint(IntPtr thisPtr, out Color __output);
	}

	/** @} */
}
