//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Rendering
	 *  @{
	 */

	/// <summary>Settings that control tonemap post-process.</summary>
	[ShowInInspector]
	public partial class TonemappingSettings : ScriptObject
	{
		private TonemappingSettings(bool __dummy0) { }

		public TonemappingSettings()
		{
			Internal_TonemappingSettings(this);
		}

		/// <summary>
		/// Controls the shoulder (upper non-linear) section of the filmic curve used for tonemapping. Mostly affects bright 
		/// areas of the image and allows you to reduce over-exposure.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveShoulderStrength
		{
			get { return Internal_getfilmicCurveShoulderStrength(mCachedPtr); }
			set { Internal_setfilmicCurveShoulderStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of the 
		/// image.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearStrength
		{
			get { return Internal_getfilmicCurveLinearStrength(mCachedPtr); }
			set { Internal_setfilmicCurveLinearStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of the 
		/// image and allows you to control how quickly does the curve climb.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearAngle
		{
			get { return Internal_getfilmicCurveLinearAngle(mCachedPtr); }
			set { Internal_setfilmicCurveLinearAngle(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Mostly affects dark areas of 
		/// the image and allows you to reduce under-exposure.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeStrength
		{
			get { return Internal_getfilmicCurveToeStrength(mCachedPtr); }
			set { Internal_setfilmicCurveToeStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve. used for tonemapping. Affects low-range.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeNumerator
		{
			get { return Internal_getfilmicCurveToeNumerator(mCachedPtr); }
			set { Internal_setfilmicCurveToeNumerator(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Affects low-range.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeDenominator
		{
			get { return Internal_getfilmicCurveToeDenominator(mCachedPtr); }
			set { Internal_setfilmicCurveToeDenominator(mCachedPtr, value); }
		}

		/// <summary>Controls the white point of the filmic curve used for tonemapping. Affects the entire curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearWhitePoint
		{
			get { return Internal_getfilmicCurveLinearWhitePoint(mCachedPtr); }
			set { Internal_setfilmicCurveLinearWhitePoint(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TonemappingSettings(TonemappingSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveShoulderStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveShoulderStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveLinearStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveLinearStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveLinearAngle(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveLinearAngle(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveToeStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveToeStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveToeNumerator(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveToeNumerator(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveToeDenominator(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveToeDenominator(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getfilmicCurveLinearWhitePoint(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setfilmicCurveLinearWhitePoint(IntPtr thisPtr, float value);
	}

	/** @} */
}
