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
	/// Physical representation of a scene, allowing creation of new physical objects in the scene and queries against those 
	/// objects. Objects created in different scenes cannot physically interact with eachother.
	/// </summary>
	[ShowInInspector]
	public partial class PhysicsScene : ScriptObject
	{
		private PhysicsScene(bool __dummy0) { }
		protected PhysicsScene() { }

		/// <summary>Determines the global gravity value for all objects in the scene.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public Vector3 Gravity
		{
			get
			{
				Vector3 temp;
				Internal_GetGravity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetGravity(mCachedPtr, ref value); }
		}

		/// <summary>Casts a ray into the scene and returns the closest found hit, if any.</summary>
		/// <param name="ray">Ray to cast into the scene.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool RayCast(Ray ray, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCast(mCachedPtr, ref ray, out hit, layer, max);
		}

		/// <summary>Casts a ray into the scene and returns the closest found hit, if any.</summary>
		/// <param name="origin">Origin of the ray to cast into the scene.</param>
		/// <param name="unitDir">Unit direction of the ray to cast into the scene.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool RayCast(Vector3 origin, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCast0(mCachedPtr, ref origin, ref unitDir, out hit, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a box and returns the closest found hit, if any.</summary>
		/// <param name="box">Box to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool BoxCast(AABox box, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_BoxCast(mCachedPtr, ref box, ref rotation, ref unitDir, out hit, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a sphere and returns the closest found hit, if any.</summary>
		/// <param name="sphere">Sphere to sweep through the scene.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool SphereCast(Sphere sphere, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_SphereCast(mCachedPtr, ref sphere, ref unitDir, out hit, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a capsule and returns the closest found hit, if any.</summary>
		/// <param name="capsule">Capsule to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool CapsuleCast(Capsule capsule, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_CapsuleCast(mCachedPtr, ref capsule, ref rotation, ref unitDir, out hit, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a convex mesh and returns the closest found hit, if any.</summary>
		/// <param name="mesh">Mesh to sweep through the scene. Must be convex.</param>
		/// <param name="position">Starting position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool ConvexCast(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_ConvexCast(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, out hit, layer, max);
		}

		/// <summary>Casts a ray into the scene and returns all found hits.</summary>
		/// <param name="ray">Ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] RayCastAll(Ray ray, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCastAll(mCachedPtr, ref ray, layer, max);
		}

		/// <summary>Casts a ray into the scene and returns all found hits.</summary>
		/// <param name="origin">Origin of the ray to cast into the scene.</param>
		/// <param name="unitDir">Unit direction of the ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] RayCastAll(Vector3 origin, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCastAll0(mCachedPtr, ref origin, ref unitDir, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a box and returns all found hits.</summary>
		/// <param name="box">Box to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] BoxCastAll(AABox box, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_BoxCastAll(mCachedPtr, ref box, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a sphere and returns all found hits.</summary>
		/// <param name="sphere">Sphere to sweep through the scene.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] SphereCastAll(Sphere sphere, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_SphereCastAll(mCachedPtr, ref sphere, ref unitDir, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a capsule and returns all found hits.</summary>
		/// <param name="capsule">Capsule to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] CapsuleCastAll(Capsule capsule, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_CapsuleCastAll(mCachedPtr, ref capsule, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a convex mesh and returns all found hits.</summary>
		/// <param name="mesh">Mesh to sweep through the scene. Must be convex.</param>
		/// <param name="position">Starting position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public PhysicsQueryHit[] ConvexCastAll(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_ConvexCastAll(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>
		/// Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other 
		/// types of cast* calls.
		/// </summary>
		/// <param name="ray">Ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool RayCastAny(Ray ray, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCastAny(mCachedPtr, ref ray, layer, max);
		}

		/// <summary>
		/// Casts a ray into the scene and checks if it has hit anything. This can be significantly more efficient than other 
		/// types of cast* calls.
		/// </summary>
		/// <param name="origin">Origin of the ray to cast into the scene.</param>
		/// <param name="unitDir">Unit direction of the ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool RayCastAny(Vector3 origin, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_RayCastAny0(mCachedPtr, ref origin, ref unitDir, layer, max);
		}

		/// <summary>
		/// Performs a sweep into the scene using a box and checks if it has hit anything. This can be significantly more 
		/// efficient than other types of cast* calls.
		/// </summary>
		/// <param name="box">Box to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool BoxCastAny(AABox box, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_BoxCastAny(mCachedPtr, ref box, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>
		/// Performs a sweep into the scene using a sphere and checks if it has hit anything. This can be significantly more 
		/// efficient than other types of cast* calls.
		/// </summary>
		/// <param name="sphere">Sphere to sweep through the scene.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool SphereCastAny(Sphere sphere, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_SphereCastAny(mCachedPtr, ref sphere, ref unitDir, layer, max);
		}

		/// <summary>
		/// Performs a sweep into the scene using a capsule and checks if it has hit anything. This can be significantly more 
		/// efficient than other types of cast* calls.
		/// </summary>
		/// <param name="capsule">Capsule to sweep through the scene.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool CapsuleCastAny(Capsule capsule, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_CapsuleCastAny(mCachedPtr, ref capsule, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>
		/// Performs a sweep into the scene using a convex mesh and checks if it has hit anything. This can be significantly more 
		/// efficient than other types of cast* calls.
		/// </summary>
		/// <param name="mesh">Mesh to sweep through the scene. Must be convex.</param>
		/// <param name="position">Starting position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public bool ConvexCastAny(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_ConvexCastAny(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided box.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the box.</returns>
		public Collider[] BoxOverlap(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_BoxOverlap(mCachedPtr, ref box, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided sphere.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the sphere.</returns>
		public Collider[] SphereOverlap(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_SphereOverlap(mCachedPtr, ref sphere, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided capsule.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the capsule.</returns>
		public Collider[] CapsuleOverlap(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_CapsuleOverlap(mCachedPtr, ref capsule, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided convex mesh.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the mesh.</returns>
		public Collider[] ConvexOverlap(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_ConvexOverlap(mCachedPtr, mesh, ref position, ref rotation, layer);
		}

		/// <summary>Checks if the provided box overlaps any other collider in the scene.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool BoxOverlapAny(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_BoxOverlapAny(mCachedPtr, ref box, ref rotation, layer);
		}

		/// <summary>Checks if the provided sphere overlaps any other collider in the scene.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool SphereOverlapAny(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_SphereOverlapAny(mCachedPtr, ref sphere, layer);
		}

		/// <summary>Checks if the provided capsule overlaps any other collider in the scene.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool CapsuleOverlapAny(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_CapsuleOverlapAny(mCachedPtr, ref capsule, ref rotation, layer);
		}

		/// <summary>Checks if the provided convex mesh overlaps any other collider in the scene.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool ConvexOverlapAny(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_ConvexOverlapAny(mCachedPtr, mesh, ref position, ref rotation, layer);
		}

		/// <summary>
		/// Adds a new physics region. Certain physics options require you to set up regions in which physics objects are allowed 
		/// to be in, and objects outside of these regions will not be handled by physics. You do not need to set up these 
		/// regions by default.
		/// </summary>
		public int AddPhysicsRegion(AABox region)
		{
			return Internal_AddBroadPhaseRegion(mCachedPtr, ref region);
		}

		/// <summary>Removes a physics region.</summary>
		public void RemovePhysicsRegion(int handle)
		{
			Internal_RemoveBroadPhaseRegion(mCachedPtr, handle);
		}

		/// <summary>Removes all physics regions.</summary>
		public void ClearPhysicsRegions()
		{
			Internal_ClearBroadPhaseRegions(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_RayCast(IntPtr thisPtr, ref Ray ray, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_RayCast0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_BoxCast(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_SphereCast(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_CapsuleCast(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_ConvexCast(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_RayCastAll(IntPtr thisPtr, ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_RayCastAll0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_BoxCastAll(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_SphereCastAll(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_CapsuleCastAll(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_ConvexCastAll(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_RayCastAny(IntPtr thisPtr, ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_RayCastAny0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_BoxCastAny(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_SphereCastAny(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_CapsuleCastAny(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_ConvexCastAny(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_BoxOverlap(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_SphereOverlap(IntPtr thisPtr, ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_CapsuleOverlap(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_ConvexOverlap(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_BoxOverlapAny(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_SphereOverlapAny(IntPtr thisPtr, ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_CapsuleOverlapAny(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_ConvexOverlapAny(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetGravity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetGravity(IntPtr thisPtr, ref Vector3 gravity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_AddBroadPhaseRegion(IntPtr thisPtr, ref AABox region);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_RemoveBroadPhaseRegion(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_ClearBroadPhaseRegions(IntPtr thisPtr);
	}

	/** @} */
}
