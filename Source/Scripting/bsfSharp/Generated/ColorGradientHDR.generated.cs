//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	/// <summary>
	/// Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of keys 
	/// that get interpolated between. Capable of representing HDR colors, unlike the normal ColorGradient.
	/// </summary>
	[ShowInInspector]
	public partial class ColorGradientHDR : ScriptObject
	{
		private ColorGradientHDR(bool __dummy0) { }

		public ColorGradientHDR()
		{
			Internal_ColorGradientHDR(this);
		}

		public ColorGradientHDR(Color color)
		{
			Internal_ColorGradientHDR0(this, ref color);
		}

		public ColorGradientHDR(ColorGradientKey[] keys)
		{
			Internal_ColorGradientHDR1(this, keys);
		}

		/// <summary>Returns the number of color keys in the gradient.</summary>
		[NativeWrapper]
		public int NumKeys
		{
			get { return Internal_getNumKeys(mCachedPtr); }
		}

		/// <summary>
		/// Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1].
		/// </summary>
		public void SetKeys(ColorGradientKey[] keys, float duration = 1f)
		{
			Internal_setKeys(mCachedPtr, keys, duration);
		}

		/// <summary>
		/// Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1].
		/// </summary>
		public ColorGradientKey[] GetKeys()
		{
			return Internal_getKeys(mCachedPtr);
		}

		/// <summary>Returns the color key at the specified index. If out of range an empty key is returned.</summary>
		public ColorGradientKey GetKey(int idx)
		{
			ColorGradientKey temp;
			Internal_getKey(mCachedPtr, idx, out temp);
			return temp;
		}

		/// <summary>Specify a &quot;gradient&quot; that represents a single color value.</summary>
		public void SetConstant(Color color)
		{
			Internal_setConstant(mCachedPtr, ref color);
		}

		/// <summary>Evaluates a color at the specified <paramref name="t"/>.</summary>
		public Color Evaluate(float t)
		{
			Color temp;
			Internal_evaluate(mCachedPtr, t, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR(ColorGradientHDR managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR0(ColorGradientHDR managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR1(ColorGradientHDR managedInstance, ColorGradientKey[] keys);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setKeys(IntPtr thisPtr, ColorGradientKey[] keys, float duration);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientKey[] Internal_getKeys(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getNumKeys(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getKey(IntPtr thisPtr, int idx, out ColorGradientKey __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setConstant(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float t, out Color __output);
	}

	/** @} */
}
