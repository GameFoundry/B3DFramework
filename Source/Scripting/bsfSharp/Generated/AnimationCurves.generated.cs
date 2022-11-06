//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/// <summary>A set of animation curves representing translation/rotation/scale and generic animation.</summary>
	[ShowInInspector]
	public partial class AnimationCurves : ScriptObject
	{
		private AnimationCurves(bool __dummy0) { }

		public AnimationCurves()
		{
			Internal_AnimationCurves(this);
		}

		/// <summary>Curves for animating scene object&apos;s position.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedVector3Curve[] Position
		{
			get { return Internal_GetPositionCurves(mCachedPtr); }
			set { Internal_SetPositionCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating scene object&apos;s rotation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedQuaternionCurve[] Rotation
		{
			get { return Internal_GetRotationCurves(mCachedPtr); }
			set { Internal_SetRotationCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating scene object&apos;s scale.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedVector3Curve[] Scale
		{
			get { return Internal_GetScaleCurves(mCachedPtr); }
			set { Internal_SetScaleCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating generic component properties.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedFloatCurve[] Generic
		{
			get { return Internal_GetGenericCurves(mCachedPtr); }
			set { Internal_SetGenericCurves(mCachedPtr, value); }
		}

		/// <summary>Registers a new curve used for animating position.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddPositionCurve(string name, Vector3Curve curve)
		{
			Internal_AddPositionCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for animating rotation.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddRotationCurve(string name, QuaternionCurve curve)
		{
			Internal_AddRotationCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for animating scale.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddScaleCurve(string name, Vector3Curve curve)
		{
			Internal_AddScaleCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for generic animation.</summary>
		/// <param name="name">
		/// Unique name of the curve. This can be used for retrieving the value of the curve from animation.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddGenericCurve(string name, AnimationCurve curve)
		{
			Internal_AddGenericCurve(mCachedPtr, name, curve);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemovePositionCurve(string name)
		{
			Internal_RemovePositionCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveRotationCurve(string name)
		{
			Internal_RemoveRotationCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveScaleCurve(string name)
		{
			Internal_RemoveScaleCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveGenericCurve(string name)
		{
			Internal_RemoveGenericCurve(mCachedPtr, name);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationCurves(AnimationCurves managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddPositionCurve(IntPtr thisPtr, string name, Vector3Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddRotationCurve(IntPtr thisPtr, string name, QuaternionCurve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddScaleCurve(IntPtr thisPtr, string name, Vector3Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AddGenericCurve(IntPtr thisPtr, string name, AnimationCurve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemovePositionCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveRotationCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveScaleCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveGenericCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedVector3Curve[] Internal_GetPositionCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPositionCurves(IntPtr thisPtr, NamedVector3Curve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedQuaternionCurve[] Internal_GetRotationCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRotationCurves(IntPtr thisPtr, NamedQuaternionCurve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedVector3Curve[] Internal_GetScaleCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetScaleCurves(IntPtr thisPtr, NamedVector3Curve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedFloatCurve[] Internal_GetGenericCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGenericCurves(IntPtr thisPtr, NamedFloatCurve[] value);
	}

	/** @} */
}
