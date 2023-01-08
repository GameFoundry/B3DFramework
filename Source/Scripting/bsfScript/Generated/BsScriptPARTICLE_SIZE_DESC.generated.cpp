//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_SIZE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTDistribution.generated.h"

namespace bs
{
	ScriptParticleSizeOptions::ScriptParticleSizeOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleSizeOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleSizeOptions::Box(const __PARTICLE_SIZE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_SIZE_DESCInterop ScriptParticleSizeOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_SIZE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_SIZE_DESC ScriptParticleSizeOptions::FromInterop(const __PARTICLE_SIZE_DESCInterop& value)
	{
		PARTICLE_SIZE_DESC output;
		SPtr<TDistribution<float>> tmpSize;
		ScriptFloatDistribution* scriptSize;
		scriptSize = ScriptFloatDistribution::ToNative(value.Size);
		if(scriptSize != nullptr)
			tmpSize = scriptSize->GetInternal();
		if(tmpSize != nullptr)
		output.Size = *tmpSize;
		SPtr<TDistribution<Vector3>> tmpSize3D;
		ScriptVector3Distribution* scriptSize3D;
		scriptSize3D = ScriptVector3Distribution::ToNative(value.Size3D);
		if(scriptSize3D != nullptr)
			tmpSize3D = scriptSize3D->GetInternal();
		if(tmpSize3D != nullptr)
		output.Size3D = *tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

	__PARTICLE_SIZE_DESCInterop ScriptParticleSizeOptions::ToInterop(const PARTICLE_SIZE_DESC& value)
	{
		__PARTICLE_SIZE_DESCInterop output;
		MonoObject* tmpSize;
		SPtr<TDistribution<float>> tmpSizecopy;
		tmpSizecopy = B3DMakeShared<TDistribution<float>>(value.Size);
		tmpSize = ScriptFloatDistribution::Create(tmpSizecopy);
		output.Size = tmpSize;
		MonoObject* tmpSize3D;
		SPtr<TDistribution<Vector3>> tmpSize3Dcopy;
		tmpSize3Dcopy = B3DMakeShared<TDistribution<Vector3>>(value.Size3D);
		tmpSize3D = ScriptVector3Distribution::Create(tmpSize3Dcopy);
		output.Size3D = tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

}
