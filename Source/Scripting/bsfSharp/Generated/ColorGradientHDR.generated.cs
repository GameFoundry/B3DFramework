//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
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
			get { return Internal_GetNumKeys(mCachedPtr); }
		}

		/// <summary>
		/// Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1].
		/// </summary>
		public void SetKeys(ColorGradientKey[] keys, float duration = 1f)
		{
			Internal_SetKeys(mCachedPtr, keys, duration);
		}

		public ColorGradientKey[] GetKeys()
		{
			return Internal_GetKeys(mCachedPtr);
		}

		/// <summary>Returns the color key at the specified index. If out of range an empty key is returned.</summary>
		public ColorGradientKey GetKey(int index)
		{
			ColorGradientKey temp;
			Internal_GetKey(mCachedPtr, index, out temp);
			return temp;
		}

		/// <summary>Specify a &quot;gradient&quot; that represents a single color value.</summary>
		public void SetConstant(Color color)
		{
			Internal_SetConstant(mCachedPtr, ref color);
		}

		public Color Evaluate(float t)
		{
			Color temp;
			Internal_Evaluate(mCachedPtr, t, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR(ColorGradientHDR managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR0(ColorGradientHDR managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradientHDR1(ColorGradientHDR managedInstance, ColorGradientKey[] keys);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetKeys(IntPtr thisPtr, ColorGradientKey[] keys, float duration);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientKey[] Internal_GetKeys(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_GetNumKeys(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetKey(IntPtr thisPtr, int index, out ColorGradientKey __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetConstant(IntPtr thisPtr, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Evaluate(IntPtr thisPtr, float t, out Color __output);
	}

	/** @} */
}
