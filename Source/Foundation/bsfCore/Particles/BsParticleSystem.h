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

	namespace ct { class ParticleSystem; }

	/** @addtogroup Implementation
	 *  @{
	 */

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Possible orientations when rendering billboard particles. */
	enum class BS_SCRIPT_EXPORT(m:Particles) ParticleOrientation
	{
		/** Orient towards view (camera) plane. */
		ViewPlane,

		/** Orient towards view (camera) position. */
		ViewPosition,

		/** Orient with a user-provided plane. */
		Plane
	};

	/** Space in which to spawn/transform particles. */
	enum class BS_SCRIPT_EXPORT(m:Particles) ParticleSimulationSpace
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
	enum class BS_SCRIPT_EXPORT(m:Particles) ParticleSortMode
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
	enum class BS_SCRIPT_EXPORT(m:Particles) ParticleRenderMode
	{
		/** Particle is represented using a 2D quad. */
		Billboard,

		/** Particle is represented using a 3D mesh. */
		Mesh
	};

	/** Controls depth buffer collisions for GPU simulated particles. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) ParticleDepthCollisionSettings : IReflectable
	{
		BS_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings() = default;

		/** Determines if depth collisions are enabled. */
		BS_SCRIPT_EXPORT()
		bool enabled = false;

		/** 
		 * Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy
		 * and higher values more. 
		 */
		BS_SCRIPT_EXPORT()
		float restitution = 1.0f;

		/**
		 * Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In
		 * range [0, 1].
		 */
		BS_SCRIPT_EXPORT()
		float dampening = 0.5f;

		/** Scale which to apply to particle size in order to determine the collision radius. */
		BS_SCRIPT_EXPORT()
		float radiusScale = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);
	public:
		friend class ParticleDepthCollisonSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common base for both sim and core thread variants of ParticleSystemSettings. */
	struct ParticleSystemSettingsBase
	{
		/** 
		 * If true the particle system will be simulated on the GPU. This allows much higher particle counts at lower
		 * performance cost. GPU simulation ignores any provided evolvers and instead uses ParticleGpuSimulationSettings
		 * to customize the GPU simulation. 
		 */
		BS_SCRIPT_EXPORT()
		bool gpuSimulation = false;

		/** Determines in which space are particles in. */
		BS_SCRIPT_EXPORT()
		ParticleSimulationSpace simulationSpace = ParticleSimulationSpace::World;

		/** Determines how are particles oriented when rendering. */
		BS_SCRIPT_EXPORT()
		ParticleOrientation orientation = ParticleOrientation::ViewPlane;

		/** 
		 * Determines should the particles only be allowed to orient themselves around the Y axis, or freely. Ignored if
		 * using the Plane orientation mode.
		 */
		BS_SCRIPT_EXPORT()
		bool orientationLockY = false;

		/** 
		 * Determines a plane to orient particles towards. Only used if particle orientation mode is set to 
		 * ParticleOrientation::Plane. 
		 */
		BS_SCRIPT_EXPORT()
		Plane orientationPlane = Plane(Vector3::UNIT_Z, Vector3::ZERO);

		/** 
		 * Determines how (and if) are particles sorted. Sorting controls in what order are particles rendered. 
		 * If GPU simulation is enabled only distance based sorting is supported.
		 */
		BS_SCRIPT_EXPORT()
		ParticleSortMode sortMode = ParticleSortMode::None;

		/** 
		 * Determines the time period during which the system runs, in seconds. This effects evaluation of distributions
		 * with curves using particle system time for evaluation.
		 */
		BS_SCRIPT_EXPORT()
		float duration = 5.0f;

		/** Determines should the particle system time wrap around once it reaches its duration. */
		BS_SCRIPT_EXPORT()
		bool isLooping = true;

		/** 
		 * Determines the maximum number of particles that can ever be active in this system. This number is ignored
		 * if GPU simulation is enabled, and instead particle count is instead only limited by the size of the internal
		 * buffers (shared between all particle systems).
		 */
		BS_SCRIPT_EXPORT()
		UINT32 maxParticles = 2000;

		/** 
		 * Determines should an automatic seed be used for the internal random number generator. This ensures the particle
		 * system yields different results each time it is ran.
		 */
		BS_SCRIPT_EXPORT()
		bool useAutomaticSeed = true;

		/** 
		 * Determines the seed to use for the internal random number generator. Allows you to guarantee identical behaviour
		 * between different runs. Only relevant if automatic seed is disabled.
		 */
		BS_SCRIPT_EXPORT()
		UINT32 manualSeed = 0;

		/**
		 * Determines should the particle system bounds be automatically calculated, or should the fixed value provided
		 * be used. Bounds are used primarily for culling purposes. Note that automatic bounds are not supported when GPU
		 * simulation is enabled.
		 */
		BS_SCRIPT_EXPORT()
		bool useAutomaticBounds = true;

		/** 
		 * Custom bounds to use them @p useAutomaticBounds is disabled. The bounds are in the simulation space of the
		 * particle system.
		 */
		BS_SCRIPT_EXPORT()
		AABox customBounds;

		/** Determines how is each particle represented on the screen. */
		BS_SCRIPT_EXPORT()
		ParticleRenderMode renderMode = ParticleRenderMode::Billboard;
	};

	/** Templated common base for both sim and core thread variants of ParticleSystemSettings. */
	template<bool Core>
	struct TParticleSystemSettings : ParticleSystemSettingsBase
	{
		using MaterialType = CoreVariantHandleType<Material, Core>;
		using MeshType = CoreVariantHandleType<Mesh, Core>;

		/** Material to render the particles with. */
		BS_SCRIPT_EXPORT()
		MaterialType material;

		/** Mesh used for representing individual particles when using the Mesh rendering mode. */
		BS_SCRIPT_EXPORT()
		MeshType mesh;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	};

	/** Common base for both sim and core thread variants of ParticleVectorFieldSettings. */
	struct ParticleVectorFieldSettingsBase
	{
		/** Intensity of the forces and velocities applied by the vector field. */
		BS_SCRIPT_EXPORT()
		float intensity = 1.0f;

		/** 
		 * Determines how closely does the particle velocity follow the vectors in the field. If set to 1 particles
		 * will be snapped to the exact velocity of the value in the field, and if set to 0 the field will not influence
		 * particle velocities directly. 
		 */
		BS_SCRIPT_EXPORT()
		float tightness = 0.0f;

		/** Scale to apply to the vector field bounds. This is multiplied with the bounds of the vector field resource. */
		BS_SCRIPT_EXPORT()
		Vector3 scale = Vector3::ONE;

		/** 
		 * Amount of to move the vector field by relative to the parent particle system. This is added to the bounds
		 * provided in the vector field resource.
		 */
		BS_SCRIPT_EXPORT()
		Vector3 offset = Vector3::ZERO;

		/** Initial rotation of the vector field. */
		BS_SCRIPT_EXPORT()
		Quaternion rotation = Quaternion::IDENTITY;

		/** 
		 * Determines the amount to rotate the vector field every second, in degrees, around XYZ axis respectively. 
		 * Evaluated over the particle system lifetime. 
		 */
		BS_SCRIPT_EXPORT()
		Vector3Distribution rotationRate = Vector3(0.0f, 90.0f, 0.0f);

		/** 
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the X direction.
		 */
		BS_SCRIPT_EXPORT()
		bool tilingX = false;

		/** 
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Y direction.
		 */
		BS_SCRIPT_EXPORT()
		bool tilingY = false;

		/** 
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Z direction.
		 */
		BS_SCRIPT_EXPORT()
		bool tilingZ = false;
	};

	/** Templated common base for both sim and core thread variants of ParticleVectorFieldSettings. */
	template<bool Core>
	struct TParticleVectorFieldSettings : ParticleVectorFieldSettingsBase
	{
		/** Vector field resource used for influencing the particles. */
		BS_SCRIPT_EXPORT()
		CoreVariantHandleType<VectorField, Core> vectorField;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	};

	/** @} */
	/** @addtogroup Particles
	 *  @{
	 */

	/** Settings used for controlling a vector field in a GPU simulated particle system. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) 
	ParticleVectorFieldSettings : TParticleVectorFieldSettings<false>, IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleVectorFieldSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
		/** Core thread counterpart of bs::ParticleVectorFieldSettings. */
		struct ParticleVectorFieldSettings : TParticleVectorFieldSettings<true>
		{ };
	}

	/** @} */
	/** @addtogroup Implementation
	 *  @{
	 */

	template<> struct CoreThreadType<ParticleVectorFieldSettings> { typedef ct::ParticleVectorFieldSettings Type; };

	/** Common base for both sim and core threat variants of ParticleGpuSimulationSettings. */
	struct ParticleGpuSimulationSettingsBase
	{
		/** Determines particle color, evaluated over the particle lifetime. */
		BS_SCRIPT_EXPORT()
		ColorDistribution colorOverLifetime = Color::White;

		/** Determines particle size, evaluated over the particle lifetime. Multiplied by the initial particle size. */
		BS_SCRIPT_EXPORT()
		Vector2Distribution sizeScaleOverLifetime = Vector2::ONE;

		/** Constant acceleration to apply for each step of the simulation. */
		BS_SCRIPT_EXPORT()
		Vector3 acceleration = Vector3::ZERO;

		/** Amount of resistance to apply in the direction opposite of the particle's velocity. */
		BS_SCRIPT_EXPORT()
		float drag = 0.0f;

		/** Settings controlling particle depth buffer collisions. */
		BS_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings depthCollision;
	};

	/** Templated common base for both sim and core threat variants of ParticleGpuSimulationSettings. */
	template<bool Core>
	struct TParticleGpuSimulationSettings : ParticleGpuSimulationSettingsBase
	{
		BS_SCRIPT_EXPORT()
		CoreVariantType<ParticleVectorFieldSettings, Core> vectorField;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P processor);
	};

	/** @} */
	/** @addtogroup Particles
	 *  @{
	 */

	/** Generic settings used for controlling a ParticleSystem. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) 
	ParticleSystemSettings : TParticleSystemSettings<false>, IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Settings used for controlling particle system GPU simulation. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) 
	ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<false>, IReflectable
	{
		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleGpuSimulationSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal 
	 *  @{
	 */

	namespace ct
	{
		class VectorField;

		/** Core thread counterpart of bs::ParticleSystemSettings. */
		struct ParticleSystemSettings : TParticleSystemSettings<true> { };

		/** Core thread counterpart of bs::ParticleVectorFieldSettings. */
		struct ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<true> { };
	}

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Manages a list of all emitters used by a ParticleSystem. */
	class BS_CORE_EXPORT ParticleSystemEmitters : public IReflectable
	{
	public:
		/** Registers a new particle emitter. */
		void add(SPtr<ParticleEmitter> emitter)
		{
			mList.push_back(std::move(emitter));
		}

		/** Returns the number of particle emitters present in this system. */
		UINT32 getCount() const { return (UINT32)mList.size(); }

		/** 
		 * Returns the particle emitter present at the specified sequential index. Returns null if provided index is 
		 * invalid. 
		 */
		ParticleEmitter* get(UINT32 idx)
		{
			if(idx >= (UINT32)mList.size())
				return nullptr;

			return mList[idx].get();
		}

		/** Removes a particle emitter. */
		void remove(ParticleEmitter* emitter)
		{
			const auto iterFind = std::find_if(mList.begin(), mList.end(), 
				[emitter](const SPtr<ParticleEmitter>& curEmitter)
			{
				return curEmitter.get() == emitter;
				
			});

			if(iterFind != mList.end())
				mList.erase(iterFind);
		}

	private:
		friend class ParticleSystem;

		Vector<SPtr<ParticleEmitter>> mList;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemEmittersRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** Manages a list of all evolvers used by a ParticleSystem. */
	class BS_CORE_EXPORT ParticleSystemEvolvers : public IReflectable
	{
	public:
		ParticleSystemEvolvers();

		/** Registers a new particle evolver. */
		void add(SPtr<ParticleEvolver> evolver)
		{
			addToSortedList(evolver.get());
			mList.push_back(std::move(evolver));
		}

		/** Returns the number of particle evolvers present in this system. */
		UINT32 getCount() const { return (UINT32)mList.size(); }

		/** 
		 * Returns the particle evolver present at the specified sequential index. Returns null if provided index is 
		 * invalid. 
		 */
		ParticleEvolver* get(UINT32 idx)
		{
			if(idx >= (UINT32)mList.size())
				return nullptr;

			return mList[idx].get();
		}

		/** Removes a particle evolver. */
		void remove(ParticleEvolver* evolver)
		{
			const auto iterFind = std::find_if(mList.begin(), mList.end(), 
				[evolver](const SPtr<ParticleEvolver>& curEvolver)
			{
				return curEvolver.get() == evolver;
				
			});

			if(iterFind != mList.end())
				mList.erase(iterFind);

			mSortedList.erase(evolver);
		}

	private:
		friend class ParticleSystem;

		/** Registers the particle evolver in one or multiple sorted lists, depending on its type. */
		void addToSortedList(ParticleEvolver* evolver)
		{
			mSortedList.insert(evolver);
		}

		typedef std::function<bool(const ParticleEvolver*, const ParticleEvolver*)> EvolverComparison; 
		Set<ParticleEvolver*, EvolverComparison> mSortedList;

		Vector<SPtr<ParticleEvolver>> mList;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemEvolversRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

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
		void setSettings(const ParticleSystemSettings& settings);

		/** @copydoc setSettings */
		const ParticleSystemSettings& getSettings() const { return mSettings; }

		/** Determines settings that control particle GPU simulation. */
		void setGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/** @copydoc setGpuSimulationSettings */
		const ParticleGpuSimulationSettings& getGpuSimulationSettings() const { return mGpuSimulationSettings; }

		/** Returns an object that allows you to modify the list of emitters used by this system. */
		ParticleSystemEmitters& getEmitters() { return mEmitters; }

		/** Returns an object that allows you to modify the list of evolvers used by this system. */
		ParticleSystemEvolvers& getEvolvers() { return mEvolvers; }

		/**
		 * Determines the layer bitfield that controls whether a system is considered visible in a specific camera. 
		 * Layer must match camera layer in order for the camera to render the component.
		 */
		void setLayer(UINT64 layer);

		/** @copydoc setLayer() */
		UINT64 getLayer() const { return mLayer; }

		/** Starts the particle system. New particles will be emitted and existing particles will be evolved. */
		void play();

		/** Pauses the particle system. New particles will stop being emitted and existing particle state will be frozen. */
		void pause();

		/** Stops the particle system and resets it to initial state, clearing all particles. */
		void stop();

		/**	Retrieves an implementation of the particle system usable only from the core thread. */
		SPtr<ct::ParticleSystem> getCore() const;

		/** Creates a new empty ParticleSystem object. */
		static SPtr<ParticleSystem> create();

		/** 
		 * @name Internal
		 */

		/** 
		 * Updates the particle simulation by advancing it by @p timeDelta. New state will be updated in the internal
		 * ParticleSet.
		 */
		void _simulate(float timeDelta, const EvaluatedAnimationData* animData);

		/** 
		 * Calculates the bounds of all the particles in the system. Should be called after a call to _simulate() to get
		 * up-to-date bounds. The bounds are in the simulation space of the particle system.
		 */
		AABox _calculateBounds() const;

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
		static float _advanceTime(float time, float timeDelta, float duration, bool loop, float& timeStep);

		/** @} */
	private:
		friend class ParticleManager;
		friend class ParticleSystemRTTI;
		friend class ct::ParticleSystem;

		/** States the particle system can be in. */
		enum class State
		{
			Uninitialized, Stopped, Paused, Playing
		};

		ParticleSystem();

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc SceneActor::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates a new ParticleSystem instance without initializing it. */
		static SPtr<ParticleSystem> createEmpty();

		ParticleSystemSettings mSettings;
		ParticleGpuSimulationSettings mGpuSimulationSettings;
		ParticleSystemEmitters mEmitters;
		ParticleSystemEvolvers mEvolvers;
		UINT64 mLayer = 1;

		// Internal state
		UINT32 mId = 0;
		State mState = State::Uninitialized;
		float mTime = 0.0f;
		UINT32 mSeed = 0;

		Random mRandom;
		ParticleSet* mParticleSet = nullptr;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleSystemRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
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
			const ParticleSystemSettings& getSettings() const { return mSettings; }

			/** @copydoc bs::ParticleSystem::setGpuSimulationSettings */
			const ParticleGpuSimulationSettings& getGpuSimulationSettings() const { return mGpuSimulationSettings; }

			/**
			 * Determines the layer bitfield that controls whether a system is considered visible in a specific camera.
			 * Layer must match camera layer in order for the camera to render the component.
			 */
			void setLayer(UINT64 layer);

			/** @copydoc setLayer() */
			UINT64 getLayer() const { return mLayer; }

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void setRendererId(UINT32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			UINT32 getRendererId() const { return mRendererId; }

			/** 
			 * Returns an ID that uniquely identifies the particle system. Can be used for locating evaluated particle 
			 * system render data in the structure output by the ParticlesManager. 
			 */
			UINT32 getId() const { return mId; }

			/** @copydoc CoreObject::initialize */
			void initialize() override;
		private:
			friend class bs::ParticleSystem;

			ParticleSystem(UINT32 id)
				:mId(id)
			{ }

			/** @copydoc CoreObject::syncToCore */
			void syncToCore(const CoreSyncData& data) override;

			UINT32 mRendererId = 0;
			UINT32 mId;

			ParticleSystemSettings mSettings;
			ParticleGpuSimulationSettings mGpuSimulationSettings;
			UINT64 mLayer = 1;
		};
	}

	/** @} */
}
