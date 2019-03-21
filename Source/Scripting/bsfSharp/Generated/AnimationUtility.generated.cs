//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>Helper class for dealing with animations, animation clips and curves.</summary>
	[ShowInInspector]
	public partial class AnimationUtility : ScriptObject
	{
		private AnimationUtility(bool __dummy0) { }
		protected AnimationUtility() { }

		/// <summary>Converts a curve in euler angles (in degrees) into a curve using quaternions.</summary>
		public static QuaternionCurve EulerToQuaternionCurve(Vector3Curve eulerCurve, EulerAngleOrder order = EulerAngleOrder.YXZ)
		{
			return Internal_eulerToQuaternionCurve(eulerCurve, order);
		}

		/// <summary>Converts a curve in quaternions into a curve using euler angles (in degrees).</summary>
		public static Vector3Curve QuaternionToEulerCurve(QuaternionCurve quatCurve)
		{
			return Internal_quaternionToEulerCurve(quatCurve);
		}

		/// <summary>Splits a Vector3 curve into three individual curves, one for each component.</summary>
		public static AnimationCurve[] SplitCurve3D(Vector3Curve compoundCurve)
		{
			return Internal_splitCurve3D(compoundCurve);
		}

		/// <summary>Combines three single component curves into a Vector3 curve.</summary>
		public static Vector3Curve CombineCurve3D(AnimationCurve[] curveComponents)
		{
			return Internal_combineCurve3D(curveComponents);
		}

		/// <summary>Splits a Vector2 curve into two individual curves, one for each component.</summary>
		public static AnimationCurve[] SplitCurve2D(Vector2Curve compoundCurve)
		{
			return Internal_splitCurve2D(compoundCurve);
		}

		/// <summary>Combines two single component curves into a Vector2 curve.</summary>
		public static Vector2Curve CombineCurve2D(AnimationCurve[] curveComponents)
		{
			return Internal_combineCurve2D(curveComponents);
		}

		/// <summary>Calculates the total range covered by a set of curves.</summary>
		/// <param name="curves">Curves to calculate range for.</param>
		/// <param name="xMin">Minimum time value present in the curves.</param>
		/// <param name="xMax">Maximum time value present in the curves.</param>
		/// <param name="yMin">Minimum curve value present in the curves.</param>
		/// <param name="yMax">Maximum curve value present in the curves.</param>
		public static void CalculateRange(AnimationCurve[] curves, out float xMin, out float xMax, out float yMin, out float yMax)
		{
			Internal_calculateRange(curves, out xMin, out xMax, out yMin, out yMax);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern QuaternionCurve Internal_eulerToQuaternionCurve(Vector3Curve eulerCurve, EulerAngleOrder order);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_quaternionToEulerCurve(QuaternionCurve quatCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve[] Internal_splitCurve3D(Vector3Curve compoundCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_combineCurve3D(AnimationCurve[] curveComponents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve[] Internal_splitCurve2D(Vector2Curve compoundCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Curve Internal_combineCurve2D(AnimationCurve[] curveComponents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_calculateRange(AnimationCurve[] curves, out float xMin, out float xMax, out float yMin, out float yMax);
	}

	/** @} */
}
