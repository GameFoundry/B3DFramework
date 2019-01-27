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

	/// <summary>
	/// Represents a physics mesh that can be used with a MeshCollider. Physics mesh can be a generic triangle mesh or a 
	/// convex mesh. Convex meshes are limited to 255 faces.
	/// </summary>
	[ShowInInspector]
	public partial class PhysicsMesh : Resource
	{
		private PhysicsMesh(bool __dummy0) { }
		protected PhysicsMesh() { }

		/// <summary>Creates a new physics mesh.</summary>
		/// <param name="meshData">Index and vertices of the mesh data.</param>
		/// <param name="type">
		/// Type of the mesh. If convex the provided mesh geometry will be converted into a convex mesh (that might not be the 
		/// same as the provided mesh data).
		/// </param>
		public PhysicsMesh(MeshData meshData, PhysicsMeshType type = PhysicsMeshType.Convex)
		{
			Internal_create(this, meshData, type);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<PhysicsMesh> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns the type of the physics mesh.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public PhysicsMeshType Type
		{
			get { return Internal_getType(mCachedPtr); }
		}

		/// <summary>Returns the mesh's indices and vertices.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public MeshData MeshData
		{
			get { return Internal_getMeshData(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<PhysicsMesh>(PhysicsMesh x)
		{ return Internal_GetRef(x.mCachedPtr); }

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<PhysicsMesh> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsMeshType Internal_getType(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(PhysicsMesh managedInstance, MeshData meshData, PhysicsMeshType type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern MeshData Internal_getMeshData(IntPtr thisPtr);
	}

	/** @} */
}
