//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
			get { return Internal_GetFilmicCurveShoulderStrength(mCachedPtr); }
			set { Internal_SetFilmicCurveShoulderStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of the 
		/// image.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearStrength
		{
			get { return Internal_GetFilmicCurveLinearStrength(mCachedPtr); }
			set { Internal_SetFilmicCurveLinearStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the linear (middle) section of the filmic curve used for tonemapping. Mostly affects mid-range areas of the 
		/// image and allows you to control how quickly does the curve climb.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearAngle
		{
			get { return Internal_GetFilmicCurveLinearAngle(mCachedPtr); }
			set { Internal_SetFilmicCurveLinearAngle(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Mostly affects dark areas of 
		/// the image and allows you to reduce under-exposure.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeStrength
		{
			get { return Internal_GetFilmicCurveToeStrength(mCachedPtr); }
			set { Internal_SetFilmicCurveToeStrength(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve. used for tonemapping. Affects low-range.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeNumerator
		{
			get { return Internal_GetFilmicCurveToeNumerator(mCachedPtr); }
			set { Internal_SetFilmicCurveToeNumerator(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls the toe (lower non-linear) section of the filmic curve used for tonemapping. Affects low-range.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveToeDenominator
		{
			get { return Internal_GetFilmicCurveToeDenominator(mCachedPtr); }
			set { Internal_SetFilmicCurveToeDenominator(mCachedPtr, value); }
		}

		/// <summary>Controls the white point of the filmic curve used for tonemapping. Affects the entire curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float FilmicCurveLinearWhitePoint
		{
			get { return Internal_GetFilmicCurveLinearWhitePoint(mCachedPtr); }
			set { Internal_SetFilmicCurveLinearWhitePoint(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TonemappingSettings(TonemappingSettings managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveShoulderStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveShoulderStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveLinearStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveLinearStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveLinearAngle(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveLinearAngle(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveToeStrength(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveToeStrength(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveToeNumerator(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveToeNumerator(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveToeDenominator(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveToeDenominator(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetFilmicCurveLinearWhitePoint(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetFilmicCurveLinearWhitePoint(IntPtr thisPtr, float value);
	}

	/** @} */
}
