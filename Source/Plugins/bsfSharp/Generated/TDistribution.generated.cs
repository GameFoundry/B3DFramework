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

	/// <summary>Specifies a value as a distribution, which can include a constant value, random range or a curve.</summary>
	[ShowInInspector]
	public partial class FloatDistribution : ScriptObject
	{
		private FloatDistribution(bool __dummy0) { }

		/// <summary>Creates a new empty distribution.</summary>
		public FloatDistribution()
		{
			Internal_TDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant value.</summary>
		public FloatDistribution(float value)
		{
			Internal_TDistribution0(this, value);
		}

		/// <summary>Creates a new distribution that returns a random value in the specified range.</summary>
		public FloatDistribution(float minValue, float maxValue)
		{
			Internal_TDistribution1(this, minValue, maxValue);
		}

		/// <summary>Creates a new distribution that evaluates a curve.</summary>
		public FloatDistribution(AnimationCurve curve)
		{
			Internal_TDistribution2(this, curve);
		}

		/// <summary>Creates a new distribution that returns a random value in a range determined by two curves.</summary>
		public FloatDistribution(AnimationCurve minCurve, AnimationCurve maxCurve)
		{
			Internal_TDistribution3(this, minCurve, maxCurve);
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
		/// distribution is represented by a curve.
		/// </summary>
		public float GetMinConstant()
		{
			return Internal_getMinConstant(mCachedPtr);
		}

		/// <summary>
		/// Returns the maximum value of a constant range. Only defined if the distribution represents a non-curve range.
		/// </summary>
		public float GetMaxConstant()
		{
			return Internal_getMaxConstant(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the distribution, or the first curve representing a curve range. Undefined if the 
		/// distribution is represented by a constant or a non-curve range.
		/// </summary>
		public AnimationCurve GetMinCurve()
		{
			return Internal_getMinCurve(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second curve of a curve range. Only defined if the distribution represents a curve 
		/// range.
		/// </summary>
		public AnimationCurve GetMaxCurve()
		{
			return Internal_getMaxCurve(mCachedPtr);
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Value in range [0, 1] that determines how to interpolate between min/max value, if the distribution represents a 
		/// range. Value of 0 will return the minimum value, while value of 1 will return the maximum value, and interpolate the 
		/// values in-between.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public float Evaluate(float t, float factor)
		{
			return Internal_evaluate(mCachedPtr, t, factor);
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Random number generator that determines the factor. Factor determines how to interpolate between min/max value, if 
		/// the distribution represents a range.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public float Evaluate(float t, Random factor)
		{
			return Internal_evaluate0(mCachedPtr, t, factor);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution(FloatDistribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution0(FloatDistribution managedInstance, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution1(FloatDistribution managedInstance, float minValue, float maxValue);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution2(FloatDistribution managedInstance, AnimationCurve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution3(FloatDistribution managedInstance, AnimationCurve minCurve, AnimationCurve maxCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PropertyDistributionType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMinConstant(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMaxConstant(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve Internal_getMinCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve Internal_getMaxCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_evaluate(IntPtr thisPtr, float t, float factor);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_evaluate0(IntPtr thisPtr, float t, Random factor);
	}

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Specifies a value as a distribution, which can include a constant value, random range or a curve.</summary>
	[ShowInInspector]
	public partial class Vector3Distribution : ScriptObject
	{
		private Vector3Distribution(bool __dummy0) { }

		/// <summary>Creates a new empty distribution.</summary>
		public Vector3Distribution()
		{
			Internal_TDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant value.</summary>
		public Vector3Distribution(Vector3 value)
		{
			Internal_TDistribution0(this, ref value);
		}

		/// <summary>Creates a new distribution that returns a random value in the specified range.</summary>
		public Vector3Distribution(Vector3 minValue, Vector3 maxValue)
		{
			Internal_TDistribution1(this, ref minValue, ref maxValue);
		}

		/// <summary>Creates a new distribution that evaluates a curve.</summary>
		public Vector3Distribution(Vector3Curve curve)
		{
			Internal_TDistribution2(this, curve);
		}

		/// <summary>Creates a new distribution that returns a random value in a range determined by two curves.</summary>
		public Vector3Distribution(Vector3Curve minCurve, Vector3Curve maxCurve)
		{
			Internal_TDistribution3(this, minCurve, maxCurve);
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
		/// distribution is represented by a curve.
		/// </summary>
		public Vector3 GetMinConstant()
		{
			Vector3 temp;
			Internal_getMinConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the maximum value of a constant range. Only defined if the distribution represents a non-curve range.
		/// </summary>
		public Vector3 GetMaxConstant()
		{
			Vector3 temp;
			Internal_getMaxConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the curve representing the distribution, or the first curve representing a curve range. Undefined if the 
		/// distribution is represented by a constant or a non-curve range.
		/// </summary>
		public Vector3Curve GetMinCurve()
		{
			return Internal_getMinCurve(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second curve of a curve range. Only defined if the distribution represents a curve 
		/// range.
		/// </summary>
		public Vector3Curve GetMaxCurve()
		{
			return Internal_getMaxCurve(mCachedPtr);
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Value in range [0, 1] that determines how to interpolate between min/max value, if the distribution represents a 
		/// range. Value of 0 will return the minimum value, while value of 1 will return the maximum value, and interpolate the 
		/// values in-between.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public Vector3 Evaluate(float t, float factor)
		{
			Vector3 temp;
			Internal_evaluate(mCachedPtr, t, factor, out temp);
			return temp;
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Random number generator that determines the factor. Factor determines how to interpolate between min/max value, if 
		/// the distribution represents a range.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public Vector3 Evaluate(float t, Random factor)
		{
			Vector3 temp;
			Internal_evaluate0(mCachedPtr, t, factor, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution(Vector3Distribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution0(Vector3Distribution managedInstance, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution1(Vector3Distribution managedInstance, ref Vector3 minValue, ref Vector3 maxValue);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution2(Vector3Distribution managedInstance, Vector3Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution3(Vector3Distribution managedInstance, Vector3Curve minCurve, Vector3Curve maxCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PropertyDistributionType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMinConstant(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMaxConstant(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_getMinCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_getMaxCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float t, float factor, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate0(IntPtr thisPtr, float t, Random factor, out Vector3 __output);
	}

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>Specifies a value as a distribution, which can include a constant value, random range or a curve.</summary>
	[ShowInInspector]
	public partial class Vector2Distribution : ScriptObject
	{
		private Vector2Distribution(bool __dummy0) { }

		/// <summary>Creates a new empty distribution.</summary>
		public Vector2Distribution()
		{
			Internal_TDistribution(this);
		}

		/// <summary>Creates a new distribution that returns a constant value.</summary>
		public Vector2Distribution(Vector2 value)
		{
			Internal_TDistribution0(this, ref value);
		}

		/// <summary>Creates a new distribution that returns a random value in the specified range.</summary>
		public Vector2Distribution(Vector2 minValue, Vector2 maxValue)
		{
			Internal_TDistribution1(this, ref minValue, ref maxValue);
		}

		/// <summary>Creates a new distribution that evaluates a curve.</summary>
		public Vector2Distribution(Vector2Curve curve)
		{
			Internal_TDistribution2(this, curve);
		}

		/// <summary>Creates a new distribution that returns a random value in a range determined by two curves.</summary>
		public Vector2Distribution(Vector2Curve minCurve, Vector2Curve maxCurve)
		{
			Internal_TDistribution3(this, minCurve, maxCurve);
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
		/// distribution is represented by a curve.
		/// </summary>
		public Vector2 GetMinConstant()
		{
			Vector2 temp;
			Internal_getMinConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the maximum value of a constant range. Only defined if the distribution represents a non-curve range.
		/// </summary>
		public Vector2 GetMaxConstant()
		{
			Vector2 temp;
			Internal_getMaxConstant(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns the curve representing the distribution, or the first curve representing a curve range. Undefined if the 
		/// distribution is represented by a constant or a non-curve range.
		/// </summary>
		public Vector2Curve GetMinCurve()
		{
			return Internal_getMinCurve(mCachedPtr);
		}

		/// <summary>
		/// Returns the curve representing the second curve of a curve range. Only defined if the distribution represents a curve 
		/// range.
		/// </summary>
		public Vector2Curve GetMaxCurve()
		{
			return Internal_getMaxCurve(mCachedPtr);
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Value in range [0, 1] that determines how to interpolate between min/max value, if the distribution represents a 
		/// range. Value of 0 will return the minimum value, while value of 1 will return the maximum value, and interpolate the 
		/// values in-between.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public Vector2 Evaluate(float t, float factor)
		{
			Vector2 temp;
			Internal_evaluate(mCachedPtr, t, factor, out temp);
			return temp;
		}

		/// <summary>Evaluates the value of the distribution.</summary>
		/// <param name="t">
		/// Time at which to evaluate the distribution. This is only relevant if the distribution contains curves.
		/// </param>
		/// <param name="factor">
		/// Random number generator that determines the factor. Factor determines how to interpolate between min/max value, if 
		/// the distribution represents a range.
		/// </param>
		/// <returns>Evaluated value.</returns>
		public Vector2 Evaluate(float t, Random factor)
		{
			Vector2 temp;
			Internal_evaluate0(mCachedPtr, t, factor, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution(Vector2Distribution managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution0(Vector2Distribution managedInstance, ref Vector2 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution1(Vector2Distribution managedInstance, ref Vector2 minValue, ref Vector2 maxValue);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution2(Vector2Distribution managedInstance, Vector2Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TDistribution3(Vector2Distribution managedInstance, Vector2Curve minCurve, Vector2Curve maxCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PropertyDistributionType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMinConstant(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getMaxConstant(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Curve Internal_getMinCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Curve Internal_getMaxCurve(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float t, float factor, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate0(IntPtr thisPtr, float t, Random factor, out Vector2 __output);
	}

	/** @} */
}
