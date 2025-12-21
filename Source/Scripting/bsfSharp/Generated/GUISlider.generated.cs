//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/// <summary>A slider with a draggable handle that can be vertical or horizontal.</summary>
	[ShowInInspector]
	public partial class GUISlider : GUIInteractable
	{
		private GUISlider(bool __dummy0) { }
		protected GUISlider() { }

		/// <summary>Current position of the slider handle, in percent ranging [0.0f, 1.0f].</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HandlePositionInPercent
		{
			get { return Internal_GetHandlePositionInPercent(mCachedPtr); }
			set { Internal_SetHandlePositionInPercent(mCachedPtr, value); }
		}

		/// <summary>
		/// Current position of the slider handle, scaled within the current minimum and maximum range, rounded up to nearest 
		/// step increment. If no range is provided, the range is [0, 1].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HandlePositionInRange
		{
			get { return Internal_GetHandlePositionInRange(mCachedPtr); }
			set { Internal_SetHandlePositionInRange(mCachedPtr, value); }
		}

		/// <summary>Returns the minimum value of the slider</summary>
		[NativeWrapper]
		public float RangeMinimum
		{
			get { return Internal_GetRangeMinimum(mCachedPtr); }
		}

		/// <summary>Returns the maximum value of the slider</summary>
		[NativeWrapper]
		public float RangeMaximum
		{
			get { return Internal_GetRangeMaximum(mCachedPtr); }
		}

		/// <summary>
		/// Step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no step.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Step
		{
			get { return Internal_GetStep(mCachedPtr); }
			set { Internal_SetStep(mCachedPtr, value); }
		}

		/// <summary>Triggered when the user changes the value of the slider.</summary>
		public event Action<float> OnChanged;

		/// <summary>
		/// Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don&apos;t 
		/// require clamping.
		/// </summary>
		public void SetRange(float minimum, float maximum)
		{
			Internal_SetRange(mCachedPtr, minimum, maximum);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHandlePositionInPercent(IntPtr thisPtr, float percent);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHandlePositionInPercent(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHandlePositionInRange(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHandlePositionInRange(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRange(IntPtr thisPtr, float minimum, float maximum);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRangeMinimum(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRangeMaximum(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetStep(IntPtr thisPtr, float step);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetStep(IntPtr thisPtr);
		private void Internal_OnChanged(float p0)
		{
			OnChanged?.Invoke(p0);
		}
	}

	/** @} */
}
