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

	/// <summary>A collider with sphere geometry.</summary>
	[ShowInInspector]
	public partial class SphereCollider : Collider
	{
		private SphereCollider(bool __dummy0) { }
		protected SphereCollider() { }

		/// <summary>Determines the radius of the sphere geometry.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Radius
		{
			get { return Internal_getRadius(mCachedPtr); }
			set { Internal_setRadius(mCachedPtr, value); }
		}

		/// <summary>Determines position of the sphere shape, relative to the component&apos;s scene object.</summary>
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

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRadius(IntPtr thisPtr, float radius);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRadius(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCenter(IntPtr thisPtr, ref Vector3 center);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCenter(IntPtr thisPtr, out Vector3 __output);
	}

	/** @} */
}
