//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_SIZE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_SIZE_DESC::ScriptPARTICLE_SIZE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_SIZE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_SIZE_DESC::box(const __PARTICLE_SIZE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_SIZE_DESCInterop ScriptPARTICLE_SIZE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_SIZE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_SIZE_DESC ScriptPARTICLE_SIZE_DESC::fromInterop(const __PARTICLE_SIZE_DESCInterop& value)
	{
		PARTICLE_SIZE_DESC output;
		SPtr<TDistribution<float>> tmpsize;
		ScriptTDistributionfloat* scriptsize;
		scriptsize = ScriptTDistributionfloat::toNative(value.size);
		if(scriptsize != nullptr)
			tmpsize = scriptsize->getInternal();
		if(tmpsize != nullptr)
		output.size = *tmpsize;
		SPtr<TDistribution<Vector3>> tmpsize3D;
		ScriptTDistributionVector3* scriptsize3D;
		scriptsize3D = ScriptTDistributionVector3::toNative(value.size3D);
		if(scriptsize3D != nullptr)
			tmpsize3D = scriptsize3D->getInternal();
		if(tmpsize3D != nullptr)
		output.size3D = *tmpsize3D;
		output.use3DSize = value.use3DSize;

		return output;
	}

	__PARTICLE_SIZE_DESCInterop ScriptPARTICLE_SIZE_DESC::toInterop(const PARTICLE_SIZE_DESC& value)
	{
		__PARTICLE_SIZE_DESCInterop output;
		MonoObject* tmpsize;
		SPtr<TDistribution<float>> tmpsizecopy;
		tmpsizecopy = bs_shared_ptr_new<TDistribution<float>>(value.size);
		tmpsize = ScriptTDistributionfloat::create(tmpsizecopy);
		output.size = tmpsize;
		MonoObject* tmpsize3D;
		SPtr<TDistribution<Vector3>> tmpsize3Dcopy;
		tmpsize3Dcopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.size3D);
		tmpsize3D = ScriptTDistributionVector3::create(tmpsize3Dcopy);
		output.size3D = tmpsize3D;
		output.use3DSize = value.use3DSize;

		return output;
	}

}
