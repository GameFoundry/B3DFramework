//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Particles/BsParticleModule.h"
#include "BsParticleDistribution.h"

namespace bs
{
	class Random;
	class ParticleSet;

	/** @addtogroup Particles
	 *  @{
	 */

	/** Updates properties of all active particles in a particle system in some way. */
	class BS_CORE_EXPORT ParticleEvolver : public ParticleModule
	{
	public:
		ParticleEvolver() = default;
		virtual ~ParticleEvolver() = default;

		/** Updates properties of all particles in the @p set according to the ruleset of the evolver. */
		virtual void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const = 0;

		/** 
		 * Returns true if the evolver can be evaluated analytically. This means the exact particle state can be retrieved
		 * based on just the time value. Non-analytical (numerical) evolvers require the previous state of the particle
		 * and will incrementally update the particle state.
		 */
		virtual bool isAnalytical() const = 0;

		/** 
		 * Determines the order in which this evolver will be evaluated relative to other active evolvers. Higher values
		 * means that the evolver will be executed sooner. Negative values mean the evolver will be executed after
		 * position/velocity is integrated. 
		 */
		virtual INT32 getPriority() const { return 0; }
	};

	/** Structure used for initializing a ParticleTextureAnimation object. */
	struct PARTICLE_TEXTURE_ANIMATION_DESC
	{
		/**
		 * Randomly pick a row to use for animation when the particle is first spawned. This implies that only a single row
		 * of the grid will be used for individual particle's animation.
		 */
		bool randomizeRow = false;

		/** Number of cycles to loop the animation during particle's lifetime. */
		UINT32 numCycles = 1;
	};

	/** 
	 * Provides functionality for particle texture animation. Uses the sprite texture assigned to the particle's material
	 * to determine animation properties.
	 */
	class BS_CORE_EXPORT ParticleTextureAnimation : public ParticleEvolver
	{
	public:
		ParticleTextureAnimation(const PARTICLE_TEXTURE_ANIMATION_DESC& desc);

		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const override;

		/** @copydoc ParticleEvolver::isAnalytical */
		bool isAnalytical() const override { return true; }
	private:
		PARTICLE_TEXTURE_ANIMATION_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleTextureAnimation() = default; // RTTI only

		friend class ParticleTextureAnimationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleOrbit object. */
	struct PARTICLE_ORBIT_DESC
	{
		/** Position of the center around which to orbit. Evaluated over particle system lifetime. */
		Vector3Distribution center = Vector3(0.0f, 0.0f, 0.0f);

		/** 
		 * Determines the speed of rotation around each axis. The speed is specified in "turns" where 0 = no rotation, 
		 * 0.5 = 180 degree rotation and 1 = 360 degree rotation. Evaluated over particle lifetime.
		 */
		Vector3Distribution velocity = Vector3(0.0f, 1.0f, 0.0f);

		/** Speed at which to push or pull the particles towards/away from the center. Evaluated over particle lifetime. */
		FloatDistribution radial = 0.0f;

		/** True if the properties provided are in world space, false if in local space. */
		bool worldSpace = false;
	};

	/** Moves particles so that their sprites orbit their center according to the provided offset and rotation values. */
	class BS_CORE_EXPORT ParticleOrbit : public ParticleEvolver
	{
	public:
		ParticleOrbit(const PARTICLE_ORBIT_DESC&desc);

		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const override;

		/** @copydoc ParticleEvolver::isAnalytical */
		bool isAnalytical() const override { return true; }
	private:
		PARTICLE_ORBIT_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleOrbit() = default; // RTTI only

		friend class ParticleOrbitRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Types of collision modes that ParticleCollisions evolver can operate in. */
	enum class ParticleCollisionMode
	{
		/** Particles will collide with a user-provided set of planes. */
		Plane,

		/** Particles will collide with physics colliders in the scene. */
		World,
	};

	/** Structure used for initializing a ParticleCollisions object. */
	struct PARTICLE_COLLISONS_DESC
	{
		/** Collision mode determining with which geometry the particles will interact with. */
		ParticleCollisionMode mode = ParticleCollisionMode::Plane;

		/** 
		 * Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy
		 * and higher values more. 
		 */
		float restitution = 1.0f;

		/**
		 * Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In
		 * range [0, 1].
		 */
		float dampening = 0.5f;

		/**
		 * Determines how much should the particle lifetime be reduced after a collision, in percent of its original
		 * lifetime. In range [0, 1].
		 */
		float lifetimeLoss = 0.0f;

		/** Radius of every individual particle used for collisions, in meters. */
		float radius = 0.01f;

		/** 
		 * Physics layers that determine which objects will particle collide with. Only relevant when using the World
		 * collision mode.
		 */
		UINT64 layer = 0xFFFFFFFFFFFFFFFF;
	};

	/** Particle evolver that allows particles to collide with the world. */
	class BS_CORE_EXPORT ParticleCollisions : public ParticleEvolver
	{
	public:
		ParticleCollisions(const PARTICLE_COLLISONS_DESC& desc);

		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const override;

		/** @copydoc ParticleEvolver::isAnalytical */
		bool isAnalytical() const override { return false; }

		/** @copydoc ParticleEvolver::getPriority */
		INT32 getPriority() const override { return -10000; }

		/** 
		 * Determines a set of planes to use when using the Plane collision mode. Planes are expected to be in world 
		 * space. 
		 */
		void setPlanes(Vector<Plane> planes) { mCollisionPlanes = std::move(planes); }

	private:
		PARTICLE_COLLISONS_DESC mDesc;

		Vector<Plane> mCollisionPlanes;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleCollisions() = default; // RTTI only

		friend class ParticleCollisionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
