//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Math/BsVector3.h"
#include "Math/BsRandom.h"
#include "Scene/BsSceneActor.h"
#include "CoreThread/BsCoreObject.h"
#include "Image/BsPixelData.h"
#include "Math/BsAABox.h"
#include "Particles/BsParticleDistribution.h"
#include "Particles/BsParticleEvolver.h"
#include "Particles/BsParticleEmitter.h"

namespace bs
{
	struct EvaluatedAnimationData;
	class SkeletonMask;
	class ParticleSystem;
	class ParticleEmitter;
	class ParticleEvolver;
	class ParticleSet;

	namespace ct
	{
		class ParticleSystem;
	}

	/** @addtogroup Implementation
	 *  @{
	 */

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Possible orientations when rendering billboard particles. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleOrientation
	{
		/** Orient towards view (camera) plane. */
		ViewPlane,

		/** Orient towards view (camera) position. */
		ViewPosition,

		/** Orient with a user-provided axis. */
		Plane
	};

	/** Space in which to spawn/transform particles. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSimulationSpace
	{
		/**
		 * Particles will always remain local to their transform parent. This means if the transform parent moves so will
		 * all the particles.
		 */
		Local,

		/**
		 * Particles will be placed in world space. This means they will spawn at the location of the transform parent,
		 * but are no longer affected by its transform after spawn (e.g. smoke rising from a moving train).
		 */
		World
	};

	/** Determines how to sort particles before rendering. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSortMode
	{
		/** Do not sort the particles. */
		None,

		/** Sort by distance from the camera, furthest to nearest. */
		Distance,

		/** Sort by age, oldest to youngest. */
		OldToYoung,

		/** Sort by age, youngest to oldest. */
		YoungToOld
	};

	/** Determines how are particles represented on the screen. */
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleRenderMode
	{
		/** Particle is represented using a 2D quad. */
		Billboard,

		/** Particle is represented using a 3D mesh. */
		Mesh
	};

	/** Controls depth buffer collisions for GPU simulated particles. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleDepthCollisionSettings : IReflectable
	{
		BS_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings() = default;

		/** Determines if depth collisions are enabled. */
		BS_SCRIPT_EXPORT()
		bool Enabled = false;

		/**
		 * Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy
		 * and higher values more.
		 */
		BS_SCRIPT_EXPORT()
		float Restitution = 1.0f;

		/**
		 * Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In
		 * range [0, 1].
		 */
		BS_SCRIPT_EXPORT()
		float Dampening = 0.5f;

		/** Scale which to apply to particle size in order to determine the collision radius. */
		BS_SCRIPT_EXPORT()
		float RadiusScale = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P p);

	public:
		friend class ParticleDepthCollisonSettingsRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common base for both sim and core thread variants of ParticleSystemSettings. */
	struct ParticleSystemSettingsBase
	{
		/** Determines in which space are particles in. */
		BS_SCRIPT_EXPORT()
		ParticleSimulationSpace SimulationSpace = ParticleSimulationSpace::World;

		/** Determines how are particles oriented when rendering. */
		BS_SCRIPT_EXPORT()
		ParticleOrientation Orientation = ParticleOrientation::ViewPlane;

		/**
		 * Determines the time period during which the system runs, in seconds. This effects evaluation of distributions
		 * with curves using particle system time for evaluation.
		 */
		BS_SCRIPT_EXPORT()
		float Duration = 5.0f;

		/** Determines should the particle system time wrap around once it reaches its duration. */
		BS_SCRIPT_EXPORT()
		bool IsLooping = true;

		/**
		 * Determines the maximum number of particles that can ever be active in this system. This number is ignored
		 * if GPU simulation is enabled, and instead particle count is instead only limited by the size of the internal
		 * buffers (shared between all particle systems).
		 */
		BS_SCRIPT_EXPORT()
		u32 MaxParticles = 2000;

		/**
		 * If true the particle system will be simulated on the GPU. This allows much higher particle counts at lower
		 * performance cost. GPU simulation ignores any provided evolvers and instead uses ParticleGpuSimulationSettings
		 * to customize the GPU simulation.
		 */
		BS_SCRIPT_EXPORT(UICategory(Advanced), UIOrder(1))
		bool GpuSimulation = false;

		/** Determines how is each particle represented on the screen. */
		BS_SCRIPT_EXPORT(UIOrder(2))
		ParticleRenderMode RenderMode = ParticleRenderMode::Billboard;

		/**
		 * Determines should the particles only be allowed to orient themselves around the Y axis, or freely. Ignored if
		 * using the Plane orientation mode.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		bool OrientationLockY = false;

		/**
		 * Determines a normal of the plane to orient particles towards. Only used if particle orientation mode is set to
		 * ParticleOrientation::Plane.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		Vector3 OrientationPlaneNormal = Vector3::kUnitZ;

		/**
		 * Determines how (and if) are particles sorted. Sorting controls in what order are particles rendered.
		 * If GPU simulation is enabled only distance based sorting is supported.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		ParticleSortMode SortMode = ParticleSortMode::None;

		/**
		 * Determines should an automatic seed be used for the internal random number generator. This ensures the particle
		 * system yields different results each time it is ran.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		bool UseAutomaticSeed = true;

		/**
		 * Determines the seed to use for the internal random number generator. Allows you to guarantee identical behaviour
		 * between different runs. Only relevant if automatic seed is disabled.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		u32 ManualSeed = 0;

		/**
		 * Determines should the particle system bounds be automatically calculated, or should the fixed value provided
		 * be used. Bounds are used primarily for culling purposes. Note that automatic bounds are not supported when GPU
		 * simulation is enabled.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		bool UseAutomaticBounds = true;

		/**
		 * Custom bounds to use them @p useAutomaticBounds is disabled. The bounds are in the simulation space of the
		 * particle system.
		 */
		BS_SCRIPT_EXPORT(UIOrder(2))
		AABox CustomBounds;
	};

	/** Templated common base for both sim and core thread variants of ParticleSystemSettings. */
	template <bool Core>
	struct TParticleSystemSettings : ParticleSystemSettingsBase
	{
		using MaterialType = CoreVariantHandleType<Material, Core>;
		using MeshType = CoreVariantHandleType<Mesh, Core>;

		/** Material to render the particles with. */
		BS_SCRIPT_EXPORT(LoadOnAssign(true))
		MaterialType Material;

		/** Mesh used for representing individual particles when using the Mesh rendering mode. */
		BS_SCRIPT_EXPORT(LoadOnAssign(true), UIOrder(2))
		MeshType Mesh;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P processor);
	};

	/** Common base for both sim and core thread variants of ParticleVectorFieldSettings. */
	struct ParticleVectorFieldSettingsBase
	{
		/** Intensity of the forces and velocities applied by the vector field. */
		BS_SCRIPT_EXPORT()
		float Intensity = 1.0f;

		/**
		 * Determines how closely does the particle velocity follow the vectors in the field. If set to 1 particles
		 * will be snapped to the exact velocity of the value in the field, and if set to 0 the field will not influence
		 * particle velocities directly.
		 */
		BS_SCRIPT_EXPORT()
		float Tightness = 0.0f;

		/** Scale to apply to the vector field bounds. This is multiplied with the bounds of the vector field resource. */
		BS_SCRIPT_EXPORT()
		Vector3 Scale = Vector3::kOne;

		/**
		 * Amount of to move the vector field by relative to the parent particle system. This is added to the bounds
		 * provided in the vector field resource.
		 */
		BS_SCRIPT_EXPORT()
		Vector3 Offset = Vector3::kZero;

		/** Initial rotation of the vector field. */
		BS_SCRIPT_EXPORT()
		Quaternion Rotation = Quaternion::kIdentity;

		/**
		 * Determines the amount to rotate the vector field every second, in degrees, around XYZ axis respectively.
		 * Evaluated over the particle system lifetime.
		 */
		BS_SCRIPT_EXPORT()
		Vector3Distribution RotationRate = Vector3(0.0f, 90.0f, 0.0f);

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the X direction.
		 */
		BS_SCRIPT_EXPORT()
		bool TilingX = false;

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Y direction.
		 */
		BS_SCRIPT_EXPORT()
		bool TilingY = false;

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Z direction.
		 */
		BS_SCRIPT_EXPORT()
		bool TilingZ = false;
	};

	/** Templated common base for both sim and core thread variants of ParticleVectorFieldSettings. */
	template <bool Core>
	struct TParticleVectorFieldSettings : ParticleVectorFieldSettingsBase
	{
		/** Vector field resource used for influencing the particles. */
		BS_SCRIPT_EXPORT()
		CoreVariantHandleType<VectorField, Core> VectorField;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P processor);
	};

	/** @} */
	/** @addtogroup Particles
	 *  @{
	 */

	/** Settings used for controlling a vector field in a GPU simulated particle system. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles))
		ParticleVectorFieldSettings : TParticleVectorFieldSettings<false>,
									  IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleVectorFieldSettingsRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	namespace ct
	{
		/** Core thread counterpart of bs::ParticleVectorFieldSettings. */
		struct ParticleVectorFieldSettings : TParticleVectorFieldSettings<true>
		{};
	} // namespace ct

	/** @} */
	/** @addtogroup Implementation
	 *  @{
	 */

	template <>
	struct CoreThreadType<ParticleVectorFieldSettings>
	{
		typedef ct::ParticleVectorFieldSettings Type;
	};

	/** Common base for both sim and core threat variants of ParticleGpuSimulationSettings. */
	struct ParticleGpuSimulationSettingsBase
	{
		/** Determines particle color, evaluated over the particle lifetime. */
		BS_SCRIPT_EXPORT()
		ColorDistribution ColorOverLifetime = Color::kWhite;

		/** Determines particle size, evaluated over the particle lifetime. Multiplied by the initial particle size. */
		BS_SCRIPT_EXPORT()
		Vector2Distribution SizeScaleOverLifetime = Vector2::kOne;

		/** Constant acceleration to apply for each step of the simulation. */
		BS_SCRIPT_EXPORT()
		Vector3 Acceleration = Vector3::kZero;

		/** Amount of resistance to apply in the direction opposite of the particle's velocity. */
		BS_SCRIPT_EXPORT()
		float Drag = 0.0f;

		/** Settings controlling particle depth buffer collisions. */
		BS_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings DepthCollision;
	};

	/** Templated common base for both sim and core threat variants of ParticleGpuSimulationSettings. */
	template <bool Core>
	struct TParticleGpuSimulationSettings : ParticleGpuSimulationSettingsBase
	{
		BS_SCRIPT_EXPORT()
		CoreVariantType<ParticleVectorFieldSettings, Core> VectorField;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template <class P>
		void RttiEnumFields(P processor);
	};

	/** @} */
	/** @addtogroup Particles
	 *  @{
	 */

	/** Generic settings used for controlling a ParticleSystem. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles))
		ParticleSystemSettings : TParticleSystemSettings<false>,
								 IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemSettingsRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** Settings used for controlling particle system GPU simulation. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Particles))
		ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<false>,
										IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleGpuSimulationSettingsRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	namespace ct
	{
		class VectorField;

		/** Core thread counterpart of bs::ParticleSystemSettings. */
		struct ParticleSystemSettings : TParticleSystemSettings<true>
		{};

		/** Core thread counterpart of bs::ParticleVectorFieldSettings. */
		struct ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<true>
		{};
	} // namespace ct

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/**
	 * Controls spawning, evolution and rendering of particles. Particles can be 2D or 3D, with a variety of rendering
	 * options. Particle system should be used for rendering objects that cannot properly be represented using static or
	 * animated meshes, like liquids, smoke or flames.
	 *
	 * The particle system requires you to specify at least one ParticleEmitter, which controls how are new particles
	 * generated. You will also want to specify one or more ParticleEvolver%s, which change particle properties over time.
	 */
	class BS_CORE_EXPORT ParticleSystem final : public IReflectable, public CoreObject, public SceneActor, public INonCopyable
	{
	public:
		~ParticleSystem() final;

		/** Determines general purpose settings that apply to the particle system. */
		void SetSettings(const ParticleSystemSettings& settings);

		/** @copydoc setSettings */
		const ParticleSystemSettings& GetSettings() const { return mSettings; }

		/** Determines settings that control particle GPU simulation. */
		void SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc setGpuSimulationSettings */
		const ParticleGpuSimulationSettings& GetGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/**
		 * Set of objects that determine initial position, normal and other properties of newly spawned particles. Each
		 * particle system must have at least one emitter.
		 */
		void SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters);

		/** @copydoc setEmitters */
		const Vector<SPtr<ParticleEmitter>>& GetEmitters() const { return mEmitters; }

		/**
		 * Set of objects that determine how particle properties change during their lifetime. Evolvers only affect
		 * CPU simulated particles.
		 */
		void SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers);

		/** @copydoc setEmitters */
		const Vector<SPtr<ParticleEvolver>>& GetEvolvers() const { return mEvolvers; }

		/**
		 * Determines the layer bitfield that controls whether a system is considered visible in a specific camera.
		 * Layer must match camera layer in order for the camera to render the component.
		 */
		void SetLayer(u64 layer);

		/** @copydoc setLayer() */
		u64 GetLayer() const { return mLayer; }

		/** Starts the particle system. New particles will be emitted and existing particles will be evolved. */
		void Play();

		/** Pauses the particle system. New particles will stop being emitted and existing particle state will be frozen. */
		void Pause();

		/** Stops the particle system and resets it to initial state, clearing all particles. */
		void Stop();

		/**	Retrieves an implementation of the particle system usable only from the core thread. */
		SPtr<ct::ParticleSystem> GetCore() const;

		/** Creates a new empty ParticleSystem object. */
		static SPtr<ParticleSystem> Create();

		/**
		 * @name Internal
		 */

		/**
		 * Updates the particle simulation by advancing it by @p timeDelta. New state will be updated in the internal
		 * ParticleSet.
		 */
		void SimulateInternal(float timeDelta, const EvaluatedAnimationData* animData);

		/**
		 * Calculates the bounds of all the particles in the system. Should be called after a call to SimulateInternal() to get
		 * up-to-date bounds. The bounds are in the simulation space of the particle system.
		 */
		AABox CalculateBoundsInternal() const;

		/**
		 * Advances the particle system time according to the current time, time delta and the provided settings.
		 *
		 * @param[in]		time		Current time to use as a base.
		 * @param[in]		timeDelta	Amount of time to advance the time by.
		 * @param[in]		duration	Maximum time allowed by the particle system.
		 * @param[in]		loop		Determines what happens when the time exceeds @p duration. If true the time will
		 *								wrap around to 0 and start over, if false the time will be clamped to @p
		 *								duration.
		 * @param[out]		timeStep	Actual time-step the simulation was advanced by. This is normally equal to
		 *								@p timeDelta but might be a different value if time was clamped.
		 * @return						New time value.
		 */
		static float AdvanceTimeInternal(float time, float timeDelta, float duration, bool loop, float& timeStep);

		/** @} */
	private:
		friend class ParticleManager;
		friend class ParticleSystemRTTI;
		friend class ParticleEmitter;
		friend class ct::ParticleSystem;

		/** States the particle system can be in. */
		enum class State
		{
			Uninitialized,
			Stopped,
			Paused,
			Playing
		};

		ParticleSystem();

		/**
		 * Decrements particle lifetime, kills expired particles and executes evolvers that need to run before
		 * the simulation.
		 *
		 * @param[in]	state			State describing the current state of the simulation.
		 * @param[in]	startIdx		Index of the first particle to update.
		 * @param[in]	count			Number of particles to update, starting from @p startIdx.
		 * @param[in]	spacing			When false all particles will use the same time-step. If true the time-step will
		 *								be divided by @p count so particles are uniformly distributed over the
		 *								time-step.
		 * @param[in]	spacingOffset	Extra offset that controls the starting position of the first particle when
		 *								calculating spacing. Should be in range [0, 1). 0 = beginning of the current
		 *								time step, 1 = start of next particle.
		 */
		void PreSimulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset);

		/**
		 * Integrates particle properties, advancing the simulation.
		 *
		 * @param[in]	state			State describing the current state of the simulation.
		 * @param[in]	startIdx		Index of the first particle to update.
		 * @param[in]	count			Number of particles to update, starting from @p startIdx.
		 * @param[in]	spacing			When false all particles will use the same time-step. If true the time-step will
		 *								be divided by @p count so particles are uniformly distributed over the
		 *								time-step.
		 * @param[in]	spacingOffset	Extra offset that controls the starting position of the first particle when
		 *								calculating spacing. Should be in range [0, 1). 0 = beginning of the current
		 *								time step, 1 = start of next particle.
		 */
		void Simulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset);

		/**
		 * Executes evolvers that need to run after the simulation.
		 *
		 * @param[in]	state			State describing the current state of the simulation.
		 * @param[in]	startIdx		Index of the first particle to update.
		 * @param[in]	count			Number of particles to update, starting from @p startIdx.
		 * @param[in]	spacing			When false all particles will use the same time-step. If true the time-step will
		 *								be divided by @p count so particles are uniformly distributed over the
		 *								time-step.
		 * @param[in]	spacingOffset	Extra offset that controls the starting position of the first particle when
		 *								calculating spacing. Should be in range [0, 1). 0 = beginning of the current
		 *								time step, 1 = start of next particle.
		 */
		void PostSimulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> CreateCore() const;

		/** @copydoc SceneActor::_markCoreDirty */
		void MarkCoreDirtyInternal(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData SyncToCore(FrameAlloc* allocator) override;

		/** @copydoc CoreObject::getCoreDependencies */
		void GetCoreDependencies(Vector<CoreObject*>& dependencies);

		/**	Creates a new ParticleSystem instance without initializing it. */
		static SPtr<ParticleSystem> CreateEmpty();

		SPtr<SceneInstance> mScene;
		ParticleSystemSettings mSettings;
		ParticleGpuSimulationSettings mGpuSimulationSettings;
		Vector<SPtr<ParticleEmitter>> mEmitters;
		Vector<SPtr<ParticleEvolver>> mEvolvers;
		u64 mLayer = 1;

		// Internal state
		u32 mId = 0;
		State mState = State::Uninitialized;
		float mTime = 0.0f;
		u32 mSeed = 0;

		Random mRandom;
		ParticleSet* mParticleSet = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleSystemRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	namespace ct
	{
		/** Core thread counterpart of bs::ParticleSystem. */
		class BS_CORE_EXPORT ParticleSystem final : public CoreObject, public SceneActor, public INonCopyable
		{
		public:
			~ParticleSystem();

			/** @copydoc bs::ParticleSystem::setSettings */
			const ParticleSystemSettings& GetSettings() const { return mSettings; }

			/** @copydoc bs::ParticleSystem::setGpuSimulationSettings */
			const ParticleGpuSimulationSettings& GetGpuSimulationSettings() const { return mGpuSimulationSettings; }

			/**
			 * Determines the layer bitfield that controls whether a system is considered visible in a specific camera.
			 * Layer must match camera layer in order for the camera to render the component.
			 */
			void SetLayer(u64 layer);

			/** @copydoc setLayer() */
			u64 GetLayer() const { return mLayer; }

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/**
			 * Returns an ID that uniquely identifies the particle system. Can be used for locating evaluated particle
			 * system render data in the structure output by the ParticlesManager.
			 */
			u32 GetId() const { return mId; }

			/** @copydoc CoreObject::initialize */
			void Initialize() override;

		private:
			friend class bs::ParticleSystem;

			ParticleSystem(u32 id)
				: mId(id)
			{}

			/** @copydoc CoreObject::syncToCore */
			void SyncToCore(const CoreSyncData& data) override;

			u32 mRendererId = 0;
			u32 mId;

			ParticleSystemSettings mSettings;
			ParticleGpuSimulationSettings mGpuSimulationSettings;
			u64 mLayer = 1;
		};
	} // namespace ct

	/** @} */
} // namespace bs
