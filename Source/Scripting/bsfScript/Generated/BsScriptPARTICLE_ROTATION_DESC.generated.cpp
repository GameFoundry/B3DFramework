//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_ROTATION_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_ROTATION_DESC::ScriptPARTICLE_ROTATION_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_ROTATION_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_ROTATION_DESC::Box(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_ROTATION_DESCInterop ScriptPARTICLE_ROTATION_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ROTATION_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ROTATION_DESC ScriptPARTICLE_ROTATION_DESC::FromInterop(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		PARTICLE_ROTATION_DESC output;
		SPtr<TDistribution<float>> tmpRotation;
		ScriptTDistributionfloat* scriptRotation;
		scriptRotation = ScriptTDistributionfloat::ToNative(value.Rotation);
		if(scriptRotation != nullptr)
			tmpRotation = scriptRotation->GetInternal();
		if(tmpRotation != nullptr)
			output.Rotation = *tmpRotation;
		SPtr<TDistribution<Vector3>> tmpRotation3D;
		ScriptTDistributionVector3* scriptRotation3D;
		scriptRotation3D = ScriptTDistributionVector3::ToNative(value.Rotation3D);
		if(scriptRotation3D != nullptr)
			tmpRotation3D = scriptRotation3D->GetInternal();
		if(tmpRotation3D != nullptr)
			output.Rotation3D = *tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

	__PARTICLE_ROTATION_DESCInterop ScriptPARTICLE_ROTATION_DESC::ToInterop(const PARTICLE_ROTATION_DESC& value)
	{
		__PARTICLE_ROTATION_DESCInterop output;
		MonoObject* tmpRotation;
		SPtr<TDistribution<float>> tmpRotationcopy;
		tmpRotationcopy = bs_shared_ptr_new<TDistribution<float>>(value.Rotation);
		tmpRotation = ScriptTDistributionfloat::Create(tmpRotationcopy);
		output.Rotation = tmpRotation;
		MonoObject* tmpRotation3D;
		SPtr<TDistribution<Vector3>> tmpRotation3Dcopy;
		tmpRotation3Dcopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Rotation3D);
		tmpRotation3D = ScriptTDistributionVector3::Create(tmpRotation3Dcopy);
		output.Rotation3D = tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

} // namespace bs
