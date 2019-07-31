//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>
	/// Specifies a color as a distribution, which can include a constant color, random color range or a color gradient.
	/// </summary>
	[ShowInInspector]
	public partial class ColorDistribution : ScriptObject
	{
		private ColorDistribution(bool __dummy0) { }

		/// <summary>Creates a new empty distribution.</summary>
		public ColorDistribution()
		{
			Internal_TColorDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant color.</summary>
		public ColorDistribution(Color color)
		{
			Internal_TColorDistribution0(this, ref color);
		}

		/// <summary>Creates a new distribution that returns a random color in the specified range.</summary>
		public ColorDistribution(Color minColor, Color maxColor)
		{
			Internal_TColorDistribution1(this, ref minColor, ref maxColor);
		}

		/// <summary>Creates a new distribution that evaluates a color gradient.</summary>
		public ColorDistribution(ColorGradient gradient)
		{
			Internal_TColorDistribution2(this, gradient);
		}

		/// <summary>Creates a new distribution that returns a random color in a range determined by two gradients.</summary>
		public ColorDistribution(ColorGradient minGradient, ColorGradient maxGradient)
		{
			Internal_TColorDistribution3(this, minGradient, maxGradient);
		}

		/// <summary>Returns the type of the represented distribution.</summary>
		[NativeWrapper]
		public PropertyDistributionType DistributionType
		{
			get { return Internal_getType(mCachedPtr); }
		}

		/// <summary>
		/// Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if the 
		/// distribution is represented by a gradient.
		/// </summary>
		public Color GetMinConstant()
		{
			Color temp;
			Internal_getMinConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the maximum value of a constant range. Only defined if the distribution represents a non-gradient range.
		/// </summary>
		public Color GetMaxConstant()
		{
			Color temp;
			Internal_getMaxConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the gradient representing the distribution, or the first gradient representing a gradient range. Undefined if 
		/// the distribution is represented by a constant or a non-gradient range.
		/// </summary>
		public ColorGradient GetMinGradient()
		{
			return Internal_getMinGradient(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second gradient of a gradient range. Only defined if the distribution represents a 
		/// gradient range.
		/// </summary>
		public ColorGradient GetMaxGradient()
		{
			return Internal_getMaxGradient(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution(ColorDistribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution0(ColorDistribution managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution1(ColorDistribution managedInstance, ref Color minColor, ref Color maxColor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution2(ColorDistribution managedInstance, ColorGradient gradient);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution3(ColorDistribution managedInstance, ColorGradient minGradient, ColorGradient maxGradient);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PropertyDistributionType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMinConstant(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMaxConstant(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradient Internal_getMinGradient(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradient Internal_getMaxGradient(IntPtr thisPtr);
	}

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>
	/// Specifies a color as a distribution, which can include a constant color, random color range or a color gradient.
	/// </summary>
	[ShowInInspector]
	public partial class ColorHDRDistribution : ScriptObject
	{
		private ColorHDRDistribution(bool __dummy0) { }

		/// <summary>Creates a new empty distribution.</summary>
		public ColorHDRDistribution()
		{
			Internal_TColorDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant color.</summary>
		public ColorHDRDistribution(Color color)
		{
			Internal_TColorDistribution0(this, ref color);
		}

		/// <summary>Creates a new distribution that returns a random color in the specified range.</summary>
		public ColorHDRDistribution(Color minColor, Color maxColor)
		{
			Internal_TColorDistribution1(this, ref minColor, ref maxColor);
		}

		/// <summary>Creates a new distribution that evaluates a color gradient.</summary>
		public ColorHDRDistribution(ColorGradientHDR gradient)
		{
			Internal_TColorDistribution2(this, gradient);
		}

		/// <summary>Creates a new distribution that returns a random color in a range determined by two gradients.</summary>
		public ColorHDRDistribution(ColorGradientHDR minGradient, ColorGradientHDR maxGradient)
		{
			Internal_TColorDistribution3(this, minGradient, maxGradient);
		}

		/// <summary>Returns the type of the represented distribution.</summary>
		[NativeWrapper]
		public PropertyDistributionType DistributionType
		{
			get { return Internal_getType(mCachedPtr); }
		}

		/// <summary>
		/// Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if the 
		/// distribution is represented by a gradient.
		/// </summary>
		public Color GetMinConstant()
		{
			Color temp;
			Internal_getMinConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the maximum value of a constant range. Only defined if the distribution represents a non-gradient range.
		/// </summary>
		public Color GetMaxConstant()
		{
			Color temp;
			Internal_getMaxConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the gradient representing the distribution, or the first gradient representing a gradient range. Undefined if 
		/// the distribution is represented by a constant or a non-gradient range.
		/// </summary>
		public ColorGradientHDR GetMinGradient()
		{
			return Internal_getMinGradient(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second gradient of a gradient range. Only defined if the distribution represents a 
		/// gradient range.
		/// </summary>
		public ColorGradientHDR GetMaxGradient()
		{
			return Internal_getMaxGradient(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution(ColorHDRDistribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution0(ColorHDRDistribution managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution1(ColorHDRDistribution managedInstance, ref Color minColor, ref Color maxColor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution2(ColorHDRDistribution managedInstance, ColorGradientHDR gradient);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TColorDistribution3(ColorHDRDistribution managedInstance, ColorGradientHDR minGradient, ColorGradientHDR maxGradient);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PropertyDistributionType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMinConstant(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMaxConstant(IntPtr thisPtr, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientHDR Internal_getMinGradient(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ColorGradientHDR Internal_getMaxGradient(IntPtr thisPtr);
	}

	/** @} */
}
