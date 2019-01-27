//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_VELOCITY_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_VELOCITY_DESC::ScriptPARTICLE_VELOCITY_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_VELOCITY_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_VELOCITY_DESC::box(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptPARTICLE_VELOCITY_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_VELOCITY_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_VELOCITY_DESC ScriptPARTICLE_VELOCITY_DESC::fromInterop(const __PARTICLE_VELOCITY_DESCInterop& value)
	{
		PARTICLE_VELOCITY_DESC output;
		SPtr<TDistribution<Vector3>> tmpvelocity;
		ScriptTDistributionVector3* scriptvelocity;
		scriptvelocity = ScriptTDistributionVector3::toNative(value.velocity);
		if(scriptvelocity != nullptr)
			tmpvelocity = scriptvelocity->getInternal();
		if(tmpvelocity != nullptr)
		output.velocity = *tmpvelocity;
		output.worldSpace = value.worldSpace;

		return output;
	}

	__PARTICLE_VELOCITY_DESCInterop ScriptPARTICLE_VELOCITY_DESC::toInterop(const PARTICLE_VELOCITY_DESC& value)
	{
		__PARTICLE_VELOCITY_DESCInterop output;
		MonoObject* tmpvelocity;
		SPtr<TDistribution<Vector3>> tmpvelocitycopy;
		tmpvelocitycopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.velocity);
		tmpvelocity = ScriptTDistributionVector3::create(tmpvelocitycopy);
		output.velocity = tmpvelocity;
		output.worldSpace = value.worldSpace;

		return output;
	}

}
