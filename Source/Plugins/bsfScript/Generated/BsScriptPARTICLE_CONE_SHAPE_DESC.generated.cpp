//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_CONE_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmissionMode.generated.h"

namespace bs
{
	ScriptPARTICLE_CONE_SHAPE_DESC::ScriptPARTICLE_CONE_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_CONE_SHAPE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_CONE_SHAPE_DESC::box(const __PARTICLE_CONE_SHAPE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_CONE_SHAPE_DESCInterop ScriptPARTICLE_CONE_SHAPE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_CONE_SHAPE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_CONE_SHAPE_DESC ScriptPARTICLE_CONE_SHAPE_DESC::fromInterop(const __PARTICLE_CONE_SHAPE_DESCInterop& value)
	{
		PARTICLE_CONE_SHAPE_DESC output;
		output.type = value.type;
		output.radius = value.radius;
		output.angle = value.angle;
		output.length = value.length;
		output.thickness = value.thickness;
		output.arc = value.arc;
		output.mode = value.mode;

		return output;
	}

	__PARTICLE_CONE_SHAPE_DESCInterop ScriptPARTICLE_CONE_SHAPE_DESC::toInterop(const PARTICLE_CONE_SHAPE_DESC& value)
	{
		__PARTICLE_CONE_SHAPE_DESCInterop output;
		output.type = value.type;
		output.radius = value.radius;
		output.angle = value.angle;
		output.length = value.length;
		output.thickness = value.thickness;
		output.arc = value.arc;
		output.mode = value.mode;

		return output;
	}

}
