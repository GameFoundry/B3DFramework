//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
	/** @addtogroup Physics
	 *  @{
	 */

	/// <summary>Collider with a capsule geometry.</summary>
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
				Internal_getNormal(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setNormal(mCachedPtr, ref value); }
		}

		/// <summary>Determines the position of the capsule shape, relative to the component's scene object.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Center
		{
			get
			{
				Vector3 temp;
				Internal_getCenter(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setCenter(mCachedPtr, ref value); }
		}

		/// <summary>
		/// Determines the half height of the capsule, from the origin to one of the hemispherical centers, along the normal  
		/// vector.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float HalfHeight
		{
			get { return Internal_getHalfHeight(mCachedPtr); }
			set { Internal_setHalfHeight(mCachedPtr, value); }
		}

		/// <summary>Determines the radius of the capsule.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_getRadius(mCachedPtr); }
			set { Internal_setRadius(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setNormal(IntPtr thisPtr, ref Vector3 normal);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getNormal(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCenter(IntPtr thisPtr, ref Vector3 center);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCenter(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setHalfHeight(IntPtr thisPtr, float halfHeight);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getHalfHeight(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRadius(IntPtr thisPtr);
	}

	/** @} */
}
