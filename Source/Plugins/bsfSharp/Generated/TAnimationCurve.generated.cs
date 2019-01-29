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
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The spline 
	/// can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	/// </summary>
	[ShowInInspector]
	public partial class AnimationCurve : ScriptObject
	{
		private AnimationCurve(bool __dummy0) { }
		protected AnimationCurve() { }

		/// <summary>Creates a new animation curve.</summary>
		/// <param name="keyframes">Keyframes to initialize the curve with. They must be sorted by time.</param>
		public AnimationCurve(KeyFrame[] keyframes)
		{
			Internal_TAnimationCurve(this, keyframes);
		}

		/// <summary>Returns a list of all keyframes in the curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public KeyFrame[] KeyFrames
		{
			get { return Internal_getKeyFrames(mCachedPtr); }
		}

		/// <summary>
		/// Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider 
		/// using the cached version of evaluate() for better performance.
		/// </summary>
		/// <param name="time">%Time to evaluate the curve at.</param>
		/// <param name="loop">
		/// If true the curve will loop when it goes past the end or beggining. Otherwise the curve  value will be clamped.
		/// </param>
		/// <returns>Interpolated value from the curve at provided time.</returns>
		public float Evaluate(float time, bool loop = true)
		{
			return Internal_evaluate(mCachedPtr, time, loop);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TAnimationCurve(AnimationCurve managedInstance, KeyFrame[] keyframes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_evaluate(IntPtr thisPtr, float time, bool loop);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern KeyFrame[] Internal_getKeyFrames(IntPtr thisPtr);
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The spline 
	/// can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	/// </summary>
	[ShowInInspector]
	public partial class Vector3Curve : ScriptObject
	{
		private Vector3Curve(bool __dummy0) { }
		protected Vector3Curve() { }

		/// <summary>Creates a new animation curve.</summary>
		/// <param name="keyframes">Keyframes to initialize the curve with. They must be sorted by time.</param>
		public Vector3Curve(KeyFrameVec3[] keyframes)
		{
			Internal_TAnimationCurve(this, keyframes);
		}

		/// <summary>Returns a list of all keyframes in the curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public KeyFrameVec3[] KeyFrames
		{
			get { return Internal_getKeyFrames(mCachedPtr); }
		}

		/// <summary>
		/// Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider 
		/// using the cached version of evaluate() for better performance.
		/// </summary>
		/// <param name="time">%Time to evaluate the curve at.</param>
		/// <param name="loop">
		/// If true the curve will loop when it goes past the end or beggining. Otherwise the curve  value will be clamped.
		/// </param>
		/// <returns>Interpolated value from the curve at provided time.</returns>
		public Vector3 Evaluate(float time, bool loop = true)
		{
			Vector3 temp;
			Internal_evaluate(mCachedPtr, time, loop, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TAnimationCurve(Vector3Curve managedInstance, KeyFrameVec3[] keyframes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float time, bool loop, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern KeyFrameVec3[] Internal_getKeyFrames(IntPtr thisPtr);
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The spline 
	/// can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	/// </summary>
	[ShowInInspector]
	public partial class Vector2Curve : ScriptObject
	{
		private Vector2Curve(bool __dummy0) { }
		protected Vector2Curve() { }

		/// <summary>Creates a new animation curve.</summary>
		/// <param name="keyframes">Keyframes to initialize the curve with. They must be sorted by time.</param>
		public Vector2Curve(KeyFrameVec2[] keyframes)
		{
			Internal_TAnimationCurve(this, keyframes);
		}

		/// <summary>Returns a list of all keyframes in the curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public KeyFrameVec2[] KeyFrames
		{
			get { return Internal_getKeyFrames(mCachedPtr); }
		}

		/// <summary>
		/// Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider 
		/// using the cached version of evaluate() for better performance.
		/// </summary>
		/// <param name="time">%Time to evaluate the curve at.</param>
		/// <param name="loop">
		/// If true the curve will loop when it goes past the end or beggining. Otherwise the curve  value will be clamped.
		/// </param>
		/// <returns>Interpolated value from the curve at provided time.</returns>
		public Vector2 Evaluate(float time, bool loop = true)
		{
			Vector2 temp;
			Internal_evaluate(mCachedPtr, time, loop, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TAnimationCurve(Vector2Curve managedInstance, KeyFrameVec2[] keyframes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float time, bool loop, out Vector2 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern KeyFrameVec2[] Internal_getKeyFrames(IntPtr thisPtr);
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The spline 
	/// can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	/// </summary>
	[ShowInInspector]
	public partial class QuaternionCurve : ScriptObject
	{
		private QuaternionCurve(bool __dummy0) { }
		protected QuaternionCurve() { }

		/// <summary>Creates a new animation curve.</summary>
		/// <param name="keyframes">Keyframes to initialize the curve with. They must be sorted by time.</param>
		public QuaternionCurve(KeyFrameQuat[] keyframes)
		{
			Internal_TAnimationCurve(this, keyframes);
		}

		/// <summary>Returns a list of all keyframes in the curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public KeyFrameQuat[] KeyFrames
		{
			get { return Internal_getKeyFrames(mCachedPtr); }
		}

		/// <summary>
		/// Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider 
		/// using the cached version of evaluate() for better performance.
		/// </summary>
		/// <param name="time">%Time to evaluate the curve at.</param>
		/// <param name="loop">
		/// If true the curve will loop when it goes past the end or beggining. Otherwise the curve  value will be clamped.
		/// </param>
		/// <returns>Interpolated value from the curve at provided time.</returns>
		public Quaternion Evaluate(float time, bool loop = true)
		{
			Quaternion temp;
			Internal_evaluate(mCachedPtr, time, loop, out temp);
			return temp;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TAnimationCurve(QuaternionCurve managedInstance, KeyFrameQuat[] keyframes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_evaluate(IntPtr thisPtr, float time, bool loop, out Quaternion __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern KeyFrameQuat[] Internal_getKeyFrames(IntPtr thisPtr);
	}

	/** @} */

	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>
	/// Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The spline 
	/// can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	/// </summary>
	[ShowInInspector]
	public partial class IntegerCurve : ScriptObject
	{
		private IntegerCurve(bool __dummy0) { }
		protected IntegerCurve() { }

		/// <summary>Creates a new animation curve.</summary>
		/// <param name="keyframes">Keyframes to initialize the curve with. They must be sorted by time.</param>
		public IntegerCurve(KeyFrameInt[] keyframes)
		{
			Internal_TAnimationCurve(this, keyframes);
		}

		/// <summary>Returns a list of all keyframes in the curve.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public KeyFrameInt[] KeyFrames
		{
			get { return Internal_getKeyFrames(mCachedPtr); }
		}

		/// <summary>
		/// Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider 
		/// using the cached version of evaluate() for better performance.
		/// </summary>
		/// <param name="time">%Time to evaluate the curve at.</param>
		/// <param name="loop">
		/// If true the curve will loop when it goes past the end or beggining. Otherwise the curve  value will be clamped.
		/// </param>
		/// <returns>Interpolated value from the curve at provided time.</returns>
		public int Evaluate(float time, bool loop = true)
		{
			return Internal_evaluate(mCachedPtr, time, loop);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_TAnimationCurve(IntegerCurve managedInstance, KeyFrameInt[] keyframes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_evaluate(IntPtr thisPtr, float time, bool loop);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern KeyFrameInt[] Internal_getKeyFrames(IntPtr thisPtr);
	}

	/** @} */
}
