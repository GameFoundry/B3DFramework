//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
	/** @addtogroup Particles
	 *  @{
	 */

	/// <summary>
	/// Represents a three dimensional field of vectors. It is represented by spatial bounds which are split into a grid of 
	/// values with user-defined density, where each grid cell is assigned a vector.
	/// </summary>
	[ShowInInspector]
	public partial class VectorField : Resource
	{
		private VectorField(bool __dummy0) { }
		protected VectorField() { }

		/// <summary>Creates a new vector field.</summary>
		/// <param name="desc">Description of the vector field to create.</param>
		/// <param name="values">
		/// Values to assign to the vector field. Number of entries must match  countX * countY * countZ.
		/// </param>
		public VectorField(VectorFieldOptions desc, Vector3[] values)
		{
			Internal_create(this, ref desc, values);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<VectorField> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<VectorField>(VectorField x)
		{ return Internal_GetRef(x.mCachedPtr); }

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<VectorField> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(VectorField managedInstance, ref VectorFieldOptions desc, Vector3[] values);
	}

	/** @} */
}
