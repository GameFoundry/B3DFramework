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

	/// <summary>Contains a set of animation curves used for moving and rotating the root bone.</summary>
	[ShowInInspector]
	public partial class RootMotion : ScriptObject
	{
		private RootMotion(bool __dummy0) { }
		protected RootMotion() { }

		/// <summary>Animation curve representing the movement of the root bone.</summary>
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public Vector3Curve Position
		{
			get { return Internal_GetPositionCurves(mCachedPtr); }
		}

		/// <summary>Animation curve representing the rotation of the root bone.</summary>
		[NotNull]
		[PassByCopy]
		[NativeWrapper]
		public QuaternionCurve Rotation
		{
			get { return Internal_GetRotationCurves(mCachedPtr); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Vector3Curve Internal_GetPositionCurves(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern QuaternionCurve Internal_GetRotationCurves(IntPtr thisPtr);
	}

	/** @} */
}
