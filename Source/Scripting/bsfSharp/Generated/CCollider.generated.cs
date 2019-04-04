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
	/// Collider represents physics geometry that can be in multiple states: - Default: Static geometry that physics objects 
	/// can collide with. - Trigger: Static geometry that can&apos;t be collided with but will report touch events. - Dynamic: 
	/// Dynamic geometry that is a part of a Rigidbody. A set of colliders defines the shape of the parent  rigidbody.
	/// </summary>
	[ShowInInspector]
	public partial class Collider : Component
	{
		private Collider(bool __dummy0) { }
		protected Collider() { }

		/// <summary>
		/// Enables/disables a collider as a trigger. A trigger will not be used for collisions (objects will pass  through it), 
		/// but collision events will still be reported.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public bool Trigger
		{
			get { return Internal_getIsTrigger(mCachedPtr); }
			set { Internal_setIsTrigger(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the mass of the collider. Only relevant if the collider is part of a rigidbody. Ultimately this will 
		/// determine the total mass, center of mass and inertia tensors of the parent rigidbody (if they&apos;re being 
		/// calculated automatically).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Mass
		{
			get { return Internal_getMass(mCachedPtr); }
			set { Internal_setMass(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines the physical material of the collider. The material determines how objects hitting the collider  behave.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public RRef<PhysicsMaterial> Material
		{
			get { return Internal_getMaterial(mCachedPtr); }
			set { Internal_setMaterial(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines how far apart do two shapes need to be away from each other before the physics runtime starts  generating 
		/// repelling impulse for them. This distance will be the sum of contact offsets of the two interacting objects. If 
		/// objects are moving fast you can increase this value to start generating the impulse earlier and  potentially prevent 
		/// the objects from interpenetrating. This value is in meters. Must be positive and greater than rest offset.
		///
		/// Also see setRestOffset().
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float ContactOffset
		{
			get { return Internal_getContactOffset(mCachedPtr); }
			set { Internal_setContactOffset(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines at what distance should two objects resting on one another come to an equilibrium. The value used in the 
		/// runtime will be the sum of rest offsets for both interacting objects. This value is in meters. Cannot be  larger than 
		/// contact offset.
		///
		/// Also see setContactOffset().
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float RestOffset
		{
			get { return Internal_getRestOffset(mCachedPtr); }
			set { Internal_setRestOffset(mCachedPtr, value); }
		}

		/// <summary>Determines the layer of the collider. Layer controls with which objects will the collider collide.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ulong Layer
		{
			get { return Internal_getLayer(mCachedPtr); }
			set { Internal_setLayer(mCachedPtr, value); }
		}

		/// <summary>Determines which (if any) collision events are reported.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public CollisionReportMode CollisionReportMode
		{
			get { return Internal_getCollisionReportMode(mCachedPtr); }
			set { Internal_setCollisionReportMode(mCachedPtr, value); }
		}

		/// <summary>
		/// Triggered when some object starts interacting with the collider. Only triggered if proper collision report mode is 
		/// turned on.
		/// </summary>
		public event Action<CollisionData> OnCollisionBegin;

		/// <summary>
		/// Triggered for every frame that an object remains interacting with a collider. Only triggered if proper collision 
		/// report mode is turned on.
		/// </summary>
		public event Action<CollisionData> OnCollisionStay;

		/// <summary>
		/// Triggered when some object stops interacting with the collider. Only triggered if proper collision report mode is 
		/// turned on.
		/// </summary>
		public event Action<CollisionData> OnCollisionEnd;

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setIsTrigger(IntPtr thisPtr, bool value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern bool Internal_getIsTrigger(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMass(IntPtr thisPtr, float mass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getMass(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setMaterial(IntPtr thisPtr, RRef<PhysicsMaterial> material);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<PhysicsMaterial> Internal_getMaterial(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setContactOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getContactOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRestOffset(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRestOffset(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setLayer(IntPtr thisPtr, ulong layer);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern ulong Internal_getLayer(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setCollisionReportMode(IntPtr thisPtr, CollisionReportMode mode);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern CollisionReportMode Internal_getCollisionReportMode(IntPtr thisPtr);
		private void Internal_onCollisionBegin(ref CollisionData p0)
		{
			OnCollisionBegin?.Invoke(p0);
		}
		private void Internal_onCollisionStay(ref CollisionData p0)
		{
			OnCollisionStay?.Invoke(p0);
		}
		private void Internal_onCollisionEnd(ref CollisionData p0)
		{
			OnCollisionEnd?.Invoke(p0);
		}
	}

	/** @} */
}
