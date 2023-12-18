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

	/// <summary>Base class for a clickable GUI button element.</summary>
	[ShowInInspector]
	public partial class GUIClickable : GUIElement
	{
		private GUIClickable(bool __dummy0) { }
		protected GUIClickable() { }

		/// <summary>Triggered when button is clicked.</summary>
		public event Action OnClick;

		/// <summary>Triggered when pointer hovers over the button.</summary>
		public event Action OnHover;

		/// <summary>Triggered when pointer that was previously hovering leaves the button.</summary>
		public event Action OnOut;

		/// <summary>Triggered when button is clicked twice in rapid succession.</summary>
		public event Action OnDoubleClick;

		/// <summary>Change content displayed by the button.</summary>
		public void SetContent(GUIContent content)
		{
			Internal_SetContent(mCachedPtr, ref content);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetContent(IntPtr thisPtr, ref GUIContent content);
		private void Internal_OnClick()
		{
			OnClick?.Invoke();
		}
		private void Internal_OnHover()
		{
			OnHover?.Invoke();
		}
		private void Internal_OnOut()
		{
			OnOut?.Invoke();
		}
		private void Internal_OnDoubleClick()
		{
			OnDoubleClick?.Invoke();
		}
	}

	/** @} */
}
