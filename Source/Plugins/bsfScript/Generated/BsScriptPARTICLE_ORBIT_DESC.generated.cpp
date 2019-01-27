//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_ORBIT_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_ORBIT_DESC::ScriptPARTICLE_ORBIT_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_ORBIT_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_ORBIT_DESC::box(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_ORBIT_DESCInterop ScriptPARTICLE_ORBIT_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_ORBIT_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_ORBIT_DESC ScriptPARTICLE_ORBIT_DESC::fromInterop(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		PARTICLE_ORBIT_DESC output;
		SPtr<TDistribution<Vector3>> tmpcenter;
		ScriptTDistributionVector3* scriptcenter;
		scriptcenter = ScriptTDistributionVector3::toNative(value.center);
		if(scriptcenter != nullptr)
			tmpcenter = scriptcenter->getInternal();
		if(tmpcenter != nullptr)
		output.center = *tmpcenter;
		SPtr<TDistribution<Vector3>> tmpvelocity;
		ScriptTDistributionVector3* scriptvelocity;
		scriptvelocity = ScriptTDistributionVector3::toNative(value.velocity);
		if(scriptvelocity != nullptr)
			tmpvelocity = scriptvelocity->getInternal();
		if(tmpvelocity != nullptr)
		output.velocity = *tmpvelocity;
		SPtr<TDistribution<float>> tmpradial;
		ScriptTDistributionfloat* scriptradial;
		scriptradial = ScriptTDistributionfloat::toNative(value.radial);
		if(scriptradial != nullptr)
			tmpradial = scriptradial->getInternal();
		if(tmpradial != nullptr)
		output.radial = *tmpradial;
		output.worldSpace = value.worldSpace;

		return output;
	}

	__PARTICLE_ORBIT_DESCInterop ScriptPARTICLE_ORBIT_DESC::toInterop(const PARTICLE_ORBIT_DESC& value)
	{
		__PARTICLE_ORBIT_DESCInterop output;
		MonoObject* tmpcenter;
		SPtr<TDistribution<Vector3>> tmpcentercopy;
		tmpcentercopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.center);
		tmpcenter = ScriptTDistributionVector3::create(tmpcentercopy);
		output.center = tmpcenter;
		MonoObject* tmpvelocity;
		SPtr<TDistribution<Vector3>> tmpvelocitycopy;
		tmpvelocitycopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.velocity);
		tmpvelocity = ScriptTDistributionVector3::create(tmpvelocitycopy);
		output.velocity = tmpvelocity;
		MonoObject* tmpradial;
		SPtr<TDistribution<float>> tmpradialcopy;
		tmpradialcopy = bs_shared_ptr_new<TDistribution<float>>(value.radial);
		tmpradial = ScriptTDistributionfloat::create(tmpradialcopy);
		output.radial = tmpradial;
		output.worldSpace = value.worldSpace;

		return output;
	}

}
