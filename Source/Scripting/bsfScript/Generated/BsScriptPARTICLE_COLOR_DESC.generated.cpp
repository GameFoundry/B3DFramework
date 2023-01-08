//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_COLOR_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTColorDistribution.generated.h"

namespace bs
{
	ScriptParticleColorOptions::ScriptParticleColorOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleColorOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleColorOptions::Box(const __PARTICLE_COLOR_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_COLOR_DESCInterop ScriptParticleColorOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_COLOR_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_COLOR_DESC ScriptParticleColorOptions::FromInterop(const __PARTICLE_COLOR_DESCInterop& value)
	{
		PARTICLE_COLOR_DESC output;
		SPtr<TColorDistribution<ColorGradient>> tmpColor;
		ScriptColorDistribution* scriptColor;
		scriptColor = ScriptColorDistribution::ToNative(value.Color);
		if(scriptColor != nullptr)
			tmpColor = scriptColor->GetInternal();
		if(tmpColor != nullptr)
		output.Color = *tmpColor;

		return output;
	}

	__PARTICLE_COLOR_DESCInterop ScriptParticleColorOptions::ToInterop(const PARTICLE_COLOR_DESC& value)
	{
		__PARTICLE_COLOR_DESCInterop output;
		MonoObject* tmpColor;
		SPtr<TColorDistribution<ColorGradient>> tmpColorcopy;
		tmpColorcopy = B3DMakeShared<TColorDistribution<ColorGradient>>(value.Color);
		tmpColor = ScriptColorDistribution::Create(tmpColorcopy);
		output.Color = tmpColor;

		return output;
	}

}
