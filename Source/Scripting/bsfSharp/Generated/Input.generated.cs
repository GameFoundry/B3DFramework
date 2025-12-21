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
	/// Primary module used for dealing with input. Allows you to receieve and query raw or OS input for 
	/// mouse/keyboard/gamepad.
	/// </summary>
	[ShowInInspector]
	public partial class Input : ScriptObject
	{
		private Input(bool __dummy0) { }
		protected Input() { }

		/// <summary>Returns position of the pointer (for example mouse cursor) relative to the screen.</summary>
		[NativeWrapper]
		public static TVector2<int> PointerPosition
		{
			get
			{
				TVector2<int> temp;
				Internal_GetPointerPosition(out temp);
				return temp;
			}
		}

		/// <summary>Returns difference between pointer position between current and last frame.</summary>
		[NativeWrapper]
		public static TVector2<int> PointerDelta
		{
			get
			{
				TVector2<int> temp;
				Internal_GetPointerDelta(out temp);
				return temp;
			}
		}

		/// <summary>Triggered whenever a button is first pressed.</summary>
		public static event Action<ButtonEvent> OnButtonDown;

		/// <summary>Triggered whenever a button is first released.</summary>
		public static event Action<ButtonEvent> OnButtonUp;

		/// <summary>Triggered whenever user inputs a text character.</summary>
		public static event Action<TextInputEvent> OnCharInput;

		/// <summary>Triggers when some pointing device (mouse cursor, touch) moves.</summary>
		public static event Action<PointerEvent> OnPointerMoved;

		/// <summary>Triggers when some pointing device (mouse cursor, touch) button is pressed.</summary>
		public static event Action<PointerEvent> OnPointerPressed;

		/// <summary>Triggers when some pointing device (mouse cursor, touch) button is released.</summary>
		public static event Action<PointerEvent> OnPointerReleased;

		/// <summary>Triggers when some pointing device (mouse cursor, touch) button is double clicked.</summary>
		public static event Action<PointerEvent> OnPointerDoubleClick;

		/// <summary>
		/// Returns value of the specified input axis. Normally in range [-1.0, 1.0] but can be outside the range for devices 
		/// with unbound axes (for example mouse).
		/// </summary>
		/// <param name="type">Type of axis to query. Usually a type from InputAxis but can be a custom value.</param>
		/// <param name="deviceIndex">Index of the device in case more than one is hooked up (0 - primary).</param>
		public static float GetAxisValue(int type, int deviceIndex = 0)
		{
			return Internal_GetAxisValue(type, deviceIndex);
		}

		/// <summary>Query if the provided button is currently being held (this frame or previous frames).</summary>
		/// <param name="keyCode">Code of the button to query.</param>
		/// <param name="deviceIndex">Device to query the button on (0 - primary).</param>
		public static bool IsButtonHeld(ButtonCode keyCode, int deviceIndex = 0)
		{
			return Internal_IsButtonHeld(keyCode, deviceIndex);
		}

		/// <summary>Query if the provided button is currently being released (only true for one frame).</summary>
		/// <param name="keyCode">Code of the button to query.</param>
		/// <param name="deviceIndex">Device to query the button on (0 - primary).</param>
		public static bool IsButtonUp(ButtonCode keyCode, int deviceIndex = 0)
		{
			return Internal_IsButtonUp(keyCode, deviceIndex);
		}

		/// <summary>Query if the provided button is currently being pressed (only true for one frame).</summary>
		/// <param name="keyCode">Code of the button to query.</param>
		/// <param name="deviceIndex">Device to query the button on (0 - primary).</param>
		public static bool IsButtonDown(ButtonCode keyCode, int deviceIndex = 0)
		{
			return Internal_IsButtonDown(keyCode, deviceIndex);
		}

		/// <summary>Query if the provided pointer button is currently being held (this frame or previous frames).</summary>
		/// <param name="pointerButton">Code of the button to query.</param>
		public static bool IsPointerButtonHeld(PointerEventButton pointerButton)
		{
			return Internal_IsPointerButtonHeld(pointerButton);
		}

		/// <summary>Query if the provided pointer button is currently being released (only true for one frame).</summary>
		/// <param name="pointerButton">Code of the button to query.</param>
		public static bool IsPointerButtonUp(PointerEventButton pointerButton)
		{
			return Internal_IsPointerButtonUp(pointerButton);
		}

		/// <summary>Query if the provided pointer button is currently being pressed (only true for one frame).</summary>
		/// <param name="pointerButton">Code of the button to query.</param>
		public static bool IsPointerButtonDown(PointerEventButton pointerButton)
		{
			return Internal_IsPointerButtonDown(pointerButton);
		}

		/// <summary>Query has the left pointer button has been double-clicked this frame.</summary>
		public static bool IsPointerDoubleClicked()
		{
			return Internal_IsPointerDoubleClicked();
		}

		/// <summary>Enables or disables mouse smoothing. Smoothing makes the changes to mouse axes more gradual.</summary>
		public static void SetMouseSmoothing(bool enabled)
		{
			Internal_SetMouseSmoothing(enabled);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetAxisValue(int type, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonHeld(ButtonCode keyCode, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonUp(ButtonCode keyCode, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsButtonDown(ButtonCode keyCode, int deviceIndex);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetPointerPosition(out TVector2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetPointerDelta(out TVector2<int> __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsPointerButtonHeld(PointerEventButton pointerButton);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsPointerButtonUp(PointerEventButton pointerButton);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsPointerButtonDown(PointerEventButton pointerButton);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_IsPointerDoubleClicked();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMouseSmoothing(bool enabled);
		private static void Internal_OnButtonDown(ref ButtonEvent p0)
		{
			OnButtonDown?.Invoke(p0);
		}
		private static void Internal_OnButtonUp(ref ButtonEvent p0)
		{
			OnButtonUp?.Invoke(p0);
		}
		private static void Internal_OnCharInput(ref TextInputEvent p0)
		{
			OnCharInput?.Invoke(p0);
		}
		private static void Internal_OnPointerMoved(ref PointerEvent p0)
		{
			OnPointerMoved?.Invoke(p0);
		}
		private static void Internal_OnPointerPressed(ref PointerEvent p0)
		{
			OnPointerPressed?.Invoke(p0);
		}
		private static void Internal_OnPointerReleased(ref PointerEvent p0)
		{
			OnPointerReleased?.Invoke(p0);
		}
		private static void Internal_OnPointerDoubleClick(ref PointerEvent p0)
		{
			OnPointerDoubleClick?.Invoke(p0);
		}
	}

	/** @} */
}
