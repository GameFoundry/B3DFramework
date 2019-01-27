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
	/// Material that controls how two physical objects interact with each other. Materials of both objects are used during 
	/// their interaction and their combined values are used.
	/// </summary>
	[ShowInInspector]
	public partial class PhysicsMaterial : Resource
	{
		private PhysicsMaterial(bool __dummy0) { }
		protected PhysicsMaterial() { }

		/// <summary>Creates a new physics material.</summary>
		/// <param name="staticFriction">
		/// Controls friction when two in-contact objects are not moving lateral to each other (for example how difficult is to 
		/// get an object moving from a static state while it is in contact other object(s)).
		/// </param>
		/// <param name="dynamicFriction">
		/// Sets dynamic friction of the material. Controls friction when two in-contact objects are moving lateral to each other 
		/// (for example how quickly does an object slow down when sliding along another object).
		/// </param>
		/// <param name="restitution">
		/// Controls "bounciness" of an object during a collision. Value of 1 means the collision is elastic, and value of 0 
		/// means the value is inelastic. Must be in [0, 1] range.
		/// </param>
		public PhysicsMaterial(float staticFriction = 0f, float dynamicFriction = 0f, float restitution = 0f)
		{
			Internal_create(this, staticFriction, dynamicFriction, restitution);
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public RRef<PhysicsMaterial> Ref
		{
			get { return Internal_GetRef(mCachedPtr); }
		}

		/// <summary>
		/// Controls friction when two in-contact objects are not moving lateral to each other (for example how difficult  it is 
		/// to get an object moving from a static state while it is in contact with other object(s)).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float StaticFriction
		{
			get { return Internal_getStaticFriction(mCachedPtr); }
			set { Internal_setStaticFriction(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls friction when two in-contact objects are moving lateral to each other (for example how quickly does an 
		/// object slow down when sliding along another object).
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float DynamicFriction
		{
			get { return Internal_getDynamicFriction(mCachedPtr); }
			set { Internal_setDynamicFriction(mCachedPtr, value); }
		}

		/// <summary>
		/// Controls "bounciness" of an object during a collision. Value of 1 means the collision is elastic, and value of 0  
		/// means the value is inelastic. Must be in [0, 1] range.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public float Restitution
		{
			get { return Internal_getRestitutionCoefficient(mCachedPtr); }
			set { Internal_setRestitutionCoefficient(mCachedPtr, value); }
		}

		/// <summary>Returns a reference wrapper for this resource.</summary>
		public static implicit operator RRef<PhysicsMaterial>(PhysicsMaterial x)
		{ return Internal_GetRef(x.mCachedPtr); }

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern RRef<PhysicsMaterial> Internal_GetRef(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setStaticFriction(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getStaticFriction(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setDynamicFriction(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getDynamicFriction(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setRestitutionCoefficient(IntPtr thisPtr, float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern float Internal_getRestitutionCoefficient(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(PhysicsMaterial managedInstance, float staticFriction, float dynamicFriction, float restitution);
	}

	/** @} */
}
