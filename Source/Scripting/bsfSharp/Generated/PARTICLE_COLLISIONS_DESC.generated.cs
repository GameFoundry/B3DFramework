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

	/// <summary>Structure used for initializing a ParticleCollisions object.</summary>
	[StructLayout(LayoutKind.Sequential), SerializeObject]
	public partial struct ParticleCollisionsOptions
	{
		/// <summary>Initializes the struct with default values.</summary>
		public static ParticleCollisionsOptions Default()
		{
			ParticleCollisionsOptions value = new ParticleCollisionsOptions();
			value.Mode = ParticleCollisionMode.Plane;
			value.Restitution = 1f;
			value.Dampening = 0.5f;
			value.LifetimeLoss = 0f;
			value.Radius = 0.00999999977f;
			value.Layer = 18446744073709551615;

			return value;
		}

		/// <summary>Collision mode determining with which geometry the particles will interact with.</summary>
		public ParticleCollisionMode Mode;
		/// <summary>
		/// Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy and 
		/// higher values more.
		/// </summary>
		public float Restitution;
		/// <summary>
		/// Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In range 
		/// [0, 1].
		/// </summary>
		public float Dampening;
		/// <summary>
		/// Determines how much should the particle lifetime be reduced after a collision, in percent of its original lifetime. 
		/// In range [0, 1].
		/// </summary>
		public float LifetimeLoss;
		/// <summary>Radius of every individual particle used for collisions, in meters.</summary>
		public float Radius;
		/// <summary>
		/// Physics layers that determine which objects will particle collide with. Only relevant when using the World collision 
		/// mode.
		/// </summary>
		public ulong Layer;
	}

	/** @} */
}
