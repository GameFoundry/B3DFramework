//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_ORBIT_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_ORBIT_DESC::ScriptPARTICLE_ORBIT_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_ORBIT_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_ORBIT_DESC::Box(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_ORBIT_DESCInterop ScriptPARTICLE_ORBIT_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ORBIT_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ORBIT_DESC ScriptPARTICLE_ORBIT_DESC::FromInterop(const __PARTICLE_ORBIT_DESCInterop& value)
	{
		PARTICLE_ORBIT_DESC output;
		SPtr<TDistribution<Vector3>> tmpCenter;
		ScriptTDistributionVector3* scriptCenter;
		scriptCenter = ScriptTDistributionVector3::ToNative(value.Center);
		if(scriptCenter != nullptr)
			tmpCenter = scriptCenter->GetInternal();
		if(tmpCenter != nullptr)
			output.Center = *tmpCenter;
		SPtr<TDistribution<Vector3>> tmpVelocity;
		ScriptTDistributionVector3* scriptVelocity;
		scriptVelocity = ScriptTDistributionVector3::ToNative(value.Velocity);
		if(scriptVelocity != nullptr)
			tmpVelocity = scriptVelocity->GetInternal();
		if(tmpVelocity != nullptr)
			output.Velocity = *tmpVelocity;
		SPtr<TDistribution<float>> tmpRadial;
		ScriptTDistributionfloat* scriptRadial;
		scriptRadial = ScriptTDistributionfloat::ToNative(value.Radial);
		if(scriptRadial != nullptr)
			tmpRadial = scriptRadial->GetInternal();
		if(tmpRadial != nullptr)
			output.Radial = *tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

	__PARTICLE_ORBIT_DESCInterop ScriptPARTICLE_ORBIT_DESC::ToInterop(const PARTICLE_ORBIT_DESC& value)
	{
		__PARTICLE_ORBIT_DESCInterop output;
		MonoObject* tmpCenter;
		SPtr<TDistribution<Vector3>> tmpCentercopy;
		tmpCentercopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Center);
		tmpCenter = ScriptTDistributionVector3::Create(tmpCentercopy);
		output.Center = tmpCenter;
		MonoObject* tmpVelocity;
		SPtr<TDistribution<Vector3>> tmpVelocitycopy;
		tmpVelocitycopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Velocity);
		tmpVelocity = ScriptTDistributionVector3::Create(tmpVelocitycopy);
		output.Velocity = tmpVelocity;
		MonoObject* tmpRadial;
		SPtr<TDistribution<float>> tmpRadialcopy;
		tmpRadialcopy = bs_shared_ptr_new<TDistribution<float>>(value.Radial);
		tmpRadial = ScriptTDistributionfloat::Create(tmpRadialcopy);
		output.Radial = tmpRadial;
		output.WorldSpace = value.WorldSpace;

		return output;
	}

} // namespace bs
