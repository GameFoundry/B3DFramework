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

	/// <summary>Collider with box geometry.</summary>
	[ShowInInspector]
	public partial class BoxCollider : Collider
	{
		private BoxCollider(bool __dummy0) { }
		protected BoxCollider() { }

		/// <summary>Determines the extents (half size) of the geometry of the box.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Extents
		{
			get
			{
				Vector3 temp;
				Internal_getExtents(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setExtents(mCachedPtr, ref value); }
		}

		/// <summary>Determines the position of the box shape, relative to the component&apos;s scene object.</summary>
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
		private static extern void Internal_setExtents(IntPtr thisPtr, ref Vector3 extents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getExtents(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCenter(IntPtr thisPtr, ref Vector3 center);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getCenter(IntPtr thisPtr, out Vector3 __output);
	}

	/** @} */
}
