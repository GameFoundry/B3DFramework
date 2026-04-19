//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	class ParticleScene;
	class ParticleSet;
	class ParticleEvolver;
	struct EvaluatedAnimationData;

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Contains particle system state that varies from frame to frame. */
	struct ParticleSystemState
	{
		float TimeStart;
		float TimeEnd;
		float NrmTimeStart;
		float NrmTimeEnd;
		float Length;
		float TimeStep;
		u32 MaxParticles;
		bool WorldSpace;
		bool GpuSimulated;
		Matrix4 LocalToWorld;
		Matrix4 WorldToLocal;

		/** Material used by the particle system, for evolvers that need material access. */
		HMaterial Material;

		/** Pointer to the owning particle scene, for emitter sub-stepping callbacks. */
		ParticleScene* ParticleScene;

		/** Pointer to the scene instance, for physics queries. */
		const SceneInstance* Scene;

		/** Animation data for the current frame. */
		const EvaluatedAnimationData* AnimData;

		/** Active particle set, cached per-frame for simulation methods. */
		ParticleSet* Particles;

		/** Sorted evolver list, cached per-frame for simulation methods. */
		const Vector<SPtr<ParticleEvolver>>* Evolvers;

		/** Random number generator from the simulation fragment. */
		Random* Rng;
	};

	/** Module that in some way modified or effects a ParticleSystem. */
	class B3D_EXPORT ParticleModule : public IReflectable, public IScriptExportable, INonCopyable
	{
	public:
		ParticleModule(const ParticleModule&) = delete;
		ParticleModule& operator=(const ParticleModule&) = delete;

		ParticleModule(ParticleModule&&) = delete;
		ParticleModule& operator=(ParticleModule&&) = delete;

	protected:
		friend class ParticleSystem;
		friend class ParticleScene;

		ParticleModule() = default;
		virtual ~ParticleModule() = default;
	};

	/** @} */
} // namespace b3d
