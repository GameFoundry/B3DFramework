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

	/** Properties that describe a specific type of ParticleEvolver. */
	struct ParticleEvolverProperties
	{
		ParticleEvolverProperties(bool analytical, INT32 priority)
			: analytical(analytical), priority(priority)
		{ }

		/** 
		 * True if the evolver can be evaluated analytically. This means the exact particle state can be retrieved based on 
		 * just the time value. Non-analytical (numerical) evolvers require the previous state of the particle and will 
		 * incrementally update the particle state.
		 */
		bool analytical;

		/** 
		 * Determines the order in which this evolver will be evaluated relative to other active evolvers. Higher values
		 * means that the evolver will be executed sooner. Negative values mean the evolver will be executed after
		 * position/velocity is integrated. 
		 */
		INT32 priority;
	};

	/** Updates properties of all active particles in a particle system in some way. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleEvolver : public ParticleModule
	{
	public:
		ParticleEvolver() = default;
		virtual ~ParticleEvolver() = default;

		/** Returns a set of properties that describe this evolver type. */
		virtual const ParticleEvolverProperties& getProperties() const = 0;
	protected:
		friend class ParticleSystem;

		/** 
		 * Updates properties of particles in the provided range according to the ruleset of the evolver. 
		 * 
		 * @param[in]	random			Utility class for generating random numbers.
		 * @param[in]	state			Particle system state for this frame.
		 * @param[in]	set				Set containing the particles to update.
		 * @param[in]	startIdx		Index of the first particle in @p set to update.
		 * @param[in]	count			Number of particles to update, starting from @p startIdx.
		 * @param[in]	spacing			When false all particles will use the same time-step as provided by @p state. If
		 *								true the time-step will be divided by @p count so particles are uniformly
		 *								distributed over the time-step.
		 * @param[in]	spacingOffset	Extra offset that controls the starting position of the first particle when
		 *								calculating spacing. Should be in range [0, 1). 0 = beginning of the current
		 *								time step, 1 = start of next particle.
		 */
		virtual void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const = 0;
	};

	/** Structure used for initializing a ParticleTextureAnimation object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleTextureAnimationOptions) PARTICLE_TEXTURE_ANIMATION_DESC
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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleTextureAnimation : public ParticleEvolver
	{
	public:
		ParticleTextureAnimation(const PARTICLE_TEXTURE_ANIMATION_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_TEXTURE_ANIMATION_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_TEXTURE_ANIMATION_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle texture animation evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleTextureAnimation> create(const PARTICLE_TEXTURE_ANIMATION_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

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
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleOrbitOptions) PARTICLE_ORBIT_DESC
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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleOrbit : public ParticleEvolver
	{
	public:
		ParticleOrbit(const PARTICLE_ORBIT_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_ORBIT_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_ORBIT_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle orbit evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleOrbit> create(const PARTICLE_ORBIT_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

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

	/** Structure used for initializing a ParticleVelocity object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleVelocityOptions) PARTICLE_VELOCITY_DESC
	{
		/** Determines the velocity of the particles evaluated over particle lifetime. */
		Vector3Distribution velocity = Vector3(0.0f, 1.0f, 0.0f);

		/** True if the velocity is provided in world space, false if in local space. */
		bool worldSpace = false;
	};

	/** Applies linear velocity to the particles. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleVelocity : public ParticleEvolver
	{
	public:
		ParticleVelocity(const PARTICLE_VELOCITY_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_VELOCITY_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_VELOCITY_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle velocity evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleVelocity> create(const PARTICLE_VELOCITY_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_VELOCITY_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleVelocity() = default; // RTTI only

		friend class ParticleVelocityRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleForce object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleForceOptions) PARTICLE_FORCE_DESC
	{
		/** Determines the force of the particles evaluated over particle lifetime. */
		Vector3Distribution force = Vector3(0.0f, 0.0f, 0.0f);

		/** True if the force is provided in world space, false if in local space. */
		bool worldSpace = false;
	};

	/** Applies an arbitrary force to the particles. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleForce : public ParticleEvolver
	{
	public:
		ParticleForce(const PARTICLE_FORCE_DESC&desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_FORCE_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_FORCE_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle force evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleForce> create(const PARTICLE_FORCE_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_FORCE_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleForce() = default; // RTTI only

		friend class ParticleForceRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleGravity object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleGravityOptions) PARTICLE_GRAVITY_DESC
	{
		/** Scale which to apply to the gravity value retrieved from the physics sub-system. */
		float scale = 1.0f;
	};

	/** Applies gravity to the particles. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleGravity : public ParticleEvolver
	{
	public:
		ParticleGravity(const PARTICLE_GRAVITY_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_GRAVITY_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_GRAVITY_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle gravity evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleGravity> create(const PARTICLE_GRAVITY_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_GRAVITY_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleGravity() = default;

		friend class ParticleGravityRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleColor object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleColorOptions) PARTICLE_COLOR_DESC
	{
		/** Determines the color of the particles evaluated over particle lifetime. */
		ColorDistribution color = Color::White;
	};

	/** Changes the color of the particles over the particle lifetime. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleColor : public ParticleEvolver
	{
	public:
		ParticleColor(const PARTICLE_COLOR_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_COLOR_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_COLOR_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle color evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleColor> create(const PARTICLE_COLOR_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_COLOR_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleColor() = default; // RTTI only

		friend class ParticleColorRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleSize object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleSizeOptions) PARTICLE_SIZE_DESC
	{
		/** 
		 * Determines the uniform size of the particles evaluated over particle lifetime. Only used if 3D size is disabled.
		 */
		FloatDistribution size = 1.0f;

		/** 
		 * Determines the non-uniform size of the particles evaluated over particle lifetime. Only used if 3D size is
		 * enabled.
		 */
		Vector3Distribution size3D = Vector3::ONE;

		/** 
		 * Determines should the size be evaluated uniformly for all dimensions, or evaluate each dimension with its own
		 * distribution.
		 */
		bool use3DSize = false;
	};

	/** Changes the size of the particles over the particle lifetime. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleSize : public ParticleEvolver
	{
	public:
		ParticleSize(const PARTICLE_SIZE_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_SIZE_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_SIZE_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle size evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleSize> create(const PARTICLE_SIZE_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_SIZE_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleSize() = default; // RTTI only

		friend class ParticleSizeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Structure used for initializing a ParticleRotation object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleRotationOptions) PARTICLE_ROTATION_DESC
	{
		/** 
		 * Determines the rotation of the particles in degrees, applied around the particle's local Z axis. Only used if 
		 * 3D rotation is disabled. 
		 */
		FloatDistribution rotation = 0.0f;

		/** Determines the rotation of the particles in degrees as Euler angles. Only used if 3D rotation is enabled. */
		Vector3Distribution rotation3D = Vector3::ZERO;

		/**
		 * Determines should the particle rotation be a single angle applied around a Z axis (if disabled), or a 
		 * set of Euler angles that allow you to rotate around every axis (if enabled).
		 */
		bool use3DRotation = false;
	};

	/** Rotates the particles over the particle lifetime. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleRotation : public ParticleEvolver
	{
	public:
		ParticleRotation(const PARTICLE_ROTATION_DESC& desc);

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_ROTATION_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_ROTATION_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(true, 0);
			return sProperties;
		}

		/** Creates a new particle rotation evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleRotation> create(const PARTICLE_ROTATION_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_ROTATION_DESC mDesc;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		ParticleRotation() = default; // RTTI only

		friend class ParticleRotationRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Types of collision modes that ParticleCollisions evolver can operate in. */
	enum class BS_SCRIPT_EXPORT(m:Particles) ParticleCollisionMode
	{
		/** Particles will collide with a user-provided set of planes. */
		Plane,

		/** Particles will collide with physics colliders in the scene. */
		World,
	};

	/** Structure used for initializing a ParticleCollisions object. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:ParticleCollisionsOptions) PARTICLE_COLLISIONS_DESC
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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleCollisions : public ParticleEvolver
	{
	public:
		ParticleCollisions(const PARTICLE_COLLISIONS_DESC& desc);

		/** 
		 * Determines a set of planes to use when using the Plane collision mode. Planes are expected to be in world 
		 * space. 
		 */
		BS_SCRIPT_EXPORT(pr:setter,n:Planes)
		void setPlanes(Vector<Plane> planes) { mCollisionPlanes = std::move(planes); }

		/** @copydoc setPlanes */
		BS_SCRIPT_EXPORT(pr:getter,n:Planes)
		const Vector<Plane>& getPlanes() const { return mCollisionPlanes; }

		/** 
		 * Determines a set of objects whose transforms to derive the collision planes from. Objects can move in the world
		 * and collision planes will be updated automatically. Object's negative Z axis is considered to be plane normal.
		 */
		BS_SCRIPT_EXPORT(pr:setter,n:PlaneObjects)
		void setPlaneObjects(Vector<HSceneObject> objects) { mCollisionPlaneObjects = std::move(objects); }

		/** @copydoc setPlaneObjects */
		BS_SCRIPT_EXPORT(pr:getter,n:PlaneObjects)
		const Vector<HSceneObject>& getPlaneObjects() const { return mCollisionPlaneObjects; }

		/** Options describing the evolver. */
		BS_SCRIPT_EXPORT(pr:setter,n:Options)
		void setOptions(const PARTICLE_COLLISIONS_DESC& options) { mDesc = options; }

		/** @copydoc setOptions */
		BS_SCRIPT_EXPORT(pr:getter,n:Options)
		const PARTICLE_COLLISIONS_DESC& getOptions() const { return mDesc; }

		/** @copydoc ParticleEvolver::getProperties */
		const ParticleEvolverProperties& getProperties() const override
		{
			static const ParticleEvolverProperties sProperties(false, -10000);
			return sProperties;
		}

		/** Creates a new particle collision evolver. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<ParticleCollisions> create(const PARTICLE_COLLISIONS_DESC& desc);
	private:
		/** @copydoc ParticleEvolver::evolve */
		void evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, UINT32 startIdx, 
			UINT32 count, bool spacing, float spacingOffset) const override;

		PARTICLE_COLLISIONS_DESC mDesc;

		Vector<Plane> mCollisionPlanes;
		Vector<HSceneObject> mCollisionPlaneObjects;

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
