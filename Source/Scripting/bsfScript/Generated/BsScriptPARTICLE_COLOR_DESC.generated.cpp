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
	ScriptParticleColorOptions::ScriptParticleColorOptions()
	{ }

	MonoObject* ScriptParticleColorOptions::Box(const __PARTICLE_COLOR_DESCInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PARTICLE_COLOR_DESCInterop ScriptParticleColorOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_COLOR_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_COLOR_DESC ScriptParticleColorOptions::FromInterop(const __PARTICLE_COLOR_DESCInterop& value)
	{
		PARTICLE_COLOR_DESC output;
		SPtr<TColorDistribution<ColorGradient>> tmpColor;
		ScriptColorDistribution* scriptWrapperObjectColor;
		scriptWrapperObjectColor = ScriptColorDistribution::GetScriptObjectWrapper(value.Color);
		if(scriptWrapperObjectColor != nullptr)
			tmpColor = std::static_pointer_cast<TColorDistribution<ColorGradient>>(scriptWrapperObjectColor->GetBaseNativeObjectAsShared());
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
		tmpColor = ScriptColorDistribution::GetOrCreateScriptObject(tmpColorcopy);
		output.Color = tmpColor;

		return output;
	}

}
