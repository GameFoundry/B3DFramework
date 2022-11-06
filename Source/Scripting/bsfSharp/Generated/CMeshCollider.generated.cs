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
	public partial class MeshCollider : Collider
	{
		private MeshCollider(bool __dummy0) { }
		protected MeshCollider() { }

		[ShowInInspector]
		[NativeWrapper]
		public RRef<PhysicsMesh> Mesh
		{
			get { return Internal_GetMesh(mCachedPtr); }
			set { Internal_SetMesh(mCachedPtr, value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetMesh(IntPtr thisPtr, RRef<PhysicsMesh> mesh);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<PhysicsMesh> Internal_GetMesh(IntPtr thisPtr);
	}

	/** @} */
}
