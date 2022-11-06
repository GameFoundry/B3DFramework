//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
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
			Internal_Create(this, ref desc);
		}

		/// <summary>Creates a new particle collision evolver.</summary>
		public ParticleCollisions()
		{
			Internal_Create0(this);
		}

		/// <summary>
		/// Determines a set of planes to use when using the Plane collision mode. Planes are expected to be in world space.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public Plane[] Planes
		{
			get { return Internal_GetPlanes(mCachedPtr); }
			set { Internal_SetPlanes(mCachedPtr, value); }
		}

		/// <summary>
		/// Determines a set of objects whose transforms to derive the collision planes from. Objects can move in the world and 
		/// collision planes will be updated automatically. Object&apos;s negative Z axis is considered to be plane normal.
		/// </summary>
		[ShowInInspector]
		[NativeWrapper]
		public SceneObject[] PlaneObjects
		{
			get { return Internal_GetPlaneObjects(mCachedPtr); }
			set { Internal_SetPlaneObjects(mCachedPtr, value); }
		}

		/// <summary>Options describing the evolver.</summary>
		[ShowInInspector]
		[NativeWrapper]
		public ParticleCollisionsOptions Options
		{
			get
			{
				ParticleCollisionsOptions temp;
				Internal_GetOptions(mCachedPtr, out temp);
				return temp;
			}
			set { Internal_SetOptions(mCachedPtr, ref value); }
		}

		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPlanes(IntPtr thisPtr, Plane[] planes);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern Plane[] Internal_GetPlanes(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetPlaneObjects(IntPtr thisPtr, SceneObject[] objects);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern SceneObject[] Internal_GetPlaneObjects(IntPtr thisPtr);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_SetOptions(IntPtr thisPtr, ref ParticleCollisionsOptions options);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_GetOptions(IntPtr thisPtr, out ParticleCollisionsOptions __output);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create(ParticleCollisions managedInstance, ref ParticleCollisionsOptions desc);
		[MethodImpl(MethodImplOptions.InternalCall)]
		private static extern void Internal_Create0(ParticleCollisions managedInstance);
	}

	/** @} */
}
