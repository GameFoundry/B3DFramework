//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup Input
	 *  @{
	 */

	/// <summary>
	/// Handles virtual input that allows you to receive virtual input events that hide the actual physical input, allowing 
	/// you to easily change the input keys while being transparent to the external code.
	/// </summary>
	[ShowInInspector]
	public partial class VirtualInput : ScriptObject
	{
		private VirtualInput(bool __dummy0) { }
		protected VirtualInput() { }

		/// <summary>Input configuration that determines how physical keys map to virtual buttons.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static InputConfiguration Configuration
		{
			get { return Internal_GetConfiguration(); }
			set { Internal_SetConfiguration(value); }
		}

		/// <summary>Triggered when a virtual button is pressed.</summary>
		public static event Action<VirtualButton, int> OnButtonDown;

		/// <summary>Triggered when a virtual button is released.</summary>
		public static event Action<VirtualButton, int> OnButtonUp;

		/// <summary>Triggered every frame when a virtual button is being held down.</summary>
		public static event Action<VirtualButton, int> OnButtonHeld;

		/// <summary>Creates a new virtual button associated with the name, or returns an existing button if it exists.</summary>
		public static VirtualButton GetOrCreateVirtualButton(string name)
		{
			VirtualButton temp;
			Internal_GetOrCreateVirtualButton(name, out temp);
			return temp;
		}

		/// <summary>Creates a new virtual axis associated with the name, or returns an existing axis if it exists.</summary>
		public static VirtualAxis GetOrCreateVirtualAxis(string name)
		{
			VirtualAxis temp;
			Internal_GetOrCreateVirtualAxis(name, out temp);
			return temp;
		}

		/// <summary>Check is the virtual button just getting pressed. This state is only active for one frame.</summary>
		/// <param name="button">Virtual button identifier.</param>
		/// <param name="deviceIndex">Optional device index in case multiple input devices are available.</param>
		public static bool IsButtonDown(VirtualButton button, int deviceIndex = 0)
		{
			return Internal_IsButtonDown(ref button, deviceIndex);
		}

		/// <summary>Check is the virtual button just getting released. This state is only active for one frame.</summary>
		/// <param name="button">Virtual button identifier.</param>
		/// <param name="deviceIndex">Optional device index in case multiple input devices are available.</param>
		public static bool IsButtonUp(VirtualButton button, int deviceIndex = 0)
		{
			return Internal_IsButtonUp(ref button, deviceIndex);
		}

		/// <summary>
		/// Check is the virtual button is being held. This state is active as long as the button is being held down, possibly 
		/// for multiple frames.
		/// </summary>
		/// <param name="button">Virtual button identifier.</param>
		/// <param name="deviceIndex">Optional device index in case multiple input devices are available.</param>
		public static bool IsButtonHeld(VirtualButton button, int deviceIndex = 0)
		{
			return Internal_IsButtonHeld(ref button, deviceIndex);
		}

		/// <summary>
		/// Returns normalized value for the specified input axis. Returned value will usually be in [-1.0, 1.0] range, but can 
		/// be outside the range for devices with unbound axes (for example mouse).
		/// </summary>
		/// <param name="axis">Virtual axis identifier.</param>
		/// <param name="deviceIndex">Optional device index in case multiple input devices are available.</param>
		public static float GetAxisValue(VirtualAxis axis, int deviceIndex = 0)
		{
			return Internal_GetAxisValue(ref axis, deviceIndex);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetConfiguration(InputConfiguration input);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern InputConfiguration Internal_GetConfiguration();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOrCreateVirtualButton(string name, out VirtualButton __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOrCreateVirtualAxis(string name, out VirtualAxis __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonDown(ref VirtualButton button, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonUp(ref VirtualButton button, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonHeld(ref VirtualButton button, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAxisValue(ref VirtualAxis axis, int deviceIndex);
		private static void Internal_OnButtonDown(ref VirtualButton p0, int p1)
		{
			OnButtonDown?.Invoke(p0, p1);
		}
		private static void Internal_OnButtonUp(ref VirtualButton p0, int p1)
		{
			OnButtonUp?.Invoke(p0, p1);
		}
		private static void Internal_OnButtonHeld(ref VirtualButton p0, int p1)
		{
			OnButtonHeld?.Invoke(p0, p1);
		}
	}

	/** @} */
}
