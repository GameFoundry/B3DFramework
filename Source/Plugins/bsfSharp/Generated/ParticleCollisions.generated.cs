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

	/// <summary>Particle evolver that allows particles to collide with the world.</summary>
	[ShowInInspector]
	public partial class ParticleCollisions : ParticleEvolver
	{
		private ParticleCollisions(bool __dummy0) { }

		/// <summary>Creates a new particle collision evolver.</summary>
		public ParticleCollisions(ParticleCollisionsOptions desc)
		{
			Internal_create(this, ref desc);
		}

		/// <summary>Creates a new particle collision evolver.</summary>
		public ParticleCollisions()
		{
			Internal_create0(this);
		}

		/// <summary>
		/// Determines a set of planes to use when using the Plane collision mode. Planes are expected to be in world  space.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Plane[] Planes
		{
			get { return Internal_getPlanes(mCachedPtr); }
			set { Internal_setPlanes(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a set of objects whose transforms to derive the collision planes from. Objects can move in the world and 
		/// collision planes will be updated automatically. Object's negative Z axis is considered to be plane normal.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public SceneObject[] PlaneObjects
		{
			get { return Internal_getPlaneObjects(mCachedPtr); }
			set { Internal_setPlaneObjects(mCachedPtr, value); }
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleCollisionsOptions Options
		{
			get
			{
				ParticleCollisionsOptions temp;
				Internal_getOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_setOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPlanes(IntPtr thisPtr, Plane[] planes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Plane[] Internal_getPlanes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setPlaneObjects(IntPtr thisPtr, SceneObject[] objects);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject[] Internal_getPlaneObjects(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_setOptions(IntPtr thisPtr, ref ParticleCollisionsOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_getOptions(IntPtr thisPtr, out ParticleCollisionsOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create(ParticleCollisions managedInstance, ref ParticleCollisionsOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_create0(ParticleCollisions managedInstance);
	}

	/** @} */
}
