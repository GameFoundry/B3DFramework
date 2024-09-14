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
	ScriptParticleRotationOptions::ScriptParticleRotationOptions()
	{ }

	MonoObject* ScriptParticleRotationOptions::Box(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PARTICLE_ROTATION_DESCInterop ScriptParticleRotationOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_ROTATION_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_ROTATION_DESC ScriptParticleRotationOptions::FromInterop(const __PARTICLE_ROTATION_DESCInterop& value)
	{
		PARTICLE_ROTATION_DESC output;
		SPtr<TDistribution<float>> tmpRotation;
		ScriptFloatDistribution* scriptWrapperObjectRotation;
		scriptWrapperObjectRotation = ScriptFloatDistribution::GetScriptObjectWrapper(value.Rotation);
		if(scriptWrapperObjectRotation != nullptr)
			tmpRotation = std::static_pointer_cast<TDistribution<float>>(scriptWrapperObjectRotation->GetBaseNativeObjectAsShared());
		if(tmpRotation != nullptr)
		output.Rotation = *tmpRotation;
		SPtr<TDistribution<TVector3<float>>> tmpRotation3D;
		ScriptVector3Distribution* scriptWrapperObjectRotation3D;
		scriptWrapperObjectRotation3D = ScriptVector3Distribution::GetScriptObjectWrapper(value.Rotation3D);
		if(scriptWrapperObjectRotation3D != nullptr)
			tmpRotation3D = std::static_pointer_cast<TDistribution<TVector3<float>>>(scriptWrapperObjectRotation3D->GetBaseNativeObjectAsShared());
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
		tmpRotation = ScriptFloatDistribution::GetOrCreateScriptObject(tmpRotationcopy);
		output.Rotation = tmpRotation;
		MonoObject* tmpRotation3D;
		SPtr<TDistribution<TVector3<float>>> tmpRotation3Dcopy;
		tmpRotation3Dcopy = B3DMakeShared<TDistribution<TVector3<float>>>(value.Rotation3D);
		tmpRotation3D = ScriptVector3Distribution::GetOrCreateScriptObject(tmpRotation3Dcopy);
		output.Rotation3D = tmpRotation3D;
		output.Use3DRotation = value.Use3DRotation;

		return output;
	}

}
