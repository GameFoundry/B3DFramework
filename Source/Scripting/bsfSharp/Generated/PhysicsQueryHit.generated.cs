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

	/// <summary>Hit information from a physics query.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct PhysicsQueryHit
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static PhysicsQueryHit Default()
		{
			PhysicsQueryHit value = new PhysicsQueryHit();
			value.Point = Vector3.Default();
			value.Normal = Vector3.Default();
			value.Uv = Vector2.Default();
			value.Distance = 0f;
			value.TriangleIdx = 0;
			value.UnmappedTriangleIdx = 0;
			value.Collider = null;

			return value;
		}

		/// <summary>Position of the hit in world space.</summary>
		public Vector3 Point;
		/// <summary>Normal to the surface that was hit.</summary>
		public Vector3 Normal;
		/// <summary>
		/// Barycentric coordinates of the triangle that was hit (only applicable when triangle meshes are hit).
		/// </summary>
		public Vector2 Uv;
		/// <summary>Distance from the query origin to the hit position.</summary>
		public float Distance;
		/// <summary>Index of the triangle that was hit (only applicable when triangle meshes are hit).</summary>
		public int TriangleIdx;
		/// <summary>
		/// Unmapped index of the triangle that was hit (only applicable when triangle meshes are hit). It represents an index 
		/// into the original MeshData used to create the PhysicsMesh associated with <see cref="collider"/>. In contrast, <see 
		/// cref="triangleIdx"/> is only a valid index for the MeshData directly obtained from #collider which can differ from 
		/// the original MeshData due to the internal implementation.
		/// </summary>
		public int UnmappedTriangleIdx;
		/// <summary>
		/// Component of the collider that was hit. This may be null if the hit collider has no owner component, in which case 
		/// refer to #colliderRaw.
		/// </summary>
		public Collider Collider;
	}

	/** @} */
}
