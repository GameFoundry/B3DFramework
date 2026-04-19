//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Particles/B3DParticleModule.h"
#include "B3DParticleDistribution.h"

namespace b3d
{
	class Random;
	class ParticleSet;

	/** @addtogroup Particles
	 *  @{
	 */

	/** Properties that describe a specific type of ParticleEvolver. */
	struct ParticleEvolverProperties
	{
		ParticleEvolverProperties(bool analytical, i32 priority)
			: Analytical(analytical), Priority(priority)
		{}

		/**
		 * True if the evolver can be evaluated analytically. This means the exact particle state can be retrieved based on
		 * just the time value. Non-analytical (numerical) evolvers require the previous state of the particle and will
		 * incrementally update the particle state.
		 */
		bool Analytical;

		/**
		 * Determines the order in which this evolver will be evaluated relative to other active evolvers. Higher values
		 * means that the evolver will be executed sooner. Negative values mean the evolver will be executed after
		 * position/velocity is integrated.
		 */
		i32 Priority;
	};

	/** Updates properties of all active particles in a particle system in some way. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleEvolver : public ParticleModule
	{
	public:
		ParticleEvolver() = default;
		virtual ~ParticleEvolver() = default;

		/** Returns a set of properties that describe this evolver type. */
		virtual const ParticleEvolverProperties& GetProperties() const = 0;

