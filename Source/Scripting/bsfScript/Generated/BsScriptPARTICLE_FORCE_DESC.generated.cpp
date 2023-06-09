//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_FORCE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleForceOptions::ScriptParticleForceOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleForceOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleForceOptions::Box(const __PARTICLE_FORCE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_FORCE_DESCInterop ScriptParticleForceOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_FORCE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_FORCE_DESC ScriptParticleForceOptions::FromInterop(const __PARTICLE_FORCE_DESCInterop& value)
	{
		PARTICLE_FORCE_DESC output;
		SPtr<TDistribution<TVector3<float>>> tmpForce;
		ScriptVector3Distribution* scriptForce;
		scriptForce = ScriptVector3Distribution::ToNative(value.Force);
		if(scriptForce != nullptr)
			tmpForce = scriptForce->GetInternal();
		if(tmpForce != nullptr)
		output.Force = *tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_FORCE_DESCInterop ScriptParticleForceOptions::ToInterop(const PARTICLE_FORCE_DESC& value)
	{
		__PARTICLE_FORCE_DESCInterop output;
		MonoObject* tmpForce;
		SPtr<TDistribution<TVector3<float>>> tmpForcecopy;
		tmpForcecopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Force);
		tmpForce = ScriptVector3Distribution::Create(tmpForcecopy);
		output.Force = tmpForce;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

}
