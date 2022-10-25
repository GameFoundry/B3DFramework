//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_FORCE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_FORCE_DESC::ScriptPARTICLE_FORCE_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_FORCE_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_FORCE_DESC::Box(const __PARTICLE_FORCE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_FORCE_DESCInterop ScriptPARTICLE_FORCE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_FORCE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_FORCE_DESC ScriptPARTICLE_FORCE_DESC::FromInterop(const __PARTICLE_FORCE_DESCInterop& value)
	{
		PARTICLE_FORCE_DESC output;
		SPtr<TDistribution<Vector3>> tmpForce;
		ScriptTDistributionVector3* scriptForce;
		scriptForce = ScriptTDistributionVector3::ToNative(value.Force);
		if(scriptForce != nullptr)
			tmpForce = scriptForce->GetInternal();
		if(tmpForce != nullptr)
			output.Force = *tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_FORCE_DESCInterop ScriptPARTICLE_FORCE_DESC::ToInterop(const PARTICLE_FORCE_DESC& value)
	{
		__PARTICLE_FORCE_DESCInterop output;
		MonoObject* tmpForce;
		SPtr<TDistribution<Vector3>> tmpForcecopy;
		tmpForcecopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Force);
		tmpForce = ScriptTDistributionVector3::Create(tmpForcecopy);
		output.Force = tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

} // namespace bs
