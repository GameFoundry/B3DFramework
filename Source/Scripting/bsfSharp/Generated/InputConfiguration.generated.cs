//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Input
	 *  @{
	 */

	/// <summary>Contains virtual &lt;-&gt; physical key mappings.</summary>
	[ShowInInspector]
	public partial class InputConfiguration : ScriptObject
	{
		private InputConfiguration(bool __dummy0) { }

		public InputConfiguration()
		{
			Internal_InputConfiguration(this);
		}

		/// <summary>
		/// Repeat interval for held virtual buttons. Buttons will be continously triggered in interval increments as long as 
		/// they button is being held.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong RepeatInterval
		{
			get { return Internal_GetRepeatInterval(mCachedPtr); }
			set { Internal_SetRepeatInterval(mCachedPtr, value); }
		}

		/// <summary>Registers a new virtual button.</summary>
		/// <param name="name">Unique name used to access the virtual button.</param>
		/// <param name="buttonCode">Physical button the virtual button is triggered by.</param>
		/// <param name="modifiers">
		/// Modifiers required to be pressed with the physical button to trigger the virtual button.
		/// </param>
		/// <param name="repeatable">
		/// If true, the virtual button events will be sent continually while the physical button is being held.
		/// </param>
		public void RegisterButton(string name, ButtonCode buttonCode, ButtonModifier modifiers = ButtonModifier.None, bool repeatable = false)
		{
			Internal_RegisterButton(mCachedPtr, name, buttonCode, modifiers, repeatable);
		}

		/// <summary>
		/// Unregisters a virtual button with the specified name. Events will no longer be generated for that button.
		/// </summary>
		public void UnregisterButton(string name)
		{
			Internal_UnregisterButton(mCachedPtr, name);
		}

		/// <summary>Registers a new virtual axis.</summary>
		/// <param name="name">Unique name used to access the axis.</param>
		/// <param name="createInformation">Descriptor structure containing virtual axis creation parameters.</param>
		public void RegisterAxis(string name, VirtualAxisCreateInformation createInformation)
		{
			Internal_RegisterAxis(mCachedPtr, name, ref createInformation);
		}

		/// <summary>
		/// Unregisters a virtual axis with the specified name. You will no longer be able to retrieve valid values for that axis.
		/// </summary>
		public void UnregisterAxis(string name)
		{
			Internal_UnregisterAxis(mCachedPtr, name);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_InputConfiguration(InputConfiguration managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RegisterButton(IntPtr thisPtr, string name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnregisterButton(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RegisterAxis(IntPtr thisPtr, string name, ref VirtualAxisCreateInformation createInformation);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_UnregisterAxis(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRepeatInterval(IntPtr thisPtr, ulong milliseconds);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_GetRepeatInterval(IntPtr thisPtr);
	}

	/** @} */
}
