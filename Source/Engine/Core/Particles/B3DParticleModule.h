//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
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
		ParticleSystem* System;
		const SceneInstance* Scene;
		const EvaluatedAnimationData* AnimData;
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

		ParticleModule() = default;
		virtual ~ParticleModule() = default;
	};

	/** @} */
} // namespace b3d
