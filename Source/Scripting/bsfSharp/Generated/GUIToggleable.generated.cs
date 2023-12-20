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

	/// <summary>GUI element representing a toggleable button.</summary>
	[ShowInInspector]
	public partial class GUIToggleable : GUIClickable
	{
		private GUIToggleable(bool __dummy0) { }
		protected GUIToggleable() { }

		/// <summary>Checks or unchecks the toggle.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool IsToggled
		{
			get { return Internal_IsToggled(mCachedPtr); }
			set { Internal_SetIsToggled(mCachedPtr, value); }
		}

		/// <summary>Triggered whenever the button is toggled on or off.</summary>
		public event Action<bool> OnToggled;

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetIsToggled(IntPtr thisPtr, bool isToggled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsToggled(IntPtr thisPtr);
		private void Internal_OnToggled(bool p0)
		{
			OnToggled?.Invoke(p0);
		}
	}

	/** @} */
}
