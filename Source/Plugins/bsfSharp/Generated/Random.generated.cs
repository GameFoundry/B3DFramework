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
	/** @addtogroup Math
	 *  @{
	 */

	/// <summary>
	/// Generates pseudo random numbers using the Xorshift128 algorithm. Suitable for high performance requirements.
	/// </summary>
	[ShowInInspector]
	public partial class Random : ScriptObject
	{
		private Random(bool __dummy0) { }
		protected Random() { }

		/// <summary>Initializes a new generator using the specified seed.</summary>
		public Random(uint seed = 0)
		{
			Internal_Random(this, seed);
		}

		/// <summary>Changes the seed of the generator to the specified value.</summary>
		public void SetSeed(uint seed)
		{
			Internal_setSeed(mCachedPtr, seed);
		}

		/// <summary>Returns a random value in range [0, std::numeric_limits<uint32_t>::max()].</summary>
		public uint Get()
		{
			return Internal_get(mCachedPtr);
		}

		/// <summary>Returns a random value in range [min, max].</summary>
		public int GetRange(int min, int max)
		{
			return Internal_getRange(mCachedPtr, min, max);
		}

		/// <summary>Returns a random value in range [0, 1].</summary>
		public float GetUNorm()
		{
			return Internal_getUNorm(mCachedPtr);
		}

		/// <summary>Returns a random value in range [-1, 1].</summary>
		public float GetSNorm()
		{
			return Internal_getSNorm(mCachedPtr);
		}

		/// <summary>Returns a random unit vector in three dimensions.</summary>
		public Vector3 GetUnitVector()
		{
			Vector3 temp;
			Internal_getUnitVector(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>Returns a random unit vector in two dimensions.</summary>
		public Vector2 GetUnitVector2D()
		{
			Vector2 temp;
			Internal_getUnitVector2D(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>Returns a random point inside a unit sphere.</summary>
		public Vector3 GetPointInSphere()
		{
			Vector3 temp;
			Internal_getPointInSphere(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a random point inside the specified range in a sphere shell of unit radius, with the specified thickness, in 
		/// range [0, 1]. Thickness of 0 will generate points on the sphere surface, while thickness of 1 will generate points 
		/// within the entire sphere volume. Intermediate values represent the shell, which is a volume between two concentric 
		/// spheres.
		/// </summary>
		public Vector3 GetPointInSphereShell(float thickness)
		{
			Vector3 temp;
			Internal_getPointInSphereShell(mCachedPtr, thickness, out temp);
			return temp;
		}

		/// <summary>Returns a random point inside a unit circle.</summary>
		public Vector2 GetPointInCircle()
		{
			Vector2 temp;
			Internal_getPointInCircle(mCachedPtr, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a random point inside the specified range in a circle shell of unit radius, with the specified thickness, in 
		/// range [0, 1]. Thickness of 0 will generate points on the circle edge, while thickness of 1 will generate points 
		/// within the entire circle surface. Intermediate values represent the shell, which is the surface between two 
		/// concentric circles.
		/// </summary>
		public Vector2 GetPointInCircleShell(float thickness)
		{
			Vector2 temp;
			Internal_getPointInCircleShell(mCachedPtr, thickness, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a random point on a unit arc with the specified length (angle). Angle of 360 represents a circle.
		/// </summary>
		public Vector2 GetPointInArc(Degree angle)
		{
			Vector2 temp;
			Internal_getPointInArc(mCachedPtr, ref angle, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a random point inside the specified range in an arc shell of unit radius, with the specified  length (angle) 
		/// and thickness in range [0, 1]. Angle of 360 represents a circle shell. Thickness of 0 will generate points on the arc 
		/// edge, while thickness of 1 will generate points on the entire arc 'slice'.  Intermediate vlaues represent the shell, 
		/// which is the surface between two concentric circles.
		/// </summary>
		public Vector2 GetPointInArcShell(Degree angle, float thickness)
		{
			Vector2 temp;
			Internal_getPointInArcShell(mCachedPtr, ref angle, thickness, out temp);
			return temp;
		}

		/// <summary>
		/// Returns a random set of Barycentric coordinates that may be used for generating random points on a triangle.
		/// </summary>
		public Vector3 GetBarycentric()
		{
			Vector3 temp;
			Internal_getBarycentric(mCachedPtr, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Random(Random managedInstance, uint seed);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setSeed(IntPtr thisPtr, uint seed);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_get(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_getRange(IntPtr thisPtr, int min, int max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getUNorm(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getSNorm(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getUnitVector(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getUnitVector2D(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInSphere(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInSphereShell(IntPtr thisPtr, float thickness, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInCircle(IntPtr thisPtr, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInCircleShell(IntPtr thisPtr, float thickness, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInArc(IntPtr thisPtr, ref Degree angle, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getPointInArcShell(IntPtr thisPtr, ref Degree angle, float thickness, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getBarycentric(IntPtr thisPtr, out Vector3 __output);
	}

	/** @} */
}
