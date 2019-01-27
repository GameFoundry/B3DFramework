//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_CIRCLE_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmissionMode.generated.h"

namespace bs
{
	ScriptPARTICLE_CIRCLE_SHAPE_DESC::ScriptPARTICLE_CIRCLE_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_CIRCLE_SHAPE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_CIRCLE_SHAPE_DESC::box(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_CIRCLE_SHAPE_DESCInterop ScriptPARTICLE_CIRCLE_SHAPE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_CIRCLE_SHAPE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_CIRCLE_SHAPE_DESC ScriptPARTICLE_CIRCLE_SHAPE_DESC::fromInterop(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value)
	{
		PARTICLE_CIRCLE_SHAPE_DESC output;
		output.radius = value.radius;
		output.thickness = value.thickness;
		output.arc = value.arc;
		output.mode = value.mode;

		return output;
	}

	__PARTICLE_CIRCLE_SHAPE_DESCInterop ScriptPARTICLE_CIRCLE_SHAPE_DESC::toInterop(const PARTICLE_CIRCLE_SHAPE_DESC& value)
	{
		__PARTICLE_CIRCLE_SHAPE_DESCInterop output;
		output.radius = value.radius;
		output.thickness = value.thickness;
		output.arc = value.arc;
		output.mode = value.mode;

		return output;
	}

}
