//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace b3d
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
			return Internal_EulerToQuaternionCurve(eulerCurve, order);
		}

		/// <summary>Converts a curve in quaternions into a curve using euler angles (in degrees).</summary>
		public static Vector3Curve QuaternionToEulerCurve(QuaternionCurve quatCurve)
		{
			return Internal_QuaternionToEulerCurve(quatCurve);
		}

		/// <summary>Splits a Vector3 curve into three individual curves, one for each component.</summary>
		public static AnimationCurve[] SplitCurve3D(Vector3Curve compoundCurve)
		{
			return Internal_SplitCurve3D(compoundCurve);
		}

		/// <summary>Combines three single component curves into a Vector3 curve.</summary>
		public static Vector3Curve CombineCurve3D(AnimationCurve[] curveComponents)
		{
			return Internal_CombineCurve3D(curveComponents);
		}

		/// <summary>Splits a Vector2 curve into two individual curves, one for each component.</summary>
		public static AnimationCurve[] SplitCurve2D(Vector2Curve compoundCurve)
		{
			return Internal_SplitCurve2D(compoundCurve);
		}

		/// <summary>Combines two single component curves into a Vector2 curve.</summary>
		public static Vector2Curve CombineCurve2D(AnimationCurve[] curveComponents)
		{
			return Internal_CombineCurve2D(curveComponents);
		}

		public static void CalculateRange(AnimationCurve[] curves, out float outXMin, out float outXMax, out float outYMin, out float outYMax)
		{
			Internal_CalculateRange(curves, out outXMin, out outXMax, out outYMin, out outYMax);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern QuaternionCurve Internal_EulerToQuaternionCurve(Vector3Curve eulerCurve, EulerAngleOrder order);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_QuaternionToEulerCurve(QuaternionCurve quatCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve[] Internal_SplitCurve3D(Vector3Curve compoundCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_CombineCurve3D(AnimationCurve[] curveComponents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern AnimationCurve[] Internal_SplitCurve2D(Vector2Curve compoundCurve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector2Curve Internal_CombineCurve2D(AnimationCurve[] curveComponents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_CalculateRange(AnimationCurve[] curves, out float outXMin, out float outXMax, out float outYMin, out float outYMax);
	}

	/** @} */
}
