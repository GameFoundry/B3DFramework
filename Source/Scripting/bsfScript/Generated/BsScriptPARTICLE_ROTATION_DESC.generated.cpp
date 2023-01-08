//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	ScriptParticleRotationOptions::ScriptParticleRotationOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleRotationOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleRotationOptions::Box(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_ROTATION_DESCInterop ScriptParticleRotationOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ROTATION_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ROTATION_DESC ScriptParticleRotationOptions::FromInterop(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		PARTICLE_ROTATION_DESC output;
		SPtr<TDistribution<float>> tmpRotation;
		ScriptFloatDistribution* scriptRotation;
		scriptRotation = ScriptFloatDistribution::ToNative(value.Rotation);
		if(scriptRotation != nullptr)
			tmpRotation = scriptRotation->GetInternal();
		if(tmpRotation != nullptr)
		output.Rotation = *tmpRotation;
		SPtr<TDistribution<Vector3>> tmpRotation3D;
		ScriptVector3Distribution* scriptRotation3D;
		scriptRotation3D = ScriptVector3Distribution::ToNative(value.Rotation3D);
		if(scriptRotation3D != nullptr)
			tmpRotation3D = scriptRotation3D->GetInternal();
		if(tmpRotation3D != nullptr)
		output.Rotation3D = *tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

	__PARTICLE_ROTATION_DESCInterop ScriptParticleRotationOptions::ToInterop(const PARTICLE_ROTATION_DESC& value)
	{
		__PARTICLE_ROTATION_DESCInterop output;
		MonoObject* tmpRotation;
		SPtr<TDistribution<float>> tmpRotationcopy;
		tmpRotationcopy = B3DMakeShared<TDistribution<float>>(value.Rotation);
		tmpRotation = ScriptFloatDistribution::Create(tmpRotationcopy);
		output.Rotation = tmpRotation;
		MonoObject* tmpRotation3D;
		SPtr<TDistribution<Vector3>> tmpRotation3Dcopy;
		tmpRotation3Dcopy = B3DMakeShared<TDistribution<Vector3>>(value.Rotation3D);
		tmpRotation3D = ScriptVector3Distribution::Create(tmpRotation3Dcopy);
		output.Rotation3D = tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

}
