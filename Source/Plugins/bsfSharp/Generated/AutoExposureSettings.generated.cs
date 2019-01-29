//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Settings that control automatic exposure (eye adaptation) post-process.</summary>
	[ShowInInspector]
	public partial class AutoExposureSettings : ScriptObject
	{
		private AutoExposureSettings(bool __dummy0) { }

		public AutoExposureSettings()
		{
			Internal_AutoExposureSettings(this);
		}

		/// <summary>
		/// Determines minimum luminance value in the eye adaptation histogram. The histogram is used for calculating the average 
		/// brightness of the scene. Any luminance value below this value will not be included in the histogram and ignored in 
		/// scene brightness calculations. In log2 units (-8 = 1/256). In the range [-16, 0].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HistogramLog2Min
		{
			get { return Internal_gethistogramLog2Min(mCachedPtr); }
			set { Internal_sethistogramLog2Min(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines maximum luminance value in the eye adaptation histogram. The histogram is used for calculating the average 
		/// brightness of the scene. Any luminance value above this value will not be included in the histogram and ignored in 
		/// scene brightness calculations. In log2 units (4 = 16). In the range [0, 16].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HistogramLog2Max
		{
			get { return Internal_gethistogramLog2Max(mCachedPtr); }
			set { Internal_sethistogramLog2Max(mCachedPtr, value); }
		}

		/// <summary>
		/// Percentage below which to ignore values in the eye adaptation histogram. The histogram is used for calculating the 
		/// average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this value to 
		/// calculate minimal luminance. Luminance values below the minimal luminance will be ignored and not used in scene 
		/// brightness calculations. This allows you to remove outliers on the lower end of the histogram (for example a few very 
		/// dark pixels in an otherwise bright image). In range [0.0f, 1.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HistogramPctLow
		{
			get { return Internal_gethistogramPctLow(mCachedPtr); }
			set { Internal_sethistogramPctLow(mCachedPtr, value); }
		}

		/// <summary>
		/// Percentage above which to ignore values in the eye adaptation histogram. The histogram is used for calculating the 
		/// average brightness of the scene. Total luminance in the histogram will be summed up and multiplied by this value to 
		/// calculate maximum luminance. Luminance values above the maximum luminance will be ignored and not used in scene 
		/// brightness calculations. This allows you to remove outliers on the high end of the histogram (for  example a few very 
		/// bright pixels). In range [0.0f, 1.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HistogramPctHigh
		{
			get { return Internal_gethistogramPctHigh(mCachedPtr); }
			set { Internal_sethistogramPctHigh(mCachedPtr, value); }
		}

		/// <summary>
		/// Clamps the minimum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure is 
		/// never too high (for example when in a very dark room you probably do not want the exposure to be so high that 
		/// everything is still visible). In range [0.0f, 10.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MinEyeAdaptation
		{
			get { return Internal_getminEyeAdaptation(mCachedPtr); }
			set { Internal_setminEyeAdaptation(mCachedPtr, value); }
		}

		/// <summary>
		/// Clamps the maximum eye adaptation scale to this value. This allows you to limit eye adaptation so that exposure is 
		/// never too low (for example when looking at a very bright light source you probably don't want the exposure to be so 
		/// low that the rest of the scene is all white (overexposed). In range [0.0f, 10.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float MaxEyeAdaptation
		{
			get { return Internal_getmaxEyeAdaptation(mCachedPtr); }
			set { Internal_setmaxEyeAdaptation(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how quickly does the eye adaptation adjust to larger values. This affects how quickly does the automatic 
		/// exposure changes when the scene brightness increases. In range [0.01f, 20.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float EyeAdaptationSpeedUp
		{
			get { return Internal_geteyeAdaptationSpeedUp(mCachedPtr); }
			set { Internal_seteyeAdaptationSpeedUp(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how quickly does the eye adaptation adjust to smaller values. This affects how quickly does the automatic 
		/// exposure changes when the scene brightness decreases. In range [0.01f, 20.0f].
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float EyeAdaptationSpeedDown
		{
			get { return Internal_geteyeAdaptationSpeedDown(mCachedPtr); }
			set { Internal_seteyeAdaptationSpeedDown(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AutoExposureSettings(AutoExposureSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethistogramLog2Min(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethistogramLog2Min(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethistogramLog2Max(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethistogramLog2Max(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethistogramPctLow(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethistogramPctLow(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_gethistogramPctHigh(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_sethistogramPctHigh(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getminEyeAdaptation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setminEyeAdaptation(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getmaxEyeAdaptation(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setmaxEyeAdaptation(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_geteyeAdaptationSpeedUp(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_seteyeAdaptationSpeedUp(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_geteyeAdaptationSpeedDown(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_seteyeAdaptationSpeedDown(IntPtr thisPtr, float value);
	}

	/** @} */
}
