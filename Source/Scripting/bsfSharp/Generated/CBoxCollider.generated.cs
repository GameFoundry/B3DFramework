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
	public partial class BoxCollider : Collider
	{
		private BoxCollider(bool __dummy0) { }
		protected BoxCollider() { }

		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Extents
		{
			get
			{
				Vector3 temp;
				Internal_GetExtents(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetExtents(mCachedPtr, ref value); }
		}

		/// <summary>Determines the position of the box shape, relative to the component&apos;s scene object.</summary>
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

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetExtents(IntPtr thisPtr, ref Vector3 extents);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetExtents(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetCenter(IntPtr thisPtr, ref Vector3 center);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetCenter(IntPtr thisPtr, out Vector3 __output);
	}

	/** @} */
}
