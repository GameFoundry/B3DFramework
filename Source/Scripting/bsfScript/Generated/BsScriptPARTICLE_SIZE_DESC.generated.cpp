//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	ScriptPARTICLE_SIZE_DESC::ScriptPARTICLE_SIZE_DESC(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPARTICLE_SIZE_DESC::InitRuntimeData()
	{}

	MonoObject* ScriptPARTICLE_SIZE_DESC::Box(const __PARTICLE_SIZE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_SIZE_DESCInterop ScriptPARTICLE_SIZE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_SIZE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_SIZE_DESC ScriptPARTICLE_SIZE_DESC::FromInterop(const __PARTICLE_SIZE_DESCInterop& value)
	{
		PARTICLE_SIZE_DESC output;
		SPtr<TDistribution<float>> tmpSize;
		ScriptTDistributionfloat* scriptSize;
		scriptSize = ScriptTDistributionfloat::ToNative(value.Size);
		if(scriptSize != nullptr)
			tmpSize = scriptSize->GetInternal();
		if(tmpSize != nullptr)
			output.Size = *tmpSize;
		SPtr<TDistribution<Vector3>> tmpSize3D;
		ScriptTDistributionVector3* scriptSize3D;
		scriptSize3D = ScriptTDistributionVector3::ToNative(value.Size3D);
		if(scriptSize3D != nullptr)
			tmpSize3D = scriptSize3D->GetInternal();
		if(tmpSize3D != nullptr)
			output.Size3D = *tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

	__PARTICLE_SIZE_DESCInterop ScriptPARTICLE_SIZE_DESC::ToInterop(const PARTICLE_SIZE_DESC& value)
	{
		__PARTICLE_SIZE_DESCInterop output;
		MonoObject* tmpSize;
		SPtr<TDistribution<float>> tmpSizecopy;
		tmpSizecopy = bs_shared_ptr_new<TDistribution<float>>(value.Size);
		tmpSize = ScriptTDistributionfloat::Create(tmpSizecopy);
		output.Size = tmpSize;
		MonoObject* tmpSize3D;
		SPtr<TDistribution<Vector3>> tmpSize3Dcopy;
		tmpSize3Dcopy = bs_shared_ptr_new<TDistribution<Vector3>>(value.Size3D);
		tmpSize3D = ScriptTDistributionVector3::Create(tmpSize3Dcopy);
		output.Size3D = tmpSize3D;
		output.Use3DSize = value.Use3DSize;

		return output;
	}

} // namespace bs
