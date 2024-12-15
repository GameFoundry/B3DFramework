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
	/// Represents a GUI element that can be interacted with. All interactable elements are also renderable (i.e. have a 
	/// visual component).
	/// </summary>
	[ShowInInspector]
	public partial class GUIInteractable : GUIRenderable
	{
		private GUIInteractable(bool __dummy0) { }
		protected GUIInteractable() { }

		/// <summary>A set of flags controlling various aspects of the GUIElement. See GUIElementOptions.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public GUIElementOption OptionFlags
		{
			get { return Internal_GetOptionFlags(mCachedPtr); }
			set { Internal_SetOptionFlags(mCachedPtr, value); }
		}

		/// <summary>
		/// Assigns a new context menu that will be opened when the element is right clicked. Null is allowed in case no context 
		/// menu is wanted.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ContextMenu ContextMenu
		{
			set { Internal_SetContextMenu(mCachedPtr, value); }
		}

		/// <summary>Triggered when the element gains focus.</summary>
		public event Action OnFocusGained;

		/// <summary>Triggered when the element loses focus.</summary>
		public event Action OnFocusLost;

		/// <summary>Change the GUI element focus state.</summary>
		/// <param name="enabled">Give the element focus or take it away.</param>
		/// <param name="clear">
		/// If true the focus will be cleared from any elements currently in focus. Otherwise the element will just be appended 
		/// to the in-focus list (if enabling focus).
		/// </param>
		public void SetFocus(bool enabled, bool clear = false)
		{
			Internal_SetFocus(mCachedPtr, enabled, clear);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFocus(IntPtr thisPtr, bool enabled, bool clear);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptionFlags(IntPtr thisPtr, GUIElementOption options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern GUIElementOption Internal_GetOptionFlags(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetContextMenu(IntPtr thisPtr, ContextMenu menu);
		private void Internal_OnFocusGained()
		{
			OnFocusGained?.Invoke();
		}
		private void Internal_OnFocusLost()
		{
			OnFocusLost?.Invoke();
		}
	}

	/** @} */
}
