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

	/// <summary>
	/// Represents a physics mesh that can be used with a MeshCollider. Physics mesh can be a generic triangle mesh or a 
	/// convex mesh. Convex meshes are limited to 255 faces.
	/// </summary>
	[ShowInInspector]
	public partial class PhysicsMesh : Resource
	{
		private PhysicsMesh(bool __dummy0) { }
		protected PhysicsMesh() { }

		public PhysicsMesh(MeshData meshData, PhysicsMeshType type = PhysicsMeshType.Convex)
		{
			Internal_Create(this, meshData, type);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<PhysicsMesh> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns the type of the physics mesh.</summary>
		[NativeWrapper]
		public PhysicsMeshType Type
		{
			get { return Internal_GetType(mCachedPtr); }
		}

		[NativeWrapper]
		public MeshData MeshData
		{
			get { return Internal_GetMeshData(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<PhysicsMesh>(PhysicsMesh x)
		{
			if(x != null)
				return Internal_GetRef(x.mCachedPtr);
			else
				return null;
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<PhysicsMesh> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsMeshType Internal_GetType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(PhysicsMesh managedInstance, MeshData meshData, PhysicsMeshType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MeshData Internal_GetMeshData(IntPtr thisPtr);
	}

	/** @} */
}
