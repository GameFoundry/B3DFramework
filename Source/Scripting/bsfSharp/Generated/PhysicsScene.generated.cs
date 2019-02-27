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
				Internal_getGravity(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setGravity(mCachedPtr, ref value); }
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
			return Internal_rayCast(mCachedPtr, ref ray, out hit, layer, max);
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
			return Internal_rayCast0(mCachedPtr, ref origin, ref unitDir, out hit, layer, max);
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
			return Internal_boxCast(mCachedPtr, ref box, ref rotation, ref unitDir, out hit, layer, max);
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
			return Internal_sphereCast(mCachedPtr, ref sphere, ref unitDir, out hit, layer, max);
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
			return Internal_capsuleCast(mCachedPtr, ref capsule, ref rotation, ref unitDir, out hit, layer, max);
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
			return Internal_convexCast(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, out hit, layer, max);
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
			return Internal_rayCastAll(mCachedPtr, ref ray, layer, max);
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
			return Internal_rayCastAll0(mCachedPtr, ref origin, ref unitDir, layer, max);
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
			return Internal_boxCastAll(mCachedPtr, ref box, ref rotation, ref unitDir, layer, max);
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
			return Internal_sphereCastAll(mCachedPtr, ref sphere, ref unitDir, layer, max);
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
			return Internal_capsuleCastAll(mCachedPtr, ref capsule, ref rotation, ref unitDir, layer, max);
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
			return Internal_convexCastAll(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, layer, max);
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
			return Internal_rayCastAny(mCachedPtr, ref ray, layer, max);
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
			return Internal_rayCastAny0(mCachedPtr, ref origin, ref unitDir, layer, max);
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
			return Internal_boxCastAny(mCachedPtr, ref box, ref rotation, ref unitDir, layer, max);
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
			return Internal_sphereCastAny(mCachedPtr, ref sphere, ref unitDir, layer, max);
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
			return Internal_capsuleCastAny(mCachedPtr, ref capsule, ref rotation, ref unitDir, layer, max);
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
			return Internal_convexCastAny(mCachedPtr, mesh, ref position, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided box.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the box.</returns>
		public Collider[] BoxOverlap(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_boxOverlap(mCachedPtr, ref box, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided sphere.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the sphere.</returns>
		public Collider[] SphereOverlap(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_sphereOverlap(mCachedPtr, ref sphere, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided capsule.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the capsule.</returns>
		public Collider[] CapsuleOverlap(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_capsuleOverlap(mCachedPtr, ref capsule, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided convex mesh.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the mesh.</returns>
		public Collider[] ConvexOverlap(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_convexOverlap(mCachedPtr, mesh, ref position, ref rotation, layer);
		}

		/// <summary>Checks if the provided box overlaps any other collider in the scene.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool BoxOverlapAny(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_boxOverlapAny(mCachedPtr, ref box, ref rotation, layer);
		}

		/// <summary>Checks if the provided sphere overlaps any other collider in the scene.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool SphereOverlapAny(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_sphereOverlapAny(mCachedPtr, ref sphere, layer);
		}

		/// <summary>Checks if the provided capsule overlaps any other collider in the scene.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool CapsuleOverlapAny(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_capsuleOverlapAny(mCachedPtr, ref capsule, ref rotation, layer);
		}

		/// <summary>Checks if the provided convex mesh overlaps any other collider in the scene.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public bool ConvexOverlapAny(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_convexOverlapAny(mCachedPtr, mesh, ref position, ref rotation, layer);
		}

		/// <summary>
		/// Adds a new physics region. Certain physics options require you to set up regions in which physics objects are allowed 
		/// to be in, and objects outside of these regions will not be handled by physics. You do not need to set up these 
		/// regions by default.
		/// </summary>
		public int AddPhysicsRegion(AABox region)
		{
			return Internal_addBroadPhaseRegion(mCachedPtr, ref region);
		}

		/// <summary>Removes a physics region.</summary>
		public void RemovePhysicsRegion(int handle)
		{
			Internal_removeBroadPhaseRegion(mCachedPtr, handle);
		}

		/// <summary>Removes all physics regions.</summary>
		public void ClearPhysicsRegions()
		{
			Internal_clearBroadPhaseRegions(mCachedPtr);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCast(IntPtr thisPtr, ref Ray ray, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCast0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxCast(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereCast(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleCast(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexCast(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_rayCastAll(IntPtr thisPtr, ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_rayCastAll0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_boxCastAll(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_sphereCastAll(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_capsuleCastAll(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_convexCastAll(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCastAny(IntPtr thisPtr, ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCastAny0(IntPtr thisPtr, ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxCastAny(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereCastAny(IntPtr thisPtr, ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleCastAny(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexCastAny(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_boxOverlap(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_sphereOverlap(IntPtr thisPtr, ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_capsuleOverlap(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_convexOverlap(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxOverlapAny(IntPtr thisPtr, ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereOverlapAny(IntPtr thisPtr, ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleOverlapAny(IntPtr thisPtr, ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexOverlapAny(IntPtr thisPtr, RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getGravity(IntPtr thisPtr, out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setGravity(IntPtr thisPtr, ref Vector3 gravity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern int Internal_addBroadPhaseRegion(IntPtr thisPtr, ref AABox region);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeBroadPhaseRegion(IntPtr thisPtr, int handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_clearBroadPhaseRegions(IntPtr thisPtr);
	}

	/** @} */
}
