//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_FORCE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_FORCE_DESC::ScriptPARTICLE_FORCE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_FORCE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_FORCE_DESC::box(const __PARTICLE_FORCE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_FORCE_DESCInterop ScriptPARTICLE_FORCE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_FORCE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_FORCE_DESC ScriptPARTICLE_FORCE_DESC::fromInterop(const __PARTICLE_FORCE_DESCInterop& value)
	{
		PARTICLE_FORCE_DESC output;
		SPtr<TDistribution<Vector3>> tmpforce;
		ScriptTDistributionVector3* scriptforce;
		scriptforce = ScriptTDistributionVector3::toNative(value.force);
		if(scriptforce != nullptr)
			tmpforce = scriptforce->getInternal();
		if(tmpforce != nullptr)
		output.force = *tmpforce;
		output.worldSpace = value.worldSpace;

		return output;
	}

	__PARTICLE_FORCE_DESCInterop ScriptPARTICLE_FORCE_DESC::toInterop(const PARTICLE_FORCE_DESC& value)
	{
		__PARTICLE_FORCE_DESCInterop output;
		MonoObject* tmpforce;
		SPtr<TDistribution<Vector3>> tmpforcecopy;
		tmpforcecopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.force);
		tmpforce = ScriptTDistributionVector3::create(tmpforcecopy);
		output.force = tmpforce;
		output.worldSpace = value.worldSpace;

		return output;
	}

}
