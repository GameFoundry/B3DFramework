//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"
#include "Particles/B3DParticleDistribution.h"
#include "Particles/B3DParticleModule.h"
#include "Math/B3DTransform.h"
#include "Math/B3DRandom.h"
#include "Renderer/B3DRendererId.h"
#include "Renderer/B3DRendererObjectECSUtility.h"
#include "Renderer/B3DRendererObjectStorage.h"

namespace b3d
{
	struct EvaluatedAnimationData;
	class SkeletonMask;
	class ParticleEmitter;
	class ParticleEvolver;
	class ParticleSet;
	class ParticleSystemObjectStorageBase;
	struct ParticleSystemFullUpdateChannel;
	struct ParticleSystemTransformUpdateChannel;

	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ParticleSystemSettings)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ParticleVectorFieldSettings)
	B3D_CORE_OBJECT_FORWARD_DECLARE_STRUCT(ParticleGpuSimulationSettings)

	/** @addtogroup Particles
	 *  @{
	 */

	/** Possible orientations when rendering billboard particles. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleOrientation
	{
		/** Orient towards view (camera) plane. */
		ViewPlane,

		/** Orient towards view (camera) position. */
		ViewPosition,

		/** Orient with a user-provided axis. */
		Plane
	};

	/** Space in which to spawn/transform particles. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSimulationSpace
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSortMode
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
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleRenderMode
	{
		/** Particle is represented using a 2D quad. */
		Billboard,

		/** Particle is represented using a 3D mesh. */
		Mesh
	};

	/** Controls depth buffer collisions for GPU simulated particles. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleDepthCollisionSettings : IReflectable, IScriptExportable
	{
		struct SyncPacket;

		B3D_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings() = default;

		/** Determines if depth collisions are enabled. */
		B3D_SCRIPT_EXPORT()
		bool Enabled = false;

		/**
		 * Determines the elasticity (bounciness) of the particle collision. Lower values make the collision less bouncy
		 * and higher values more.
		 */
		B3D_SCRIPT_EXPORT()
		float Restitution = 1.0f;

		/**
		 * Determines how much velocity should a particle lose after a collision, in percent of its current velocity. In
		 * range [0, 1].
		 */
		B3D_SCRIPT_EXPORT()
		float Dampening = 0.5f;

		/** Scale which to apply to particle size in order to determine the collision radius. */
		B3D_SCRIPT_EXPORT()
		float RadiusScale = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleDepthCollisonSettingsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Common base for both main and render thread variants of ParticleSystemSettings. */
	struct ParticleSystemSettingsBase
	{
		/** Determines in which space are particles in. */
		B3D_SCRIPT_EXPORT()
		ParticleSimulationSpace SimulationSpace = ParticleSimulationSpace::World;

		/** Determines how are particles oriented when rendering. */
		B3D_SCRIPT_EXPORT()
		ParticleOrientation Orientation = ParticleOrientation::ViewPlane;

		/**
		 * Determines the time period during which the system runs, in seconds. This effects evaluation of distributions
		 * with curves using particle system time for evaluation.
		 */
		B3D_SCRIPT_EXPORT()
		float Duration = 5.0f;

		/** Determines should the particle system time wrap around once it reaches its duration. */
		B3D_SCRIPT_EXPORT()
		bool IsLooping = true;

		/**
		 * Determines the maximum number of particles that can ever be active in this system. This number is ignored
		 * if GPU simulation is enabled, and instead particle count is instead only limited by the size of the internal
		 * buffers (shared between all particle systems).
		 */
		B3D_SCRIPT_EXPORT()
		u32 MaxParticles = 2000;

		/**
		 * If true the particle system will be simulated on the GPU. This allows much higher particle counts at lower
		 * performance cost. GPU simulation ignores any provided evolvers and instead uses ParticleGpuSimulationSettings
		 * to customize the GPU simulation.
		 */
		B3D_SCRIPT_EXPORT(UICategory(Advanced), UIOrder(1))
		bool GpuSimulation = false;

		/** Determines how is each particle represented on the screen. */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		ParticleRenderMode RenderMode = ParticleRenderMode::Billboard;

		/**
		 * Determines should the particles only be allowed to orient themselves around the Y axis, or freely. Ignored if
		 * using the Plane orientation mode.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		bool OrientationLockY = false;

		/**
		 * Determines a normal of the plane to orient particles towards. Only used if particle orientation mode is set to
		 * ParticleOrientation::Plane.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		Vector3 OrientationPlaneNormal = Vector3::kUnitZ;

		/**
		 * Determines how (and if) are particles sorted. Sorting controls in what order are particles rendered.
		 * If GPU simulation is enabled only distance based sorting is supported.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		ParticleSortMode SortMode = ParticleSortMode::None;

		/**
		 * Determines should an automatic seed be used for the internal random number generator. This ensures the particle
		 * system yields different results each time it is ran.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		bool UseAutomaticSeed = true;

		/**
		 * Determines the seed to use for the internal random number generator. Allows you to guarantee identical behaviour
		 * between different runs. Only relevant if automatic seed is disabled.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		u32 ManualSeed = 0;

		/**
		 * Determines should the particle system bounds be automatically calculated, or should the fixed value provided
		 * be used. Bounds are used primarily for culling purposes. Note that automatic bounds are not supported when GPU
		 * simulation is enabled.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		bool UseAutomaticBounds = true;

		/**
		 * Custom bounds to use them @p UseAutomaticBounds is disabled. The bounds are in the simulation space of the
		 * particle system.
		 */
		B3D_SCRIPT_EXPORT(UIOrder(2))
		AABox CustomBounds;
	};

	/** Templated common base for both main and render thread variants of ParticleSystemSettings. */
	template <bool IsRenderProxy>
	struct TParticleSystemSettings : ParticleSystemSettingsBase
	{
		using MaterialType = CoreVariantHandleType<Material, IsRenderProxy>;
		using MeshType = CoreVariantHandleType<Mesh, IsRenderProxy>;

		/** Material to render the particles with. */
		B3D_SCRIPT_EXPORT(LoadOnAssign(true))
		MaterialType Material;

		/** Mesh used for representing individual particles when using the Mesh rendering mode. */
		B3D_SCRIPT_EXPORT(LoadOnAssign(true), UIOrder(2))
		MeshType Mesh;
	};

	/** Common base for both main and render thread variants of ParticleVectorFieldSettings. */
	struct ParticleVectorFieldSettingsBase
	{
		/** Intensity of the forces and velocities applied by the vector field. */
		B3D_SCRIPT_EXPORT()
		float Intensity = 1.0f;

		/**
		 * Determines how closely does the particle velocity follow the vectors in the field. If set to 1 particles
		 * will be snapped to the exact velocity of the value in the field, and if set to 0 the field will not influence
		 * particle velocities directly.
		 */
		B3D_SCRIPT_EXPORT()
		float Tightness = 0.0f;

		/** Scale to apply to the vector field bounds. This is multiplied with the bounds of the vector field resource. */
		B3D_SCRIPT_EXPORT()
		Vector3 Scale = Vector3::kOne;

		/**
		 * Amount of to move the vector field by relative to the parent particle system. This is added to the bounds
		 * provided in the vector field resource.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3 Offset = Vector3::kZero;

		/** Initial rotation of the vector field. */
		B3D_SCRIPT_EXPORT()
		Quaternion Rotation = Quaternion::kIdentity;

		/**
		 * Determines the amount to rotate the vector field every second, in degrees, around XYZ axis respectively.
		 * Evaluated over the particle system lifetime.
		 */
		B3D_SCRIPT_EXPORT()
		Vector3Distribution RotationRate = Vector3(0.0f, 90.0f, 0.0f);

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the X direction.
		 */
		B3D_SCRIPT_EXPORT()
		bool TilingX = false;

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Y direction.
		 */
		B3D_SCRIPT_EXPORT()
		bool TilingY = false;

		/**
		 * Determines should the field influence particles outside of the field bounds. If true the field will be tiled
		 * infinitely in the Z direction.
		 */
		B3D_SCRIPT_EXPORT()
		bool TilingZ = false;
	};

	/** Templated common base for both main and render thread variants of ParticleVectorFieldSettings. */
	template <bool IsRenderProxy>
	struct TParticleVectorFieldSettings : ParticleVectorFieldSettingsBase
	{
		/** Vector field resource used for influencing the particles. */
		B3D_SCRIPT_EXPORT()
		CoreVariantHandleType<VectorField, IsRenderProxy> VectorField;
	};

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Settings used for controlling a vector field in a GPU simulated particle system. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles))
	ParticleVectorFieldSettings : TParticleVectorFieldSettings<false>, IReflectable, IScriptExportable
	{
		friend struct render::ParticleVectorFieldSettings;
		struct SyncPacket;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleVectorFieldSettingsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	namespace render
	{
		/** Render thread counterpart of b3d::ParticleVectorFieldSettings. */
		struct ParticleVectorFieldSettings : TParticleVectorFieldSettings<true>
		{
			friend struct b3d::ParticleVectorFieldSettings;
		};
	} // namespace render

	/** Common base for both main and render thread variants of ParticleGpuSimulationSettings. */
	struct ParticleGpuSimulationSettingsBase
	{
		/** Determines particle color, evaluated over the particle lifetime. */
		B3D_SCRIPT_EXPORT()
		ColorDistribution ColorOverLifetime = Color::kWhite;

		/** Determines particle size, evaluated over the particle lifetime. Multiplied by the initial particle size. */
		B3D_SCRIPT_EXPORT()
		Vector2Distribution SizeScaleOverLifetime = Vector2::kOne;

		/** Constant acceleration to apply for each step of the simulation. */
		B3D_SCRIPT_EXPORT()
		Vector3 Acceleration = Vector3::kZero;

		/** Amount of resistance to apply in the direction opposite of the particle's velocity. */
		B3D_SCRIPT_EXPORT()
		float Drag = 0.0f;

		/** Settings controlling particle depth buffer collisions. */
		B3D_SCRIPT_EXPORT()
		ParticleDepthCollisionSettings DepthCollision;
	};

	/** Templated common base for both main and render thread variants of ParticleGpuSimulationSettings. */
	template <bool IsRenderProxy>
	struct TParticleGpuSimulationSettings : ParticleGpuSimulationSettingsBase
	{
		B3D_SCRIPT_EXPORT()
		CoreVariantType<ParticleVectorFieldSettings, IsRenderProxy> VectorField;
	};

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** Generic settings used for controlling a ParticleSystem. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles))
	ParticleSystemSettings : TParticleSystemSettings<false>, IReflectable, IScriptExportable
	{
		friend struct render::ParticleSystemSettings;
		struct SyncPacket;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemSettingsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** Settings used for controlling particle system GPU simulation. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles))
	ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<false>, IReflectable, IScriptExportable
	{
		friend struct render::ParticleGpuSimulationSettings;
		struct SyncPacket;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/

	public:
		friend class ParticleGpuSimulationSettingsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	namespace render
	{
		class VectorField;

		/** Render  thread counterpart of b3d::ParticleSystemSettings. */
		struct ParticleSystemSettings : TParticleSystemSettings<true>
		{
			friend struct b3d::ParticleSystemSettings;
		};

		/** Render  thread counterpart of b3d::ParticleVectorFieldSettings. */
		struct ParticleGpuSimulationSettings : TParticleGpuSimulationSettings<true>
		{
			friend struct b3d::ParticleGpuSimulationSettings;
		};
	} // namespace render

	/** Shared particle system property data used by both main and render thread variants. */
	template <bool IsRenderProxy>
	struct B3D_EXPORT TParticleSystemData
	{
		using SettingsType = CoreVariantType<ParticleSystemSettings, IsRenderProxy>;
		using GpuSimSettingsType = CoreVariantType<ParticleGpuSimulationSettings, IsRenderProxy>;

		SettingsType Settings;
		GpuSimSettingsType GpuSimulationSettings;
		u64 Layer = 1;
	};

	/** CRTP getter interface providing shared read access for particle system data to both ParticleSystem and render::ParticleSystemProxy. */
	template <typename Derived, bool IsRenderProxy>
	class TParticleSystemGetters
	{
		using SettingsType = CoreVariantType<ParticleSystemSettings, IsRenderProxy>;
		using GpuSimSettingsType = CoreVariantType<ParticleGpuSimulationSettings, IsRenderProxy>;

	public:
		/** Determines general purpose settings that apply to the particle system. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Settings), PassByCopy(true))
		const SettingsType& GetSettings() const { return GetData().Settings; }

		/** Determines settings that control particle GPU simulation. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(GpuSimulationSettings), PassByCopy(true))
		const GpuSimSettingsType& GetGpuSimulationSettings() const { return GetData().GpuSimulationSettings; }

		/**
		 * Determines the layer bitfield that controls whether a system is considered visible in a specific camera.
		 * Layer must match camera layer in order for the camera to render the component.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Layer), UI(AsLayerMask))
		u64 GetLayer() const { return GetData().Layer; }

	private:
		const TParticleSystemData<IsRenderProxy>& GetData() const
		{
			return static_cast<const Derived*>(this)->GetParticleSystemData();
		}
	};

	class RendererScene;

	namespace ecs
	{
		class ECSParticleSystemRTTI;

		/** ECS fragment storing particle system configuration data (settings, gpu simulation settings, layer, emitters, evolvers). */
		struct B3D_EXPORT ParticleSystem : TParticleSystemData<false>, IReflectable
		{
			/** Unique identifier for this particle system. */
			u32 Id = 0;

			/** Particle emitters that control how new particles are generated. */
			Vector<TShared<ParticleEmitter>> Emitters;

			/** Particle evolvers that modify particle properties over time. Stored sorted by priority (descending). */
			Vector<TShared<ParticleEvolver>> Evolvers;

			struct FullSyncPacket;
			struct TransformSyncPacket;

			friend class ECSParticleSystemRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** Determines current state of the particle system simulation. */
		enum class ParticleSimulationState : u8
		{
			Uninitialized,
			Stopped,
			Paused,
			Playing
		};

		/** ECS fragment storing particle simulation runtime state. */
		struct ParticleSimulation
		{
			/** Active particle data. Lazily allocated on first simulation frame. */
			TShared<ParticleSet> Particles;

			/** Current simulation time. */
			float Time = 0.0f;

			/** Random seed for deterministic simulation. */
			u32 Seed = 0;

			/** Random number generator seeded with @p Seed. */
			Random Rng;

			/** Current simulation state. */
			ParticleSimulationState State = ParticleSimulationState::Uninitialized;
		};

		/** ECS fragment storing the persistent render object ID for a particle system. */
		struct ParticleSystemId
		{
			RendererId Id = kInvalidRendererId;
		};

		/** Tag indicating a ParticleSystem needs to sync all of its properties to its render proxy. */
		struct ParticleSystemDirty {};

		/** Tag indicating a ParticleSystem needs to sync transform to its render proxy. */
		struct ParticleSystemTransformDirty {};

		/** Creates all ParticleSystem data fragments, a world transform, and allocates a renderer ID. Entity is ready for rendering after this call. Returns the ParticleSystem data fragment. */
		ParticleSystem& CreateParticleSystem(Registry& registry, Entity entity, const TShared<RendererScene>& rendererScene, const Transform& transform = Transform::kIdentity);

		/** Removes all ParticleSystem fragments. Cleanup (ID deallocation, dirty tags) is handled by the associated RendererScene when it is notified the fragment has been removed. */
		void DestroyParticleSystem(Registry& registry, Entity entity);

		/** ECS utility for ParticleSystem renderer objects. Provides fragment creation, renderer registration, dirty marking, and scene migration. */
		using ParticleSystemECSUtility = TRendererObjectECSUtility<
			ParticleSystemId,
			ParticleSystemDirty, ParticleSystemTransformDirty,
			&RendererScene::AllocateParticleSystemId, &RendererScene::DeallocateParticleSystemId,
			ParticleSystem, ParticleSimulation>;
	}

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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) ParticleSystem : public Component, public TParticleSystemGetters<ParticleSystem, false>, public CoreObject
	{
		friend class TParticleSystemGetters<ParticleSystem, false>;

	public:
		ParticleSystem(const HSceneObject& parent);
		virtual ~ParticleSystem() = default;

		/** @copydoc TParticleSystemGetters::GetSettings */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Settings), PassByCopy(true), UI(Inline))
		void SetSettings(const ParticleSystemSettings& settings);

		/** @copydoc TParticleSystemGetters::GetGpuSimulationSettings */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(GpuSimulationSettings), PassByCopy(true))
		void SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings);

		/**
		 * Set of objects that determine initial position, normal and other properties of newly spawned particles. Each
		 * particle system must have at least one emitter.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Emitters))
		void SetEmitters(const Vector<TShared<ParticleEmitter>>& emitters);

		/** @copydoc SetEmitters */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Emitters))
		const Vector<TShared<ParticleEmitter>>& GetEmitters() const;

		/**
		 * Set of objects that determine how particle properties change during their lifetime. Evolvers only affect
		 * CPU simulated particles.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Evolvers))
		void SetEvolvers(const Vector<TShared<ParticleEvolver>>& evolvers);

		/** @copydoc SetEvolvers */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Evolvers))
		const Vector<TShared<ParticleEvolver>>& GetEvolvers() const;

		/** @copydoc TParticleSystemGetters::GetLayer */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Layer), UI(AsLayerMask))
		void SetLayer(u64 layer);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Returns an ID that uniquely identifies the particle system. Can be used for locating evaluated particle
		 * system render data in the structure output by the ParticlesManager.
		 */
		u32 GetId() const { return GetFragment().Id; }

		/**
		 * Enables or disables preview mode. Preview mode allows the particle system to play while the game is not running,
		 * primarily for preview purposes in the editor. Returns true if the preview mode was enabled, false if it was
		 * disabled or enabling preview failed.
		 */
		B3D_SCRIPT_EXPORT(Visibility(Internal))
		bool TogglePreviewMode(bool enabled);

		/**
		 * Updates the particle simulation by advancing it by @p timeDelta. New state will be updated in the internal
		 * ParticleSet.
		 */
		void Simulate(float timeDelta, const EvaluatedAnimationData* animData);

		/**
		 * Calculates the bounds of all the particles in the system. Should be called after a call to SimulateInternal() to get
		 * up-to-date bounds. The bounds are in the simulation space of the particle system.
		 */
		AABox CalculateBounds() const;

		/**
		 * Advances the particle system time according to the current time, time delta and the provided settings.
		 *
		 * @param	time		Current time to use as a base.
		 * @param	timeDelta	Amount of time to advance the time by.
		 * @param	duration	Maximum time allowed by the particle system.
		 * @param	loop		Determines what happens when the time exceeds @p duration. If true the time will
		 *						wrap around to 0 and start over, if false the time will be clamped to @p
		 *						duration.
		 * @param	outTimeStep	Actual time-step the simulation was advanced by. This is normally equal to
		 *						@p timeDelta but might be a different value if time was clamped.
		 * @return				New time value.
		 */
		static float AdvanceTime(float time, float timeDelta, float duration, bool loop, float& outTimeStep);

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity) override;
		void OnTransformChanged(TransformChangedFlags flags) override;

	protected:
		friend class ParticleScene;

		/** Starts the particle system. New particles will be emitted and existing particles will be evolved. */
		void Play();

		/** Pauses the particle system. New particles will stop being emitted and existing particle state will be frozen. */
		void Pause();

		/** Stops the particle system and resets it to initial state, clearing all particles. */
		void Stop();

		void GetCoreDependencies(Vector<CoreObject*>& dependencies) override;

		bool mPreviewMode = false;

	private:
		/** Returns a reference to the particle system data for the CRTP getter interface. */
		const TParticleSystemData<false>& GetParticleSystemData() const;

		/** Returns a mutable reference to the particle system ECS fragment. */
		ecs::ParticleSystem& GetFragment();

		/** Returns a const reference to the particle system ECS fragment. */
		const ecs::ParticleSystem& GetFragment() const;

		/** Returns a mutable reference to the particle simulation ECS fragment. */
		ecs::ParticleSimulation& GetSimulationFragment();

		/** Returns a const reference to the particle simulation ECS fragment. */
		const ecs::ParticleSimulation& GetSimulationFragment() const;

		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ParticleSystemRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		ParticleSystem(); // Serialization only
	};

	/** @} */

	/** @addtogroup Renderer
	 *  @{
	 */

	namespace render
	{
		/** Render-thread representation of a particle system, stored in packed arrays in ParticleSystemObjectStorageBase. */
		class B3D_EXPORT ParticleSystemProxy : public TParticleSystemGetters<ParticleSystemProxy, true>
		{
			friend class TParticleSystemGetters<ParticleSystemProxy, true>;
			friend struct b3d::ParticleSystemFullUpdateChannel;
			friend struct b3d::ParticleSystemTransformUpdateChannel;

		public:
			/** Returns the world space transform of the particle system. */
			const Transform& GetWorldTransform() const { return mTransform; }

			/**
			 * Returns an ID that uniquely identifies the particle system. Can be used for locating evaluated particle
			 * system render data in the structure output by the ParticlesManager.
			 */
			u32 GetId() const { return mId; }

			/** Sets the renderer-assigned packed slot ID. */
			void SetRendererId(PackedRendererId id) { mRendererId = id; }

			/** Returns the renderer-assigned packed slot ID. */
			PackedRendererId GetRendererId() const { return mRendererId; }

		protected:
			/** Returns a reference to the particle system data for the CRTP getter interface. */
			const TParticleSystemData<true>& GetParticleSystemData() const { return mData; }

			TParticleSystemData<true> mData;
			Transform mTransform;
			u32 mId = 0;
			PackedRendererId mRendererId = kInvalidPackedRendererId;
		};
	} // namespace render

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Contains render thread representation of particle system objects, stored in packed arrays accessible by PackedRendererId.
	 * Implements main -> render thread synchronization logic for particle system objects.
	 */
	class B3D_EXPORT ParticleSystemObjectStorageBase : public RendererObjectStorage
	{
	public:
		void* SyncRead(ecs::Registry& registry, FrameAllocator& allocator) override;
		void SyncWrite(void* batchData, FrameAllocator& allocator) override;

		/** Returns the particle system proxy at the given slot. */
		render::ParticleSystemProxy& GetParticleSystemProxy(PackedRendererId slotId) { return mParticleSystemProxies[slotId]; }

		/** Returns the particle system proxy at the given slot (const). */
		const render::ParticleSystemProxy& GetParticleSystemProxy(PackedRendererId slotId) const { return mParticleSystemProxies[slotId]; }

		/** Returns the total number of particle systems. */
		u32 GetParticleSystemCount() const { return (u32)mParticleSystemProxies.size(); }

		/**
		 * Called once per frame for each new particle system being added, or a particle system whose data needs to be rebuilt
		 * after a significant update (in which case DestroyRenderState will be called first).
		 */
		virtual void CreateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each particle system that is being removed, or a particle system that needs to be rebuilt
		 * after a significant update (in which case CreateRenderState will be called right after).
		 */
		virtual void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each particle system that needs to be updated after a minor update (e.g. a transform change).
		 */
		virtual void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

	protected:
		TChunkedArray<render::ParticleSystemProxy> mParticleSystemProxies;
	};

	/** @} */
} // namespace b3d
