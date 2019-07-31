//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_COLOR_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "BsScriptTColorDistribution.generated.h"

namespace bs
{
	ScriptPARTICLE_COLOR_DESC::ScriptPARTICLE_COLOR_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_COLOR_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_COLOR_DESC::box(const __PARTICLE_COLOR_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_COLOR_DESCInterop ScriptPARTICLE_COLOR_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_COLOR_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_COLOR_DESC ScriptPARTICLE_COLOR_DESC::fromInterop(const __PARTICLE_COLOR_DESCInterop& value)
	{
		PARTICLE_COLOR_DESC output;
		SPtr<TColorDistribution<ColorGradient>> tmpcolor;
		ScriptTColorDistributionColorGradient* scriptcolor;
		scriptcolor = ScriptTColorDistributionColorGradient::toNative(value.color);
		if(scriptcolor != nullptr)
			tmpcolor = scriptcolor->getInternal();
		if(tmpcolor != nullptr)
		output.color = *tmpcolor;

		return output;
	}

	__PARTICLE_COLOR_DESCInterop ScriptPARTICLE_COLOR_DESC::toInterop(const PARTICLE_COLOR_DESC& value)
	{
		__PARTICLE_COLOR_DESCInterop output;
		MonoObject* tmpcolor;
		SPtr<TColorDistribution<ColorGradient>> tmpcolorcopy;
		tmpcolorcopy = bs_shared_ptr_new<TColorDistribution<ColorGradient>>(value.color);
		tmpcolor = ScriptTColorDistributionColorGradient::create(tmpcolorcopy);
		output.color = tmpcolor;

		return output;
	}

}
