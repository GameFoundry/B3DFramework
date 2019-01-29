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
			Internal_ColorDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant color.</summary>
		public ColorDistribution(Color color)
		{
			Internal_ColorDistribution0(this, ref color);
		}

		/// <summary>Creates a new distribution that returns a random color in the specified range.</summary>
		public ColorDistribution(Color minColor, Color maxColor)
		{
			Internal_ColorDistribution1(this, ref minColor, ref maxColor);
		}

		/// <summary>Creates a new distribution that evaluates a color gradient.</summary>
		public ColorDistribution(ColorGradient gradient)
		{
			Internal_ColorDistribution2(this, gradient);
		}

		/// <summary>Creates a new distribution that returns a random color in a range determined by two gradients.</summary>
		public ColorDistribution(ColorGradient minGradient, ColorGradient maxGradient)
		{
			Internal_ColorDistribution3(this, minGradient, maxGradient);
		}

		/// <summary>Returns the type of the represented distribution.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PropertyDistributionType DistributionType
		{
			get { return Internal_getType(mCachedPtr); }
		}

		/// <summary>
		/// Returns the constant value of the distribution, or the minimal value of a constant range. Undefined if  the 
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
		/// Returns the gradient representing the distribution, or the first gradient representing a gradient range.  Undefined 
		/// if the distribution is represented by a constant or a non-gradient range.
		/// </summary>
		public ColorGradient GetMinGradient()
		{
			return Internal_getMinGradient(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second gradient of a gradient range. Only defined if the distribution  represents 
		/// a gradient range.
		/// </summary>
		public ColorGradient GetMaxGradient()
		{
			return Internal_getMaxGradient(mCachedPtr);
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains gradients.
		/// </param>
		/// <param name="factor">
		/// Value in range [0, 1] that determines how to interpolate between min/max value, if the distribution represents a 
		/// range. Value of 0 will return the minimum value, while value of 1 will return the maximum value, and interpolate the 
		/// values in-between.
		/// </param>
		/// <returns>Evaluated color.</returns>
		public Color Evaluate(float t, float factor)
		{
			Color temp;
			Internal_evaluate(mCachedPtr, t, factor, out temp);
			return temp;
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains gradients.
		/// </param>
		/// <param name="factor">
		/// Random number generator that determines the factor. Factor determines how to interpolate between min/max value, if 
		/// the distribution represents a range.
		/// </param>
		/// <returns>Evaluated color.</returns>
		public Color Evaluate(float t, out Random factor)
		{
			Color temp;
			Internal_evaluate0(mCachedPtr, t, out factor, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorDistribution(ColorDistribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorDistribution0(ColorDistribution managedInstance, ref Color color);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorDistribution1(ColorDistribution managedInstance, ref Color minColor, ref Color maxColor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorDistribution2(ColorDistribution managedInstance, ColorGradient gradient);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ColorDistribution3(ColorDistribution managedInstance, ColorGradient minGradient, ColorGradient maxGradient);
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
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float t, float factor, out Color __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate0(IntPtr thisPtr, float t, out Random factor, out Color __output);
	}

	/** @} */
}
