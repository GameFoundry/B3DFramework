//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_VELOCITY_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_VELOCITY_DESC::ScriptPARTICLE_VELOCITY_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_VELOCITY_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_VELOCITY_DESC::Box(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptPARTICLE_VELOCITY_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_VELOCITY_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_VELOCITY_DESC ScriptPARTICLE_VELOCITY_DESC::FromInterop(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		PARTICLE_VELOCITY_DESC output;
		SPtr<TDistribution<Vector3>> tmpVelocity;
		ScriptTDistributionVector3* scriptVelocity;
		scriptVelocity = ScriptTDistributionVector3::ToNative(value.Velocity);
		if(scriptVelocity != nullptr)
			tmpVelocity = scriptVelocity->GetInternal();
		if(tmpVelocity != nullptr)
			output.Velocity = *tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptPARTICLE_VELOCITY_DESC::ToInterop(const PARTICLE_VELOCITY_DESC& value)
	{
		__PARTICLE_VELOCITY_DESCInterop output;
		MonoObject* tmpVelocity;
		SPtr<TDistribution<Vector3>> tmpVelocitycopy;
		tmpVelocitycopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Velocity);
		tmpVelocity = ScriptTDistributionVector3::Create(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

} // namespace bs
