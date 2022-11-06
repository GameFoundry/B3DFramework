//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	[ShowInInspector]
	public partial class CapsuleCollider : Collider
	{
		private CapsuleCollider(bool __dummy0) { }
		protected CapsuleCollider() { }

		/// <summary>Normal vector that determines how is the capsule oriented.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Normal
		{
			get
			{
				Vector3 temp;
				Internal_GetNormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetNormal(mCachedPtr, ref value); }
		}

		/// <summary>Determines the position of the capsule shape, relative to the component&apos;s scene object.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Center
		{
			get
			{
				Vector3 temp;
				Internal_GetCenter(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetCenter(mCachedPtr, ref value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float HalfHeight
		{
			get { return Internal_GetHalfHeight(mCachedPtr); }
			set { Internal_SetHalfHeight(mCachedPtr, value); }
		}

		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_GetRadius(mCachedPtr); }
			set { Internal_SetRadius(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNormal(IntPtr thisPtr, ref Vector3 normal);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetNormal(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCenter(IntPtr thisPtr, ref Vector3 center);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCenter(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetHalfHeight(IntPtr thisPtr, float halfHeight);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetHalfHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetRadius(IntPtr thisPtr);
	}

	/** @} */
}
