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
	public partial class PlaneCollider : Collider
	{
		private PlaneCollider(bool __dummy0) { }
		protected PlaneCollider() { }

		/// <summary>Normal vector that determines the local orientation of the plane.</summary>
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

		/// <summary>Determines the distance of the plane from the local origin, along its normal vector.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Distance
		{
			get { return Internal_GetDistance(mCachedPtr); }
			set { Internal_SetDistance(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetNormal(IntPtr thisPtr, ref Vector3 normal);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetNormal(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetDistance(IntPtr thisPtr, float distance);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_GetDistance(IntPtr thisPtr);
	}

	/** @} */
}
