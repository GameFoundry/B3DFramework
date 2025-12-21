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
	/// that get interpolated between. Stores colors as 32-bit integers, and is therefor unable to represent a color range 
	/// outside of [0, 1] - see ColorGradientHDR for an alternative.
	/// </summary>
	[ShowInInspector]
	public partial class ColorGradient : ScriptObject
	{
		private ColorGradient(bool __dummy0) { }

		public ColorGradient()
		{
			Internal_ColorGradient(this);
		}

		public ColorGradient(Color color)
		{
			Internal_ColorGradient0(this, ref color);
		}

		public ColorGradient(ColorGradientKey[] keys)
		{
			Internal_ColorGradient1(this, keys);
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
		private static extern void Internal_ColorGradient(ColorGradient managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradient0(ColorGradient managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorGradient1(ColorGradient managedInstance, ColorGradientKey[] keys);
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