	protected:
		friend class ParticleSystem;
		friend class ParticleScene;

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
		virtual void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const = 0;
	};

	/** Structure used for initializing a ParticleTextureAnimation object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleTextureAnimationSettings)) ParticleTextureAnimationSettings
	{
		/**
		 * Randomly pick a row to use for animation when the particle is first spawned. This implies that only a single row
		 * of the grid will be used for individual particle's animation.
		 */
		bool RandomizeRow = false;

		/** Number of cycles to loop the animation during particle's lifetime. */
		u32 CycleCount = 1;
	};

	/**
	 * Provides functionality for particle texture animation. Uses the sprite texture assigned to the particle's material
	 * to determine animation properties.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleTextureAnimation : public ParticleEvolver
	{
	public:
		ParticleTextureAnimation() = default;
		ParticleTextureAnimation(const ParticleTextureAnimationSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleTextureAnimationSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleTextureAnimationSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle texture animation evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleTextureAnimation> Create(const ParticleTextureAnimationSettings& settings);

		/** Creates a new particle texture animation evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleTextureAnimation> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleTextureAnimationSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleTextureAnimationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleOrbit object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleOrbitSettings)) ParticleOrbitSettings
	{
		/** Position of the center around which to orbit. Evaluated over particle system lifetime. */
		Vector3Distribution Center = Vector3(0.0f, 0.0f, 0.0f);

		/**
		 * Determines the speed of rotation around each axis. The speed is specified in "turns" where 0 = no rotation,
		 * 0.5 = 180 degree rotation and 1 = 360 degree rotation. Evaluated over particle lifetime.
		 */
		Vector3Distribution Velocity = Vector3(0.0f, 1.0f, 0.0f);

		/** Speed at which to push or pull the particles towards/away from the center. Evaluated over particle lifetime. */
		FloatDistribution Radial = 0.0f;

		/** True if the properties provided are in world space, false if in local space. */
		bool WorldSpace = false;
	};

	/** Moves particles so that their sprites orbit their center according to the provided offset and rotation values. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleOrbit : public ParticleEvolver
	{
	public:
		ParticleOrbit() = default;
		ParticleOrbit(const ParticleOrbitSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleOrbitSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleOrbitSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle orbit evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleOrbit> Create(const ParticleOrbitSettings& settings);

		/** Creates a new particle orbit evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleOrbit> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleOrbitSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleOrbitRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleVelocity object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleVelocitySettings)) ParticleVelocitySettings
	{
		/** Determines the velocity of the particles evaluated over particle lifetime. */
		Vector3Distribution Velocity = Vector3(0.0f, 1.0f, 0.0f);

		/** True if the velocity is provided in world space, false if in local space. */
		bool WorldSpace = false;
	};

	/** Applies linear velocity to the particles. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleVelocity : public ParticleEvolver
	{
	public:
		ParticleVelocity() = default;
		ParticleVelocity(const ParticleVelocitySettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleVelocitySettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleVelocitySettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle velocity evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleVelocity> Create(const ParticleVelocitySettings& settings);

		/** Creates a new particle velocity evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleVelocity> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleVelocitySettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleVelocityRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleForce object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleForceSettings)) ParticleForceSettings
	{
		/** Determines the force of the particles evaluated over particle lifetime. */
		Vector3Distribution Force = Vector3(0.0f, 0.0f, 0.0f);

		/** True if the force is provided in world space, false if in local space. */
		bool WorldSpace = false;
	};

	/** Applies an arbitrary force to the particles. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleForce : public ParticleEvolver
	{
	public:
		ParticleForce() = default;
		ParticleForce(const ParticleForceSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleForceSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleForceSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle force evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleForce> Create(const ParticleForceSettings& settings);

		/** Creates a new particle force evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleForce> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleForceSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleForceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleGravity object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleGravitySettings)) ParticleGravitySettings
	{
		/** Scale which to apply to the gravity value retrieved from the physics sub-system. */
		float Scale = 1.0f;
	};

	/** Applies gravity to the particles. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleGravity : public ParticleEvolver
	{
	public:
		ParticleGravity() = default;
		ParticleGravity(const ParticleGravitySettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleGravitySettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleGravitySettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle gravity evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleGravity> Create(const ParticleGravitySettings& settings);

		/** Creates a new particle gravity evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleGravity> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleGravitySettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleGravityRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleColor object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleColorOptions)) ParticleColorSettings
	{
		/** Determines the color of the particles evaluated over particle lifetime. */
		ColorDistribution Color = Color::kWhite;
	};

	/** Changes the color of the particles over the particle lifetime. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleColor : public ParticleEvolver
	{
	public:
		ParticleColor() = default; // RTTI only
		ParticleColor(const ParticleColorSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleColorSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleColorSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle color evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleColor> Create(const ParticleColorSettings& settings);

		/** Creates a new particle color evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleColor> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleColorSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleColorRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleSize object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleSizeSettings)) ParticleSizeSettings
	{
		/**
		 * Determines the uniform size of the particles evaluated over particle lifetime. Only used if 3D size is disabled.
		 */
		FloatDistribution Size = 1.0f;

		/**
		 * Determines the non-uniform size of the particles evaluated over particle lifetime. Only used if 3D size is
		 * enabled.
		 */
		Vector3Distribution Size3D = Vector3::kOne;

		/**
		 * Determines should the size be evaluated uniformly for all dimensions, or evaluate each dimension with its own
		 * distribution.
		 */
		bool Use3DSize = false;
	};

	/** Changes the size of the particles over the particle lifetime. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSize : public ParticleEvolver
	{
	public:
		ParticleSize() = default;
		ParticleSize(const ParticleSizeSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleSizeSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleSizeSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle size evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleSize> Create(const ParticleSizeSettings& settings);

		/** Creates a new particle size evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleSize> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleSizeSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSizeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Structure used for initializing a ParticleRotation object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleRotationSettings)) ParticleRotationSettings
	{
		/**
		 * Determines the rotation of the particles in degrees, applied around the particle's local Z axis. Only used if
		 * 3D rotation is disabled.
		 */
		FloatDistribution Rotation = 0.0f;

		/** Determines the rotation of the particles in degrees as Euler angles. Only used if 3D rotation is enabled. */
		Vector3Distribution Rotation3D = Vector3::kZero;

		/**
		 * Determines should the particle rotation be a single angle applied around a Z axis (if disabled), or a
		 * set of Euler angles that allow you to rotate around every axis (if enabled).
		 */
		bool Use3DRotation = false;
	};

	/** Rotates the particles over the particle lifetime. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleRotation : public ParticleEvolver
	{
	public:
		ParticleRotation() = default;
		ParticleRotation(const ParticleRotationSettings& settings);

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleRotationSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleRotationSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(true, 0);
			return kSProperties;
		}

		/** Creates a new particle rotation evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleRotation> Create(const ParticleRotationSettings& settings);

		/** Creates a new particle rotation evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleRotation> Create();

	private:
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleRotationSettings mSettings;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleRotationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Types of collision modes that ParticleCollisions evolver can operate in. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleCollisionMode
	{
		/** Particles will collide with a user-provided set of planes. */
		Plane,

		/** Particles will collide with physics colliders in the scene. */
		World,
	};

	/** Structure used for initializing a ParticleCollisions object. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(ParticleCollisionsSettings)) ParticleCollisionSettings
	{
		/** Collision mode determining with which geometry the particles will interact with. */
		ParticleCollisionMode Mode = ParticleCollisionMode::Plane;

		/**
		 * Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy
		 * and higher values more.
		 */
		float Restitution = 1.0f;

		/**
		 * Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In
		 * range [0, 1].
		 */
		float Dampening = 0.5f;

		/**
		 * Determines how much should the particle lifetime be reduced after a collision, in percent of its original
		 * lifetime. In range [0, 1].
		 */
		float LifetimeLoss = 0.0f;

		/** Radius of every individual particle used for collisions, in meters. */
		float Radius = 0.01f;

		/**
		 * Physics layers that determine which objects will particle collide with. Only relevant when using the World
		 * collision mode.
		 */
		u64 Layer = 0xFFFFFFFFFFFFFFFF;
	};

	/** Particle evolver that allows particles to collide with the world. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleCollisions : public ParticleEvolver
	{
	public:
		ParticleCollisions() = default;
		ParticleCollisions(const ParticleCollisionSettings& settings);

		/**
		 * Determines a set of planes to use when using the Plane collision mode. Planes are expected to be in world
		 * space.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Planes))
		void SetPlanes(Vector<Plane> planes) { mCollisionPlanes = std::move(planes); }

		/** @copydoc SetPlanes */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Planes))
		const Vector<Plane>& GetPlanes() const { return mCollisionPlanes; }

		/**
		 * Determines a set of objects whose transforms to derive the collision planes from. Objects can move in the world
		 * and collision planes will be updated automatically. Object's negative Z axis is considered to be plane normal.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(PlaneObjects))
		void SetPlaneObjects(Vector<HSceneObject> objects) { mCollisionPlaneObjects = std::move(objects); }

		/** @copydoc SetPlaneObjects */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(PlaneObjects))
		const Vector<HSceneObject>& GetPlaneObjects() const { return mCollisionPlaneObjects; }

		/** Options describing the evolver. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings))
		void SetSettings(const ParticleCollisionSettings& settings) { mSettings = settings; }

		/** @copydoc SetSettings */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings))
		const ParticleCollisionSettings& GetSettings() const { return mSettings; }

		const ParticleEvolverProperties& GetProperties() const override
		{
			static const ParticleEvolverProperties kSProperties(false, -10000);
			return kSProperties;
		}

		/** Creates a new particle collision evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleCollisions> Create(const ParticleCollisionSettings& settings);

		/** Creates a new particle collision evolver. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static SPtr<ParticleCollisions> Create();

	private:
		/** @copydoc ParticleEvolver::Evolve */
		void Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const override;

		ParticleCollisionSettings mSettings;

		Vector<Plane> mCollisionPlanes;
		Vector<HSceneObject> mCollisionPlaneObjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleCollisionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
