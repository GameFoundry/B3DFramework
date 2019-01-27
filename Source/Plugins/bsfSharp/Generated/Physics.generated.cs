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

	/// <summary>Provides global physics settings, factory methods for physics objects and scene queries.</summary>
	[ShowInInspector]
	public partial class Physics : ScriptObject
	{
		private Physics(bool __dummy0) { }
		protected Physics() { }

		/// <summary>Determines the global gravity value for all objects in the scene.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static Vector3 Gravity
		{
			get
			{
				Vector3 temp;
				Internal_getGravity(out temp);
				return temp;
			}
			set { Internal_setGravity(ref value); }
		}

		/// <summary>Checks is the physics simulation update currently in progress.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public static bool IsUpdateInProgress
		{
			get { return Internal__isUpdateInProgress(); }
		}

		/// <summary>Casts a ray into the scene and returns the closest found hit, if any.</summary>
		/// <param name="ray">Ray to cast into the scene.</param>
		/// <param name="hit">Information recorded about a hit. Only valid if method returns true.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>True if something was hit, false otherwise.</returns>
		public static bool RayCast(Ray ray, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCast(ref ray, out hit, layer, max);
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
		public static bool RayCast(Vector3 origin, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCast0(ref origin, ref unitDir, out hit, layer, max);
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
		public static bool BoxCast(AABox box, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_boxCast(ref box, ref rotation, ref unitDir, out hit, layer, max);
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
		public static bool SphereCast(Sphere sphere, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_sphereCast(ref sphere, ref unitDir, out hit, layer, max);
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
		public static bool CapsuleCast(Capsule capsule, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_capsuleCast(ref capsule, ref rotation, ref unitDir, out hit, layer, max);
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
		public static bool ConvexCast(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, out PhysicsQueryHit hit, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_convexCast(mesh, ref position, ref rotation, ref unitDir, out hit, layer, max);
		}

		/// <summary>Casts a ray into the scene and returns all found hits.</summary>
		/// <param name="ray">Ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public static PhysicsQueryHit[] RayCastAll(Ray ray, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCastAll(ref ray, layer, max);
		}

		/// <summary>Casts a ray into the scene and returns all found hits.</summary>
		/// <param name="origin">Origin of the ray to cast into the scene.</param>
		/// <param name="unitDir">Unit direction of the ray to cast into the scene.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public static PhysicsQueryHit[] RayCastAll(Vector3 origin, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCastAll0(ref origin, ref unitDir, layer, max);
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
		public static PhysicsQueryHit[] BoxCastAll(AABox box, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_boxCastAll(ref box, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Performs a sweep into the scene using a sphere and returns all found hits.</summary>
		/// <param name="sphere">Sphere to sweep through the scene.</param>
		/// <param name="unitDir">Unit direction towards which to perform the sweep.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <param name="max">
		/// Maximum distance at which to perform the query. Hits past this distance will not be detected.
		/// </param>
		/// <returns>List of all detected hits.</returns>
		public static PhysicsQueryHit[] SphereCastAll(Sphere sphere, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_sphereCastAll(ref sphere, ref unitDir, layer, max);
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
		public static PhysicsQueryHit[] CapsuleCastAll(Capsule capsule, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_capsuleCastAll(ref capsule, ref rotation, ref unitDir, layer, max);
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
		public static PhysicsQueryHit[] ConvexCastAll(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_convexCastAll(mesh, ref position, ref rotation, ref unitDir, layer, max);
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
		public static bool RayCastAny(Ray ray, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCastAny(ref ray, layer, max);
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
		public static bool RayCastAny(Vector3 origin, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_rayCastAny0(ref origin, ref unitDir, layer, max);
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
		public static bool BoxCastAny(AABox box, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_boxCastAny(ref box, ref rotation, ref unitDir, layer, max);
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
		public static bool SphereCastAny(Sphere sphere, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_sphereCastAny(ref sphere, ref unitDir, layer, max);
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
		public static bool CapsuleCastAny(Capsule capsule, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_capsuleCastAny(ref capsule, ref rotation, ref unitDir, layer, max);
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
		public static bool ConvexCastAny(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, Vector3 unitDir, ulong layer = 18446744073709551615, float max = 3.40282347E+38f)
		{
			return Internal_convexCastAny(mesh, ref position, ref rotation, ref unitDir, layer, max);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided box.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the box.</returns>
		public static Collider[] BoxOverlap(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_boxOverlap(ref box, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided sphere.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the sphere.</returns>
		public static Collider[] SphereOverlap(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_sphereOverlap(ref sphere, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided capsule.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the capsule.</returns>
		public static Collider[] CapsuleOverlap(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_capsuleOverlap(ref capsule, ref rotation, layer);
		}

		/// <summary>Returns a list of all colliders in the scene that overlap the provided convex mesh.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>List of all colliders that overlap the mesh.</returns>
		public static Collider[] ConvexOverlap(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_convexOverlap(mesh, ref position, ref rotation, layer);
		}

		/// <summary>Checks if the provided box overlaps any other collider in the scene.</summary>
		/// <param name="box">Box to check for overlap.</param>
		/// <param name="rotation">Orientation of the box.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public static bool BoxOverlapAny(AABox box, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_boxOverlapAny(ref box, ref rotation, layer);
		}

		/// <summary>Checks if the provided sphere overlaps any other collider in the scene.</summary>
		/// <param name="sphere">Sphere to check for overlap.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public static bool SphereOverlapAny(Sphere sphere, ulong layer = 18446744073709551615)
		{
			return Internal_sphereOverlapAny(ref sphere, layer);
		}

		/// <summary>Checks if the provided capsule overlaps any other collider in the scene.</summary>
		/// <param name="capsule">Capsule to check for overlap.</param>
		/// <param name="rotation">Orientation of the capsule.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public static bool CapsuleOverlapAny(Capsule capsule, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_capsuleOverlapAny(ref capsule, ref rotation, layer);
		}

		/// <summary>Checks if the provided convex mesh overlaps any other collider in the scene.</summary>
		/// <param name="mesh">Mesh to check for overlap. Must be convex.</param>
		/// <param name="position">Position of the mesh.</param>
		/// <param name="rotation">Orientation of the mesh.</param>
		/// <param name="layer">Layers to consider for the query. This allows you to ignore certain groups of objects.</param>
		/// <returns>True if there is overlap with another object, false otherwise.</returns>
		public static bool ConvexOverlapAny(RRef<PhysicsMesh> mesh, Vector3 position, Quaternion rotation, ulong layer = 18446744073709551615)
		{
			return Internal_convexOverlapAny(mesh, ref position, ref rotation, layer);
		}

		/// <summary>
		/// Adds a new physics region. Certain physics options require you to set up regions in which physics objects are allowed 
		/// to be in, and objects outside of these regions will not be handled by physics. You do not need to set up these 
		/// regions by default.
		/// </summary>
		public static uint AddPhysicsRegion(AABox region)
		{
			return Internal_addBroadPhaseRegion(ref region);
		}

		/// <summary>Removes a physics region.</summary>
		public static void RemovePhysicsRegion(uint handle)
		{
			Internal_removeBroadPhaseRegion(handle);
		}

		/// <summary>Removes all physics regions.</summary>
		public static void ClearPhysicsRegions()
		{
			Internal_clearBroadPhaseRegions();
		}

		/// <summary>
		/// Enables or disables collision between two layers. Each physics object can be assigned a specific layer, and here you 
		/// can determine which layers can interact with each other.
		/// </summary>
		public static void ToggleCollision(ulong groupA, ulong groupB, bool enabled)
		{
			Internal_toggleCollision(groupA, groupB, enabled);
		}

		/// <summary>Checks if two collision layers are allowed to interact.</summary>
		public static bool IsCollisionEnabled(ulong groupA, ulong groupB)
		{
			return Internal_isCollisionEnabled(groupA, groupB);
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCast(ref Ray ray, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCast0(ref Vector3 origin, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxCast(ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereCast(ref Sphere sphere, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleCast(ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexCast(RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, out PhysicsQueryHit hit, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_rayCastAll(ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_rayCastAll0(ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_boxCastAll(ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_sphereCastAll(ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_capsuleCastAll(ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern PhysicsQueryHit[] Internal_convexCastAll(RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCastAny(ref Ray ray, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_rayCastAny0(ref Vector3 origin, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxCastAny(ref AABox box, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereCastAny(ref Sphere sphere, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleCastAny(ref Capsule capsule, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexCastAny(RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ref Vector3 unitDir, ulong layer, float max);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_boxOverlap(ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_sphereOverlap(ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_capsuleOverlap(ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Collider[] Internal_convexOverlap(RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_boxOverlapAny(ref AABox box, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_sphereOverlapAny(ref Sphere sphere, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_capsuleOverlapAny(ref Capsule capsule, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_convexOverlapAny(RRef<PhysicsMesh> mesh, ref Vector3 position, ref Quaternion rotation, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getGravity(out Vector3 __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setGravity(ref Vector3 gravity);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern uint Internal_addBroadPhaseRegion(ref AABox region);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_removeBroadPhaseRegion(uint handle);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_clearBroadPhaseRegions();
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_toggleCollision(ulong groupA, ulong groupB, bool enabled);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_isCollisionEnabled(ulong groupA, ulong groupB);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal__isUpdateInProgress();
	}

	/** @} */
}
