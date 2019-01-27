//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_ROTATION_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_ROTATION_DESC::ScriptPARTICLE_ROTATION_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_ROTATION_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_ROTATION_DESC::box(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_ROTATION_DESCInterop ScriptPARTICLE_ROTATION_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_ROTATION_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_ROTATION_DESC ScriptPARTICLE_ROTATION_DESC::fromInterop(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		PARTICLE_ROTATION_DESC output;
		SPtr<TDistribution<float>> tmprotation;
		ScriptTDistributionfloat* scriptrotation;
		scriptrotation = ScriptTDistributionfloat::toNative(value.rotation);
		if(scriptrotation != nullptr)
			tmprotation = scriptrotation->getInternal();
		if(tmprotation != nullptr)
		output.rotation = *tmprotation;
		SPtr<TDistribution<Vector3>> tmprotation3D;
		ScriptTDistributionVector3* scriptrotation3D;
		scriptrotation3D = ScriptTDistributionVector3::toNative(value.rotation3D);
		if(scriptrotation3D != nullptr)
			tmprotation3D = scriptrotation3D->getInternal();
		if(tmprotation3D != nullptr)
		output.rotation3D = *tmprotation3D;
		output.use3DRotation = value.use3DRotation;

		return output;
	}

	__PARTICLE_ROTATION_DESCInterop ScriptPARTICLE_ROTATION_DESC::toInterop(const PARTICLE_ROTATION_DESC& value)
	{
		__PARTICLE_ROTATION_DESCInterop output;
		MonoObject* tmprotation;
		SPtr<TDistribution<float>> tmprotationcopy;
		tmprotationcopy = bs_shared_ptr_new<TDistribution<float>>(value.rotation);
		tmprotation = ScriptTDistributionfloat::create(tmprotationcopy);
		output.rotation = tmprotation;
		MonoObject* tmprotation3D;
		SPtr<TDistribution<Vector3>> tmprotation3Dcopy;
		tmprotation3Dcopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.rotation3D);
		tmprotation3D = ScriptTDistributionVector3::create(tmprotation3Dcopy);
		output.rotation3D = tmprotation3D;
		output.use3DRotation = value.use3DRotation;

		return output;
	}

}
