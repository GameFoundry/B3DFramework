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
			get { return Internal_getPositionCurves(mCachedPtr); }
			set { Internal_setPositionCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating scene object&apos;s rotation.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedQuaternionCurve[] Rotation
		{
			get { return Internal_getRotationCurves(mCachedPtr); }
			set { Internal_setRotationCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating scene object&apos;s scale.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedVector3Curve[] Scale
		{
			get { return Internal_getScaleCurves(mCachedPtr); }
			set { Internal_setScaleCurves(mCachedPtr, value); }
		}

		/// <summary>Curves for animating generic component properties.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public NamedFloatCurve[] Generic
		{
			get { return Internal_getGenericCurves(mCachedPtr); }
			set { Internal_setGenericCurves(mCachedPtr, value); }
		}

		/// <summary>Registers a new curve used for animating position.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddPositionCurve(string name, Vector3Curve curve)
		{
			Internal_addPositionCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for animating rotation.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddRotationCurve(string name, QuaternionCurve curve)
		{
			Internal_addRotationCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for animating scale.</summary>
		/// <param name="name">
		/// Unique name of the curve. This name will be used mapping the curve to the relevant bone in a skeleton, if any.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddScaleCurve(string name, Vector3Curve curve)
		{
			Internal_addScaleCurve(mCachedPtr, name, curve);
		}

		/// <summary>Registers a new curve used for generic animation.</summary>
		/// <param name="name">
		/// Unique name of the curve. This can be used for retrieving the value of the curve from animation.
		/// </param>
		/// <param name="curve">Curve to add to the clip.</param>
		public void AddGenericCurve(string name, AnimationCurve curve)
		{
			Internal_addGenericCurve(mCachedPtr, name, curve);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemovePositionCurve(string name)
		{
			Internal_removePositionCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveRotationCurve(string name)
		{
			Internal_removeRotationCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveScaleCurve(string name)
		{
			Internal_removeScaleCurve(mCachedPtr, name);
		}

		/// <summary>Removes an existing curve from the clip.</summary>
		public void RemoveGenericCurve(string name)
		{
			Internal_removeGenericCurve(mCachedPtr, name);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_AnimationCurves(AnimationCurves managedInstance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addPositionCurve(IntPtr thisPtr, string name, Vector3Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addRotationCurve(IntPtr thisPtr, string name, QuaternionCurve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addScaleCurve(IntPtr thisPtr, string name, Vector3Curve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_addGenericCurve(IntPtr thisPtr, string name, AnimationCurve curve);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removePositionCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeRotationCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeScaleCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeGenericCurve(IntPtr thisPtr, string name);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedVector3Curve[] Internal_getPositionCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPositionCurves(IntPtr thisPtr, NamedVector3Curve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedQuaternionCurve[] Internal_getRotationCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRotationCurves(IntPtr thisPtr, NamedQuaternionCurve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedVector3Curve[] Internal_getScaleCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setScaleCurves(IntPtr thisPtr, NamedVector3Curve[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern NamedFloatCurve[] Internal_getGenericCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setGenericCurves(IntPtr thisPtr, NamedFloatCurve[] value);
	}

	/** @} */
}
