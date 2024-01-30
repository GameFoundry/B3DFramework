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
	/// GUI element containing a background image and a fill image that is scaled depending on the percentage set by the 
	/// caller.
	/// </summary>
	[ShowInInspector]
	public partial class GUIProgressBar : GUIElement
	{
		private GUIProgressBar(bool __dummy0) { }
		protected GUIProgressBar() { }

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="styleClass">
		/// Style class that will be used for determining GUI element visuals from the current style sheet. If no class is 
		/// provided, default style is determined based on GUI element type.
		/// </param>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIProgressBar(string styleClass, params GUIOption[] options)
		{
			Internal_Create(this, styleClass, options);
		}

		/// <summary>Creates a new GUI element.</summary>
		/// <param name="options">
		/// Additional options that control GUI element size and position. This will override options set in the style sheet.
		/// </param>
		public GUIProgressBar(params GUIOption[] options)
		{
			Internal_Create0(this, options);
		}

		/// <summary>Determines how far is the progress bar filled, in range [0, 1].</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Percent
		{
			get { return Internal_GetPercent(mCachedPtr); }
			set { Internal_SetPercent(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPercent(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetPercent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(GUIProgressBar managedInstance, string styleClass, params GUIOption[] options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(GUIProgressBar managedInstance, params GUIOption[] options);
	}

	/** @} */
}
