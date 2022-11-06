//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_CIRCLE_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmissionMode.generated.h"

namespace bs
{
	ScriptPARTICLE_CIRCLE_SHAPE_DESC::ScriptPARTICLE_CIRCLE_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_CIRCLE_SHAPE_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_CIRCLE_SHAPE_DESC::Box(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_CIRCLE_SHAPE_DESCInterop ScriptPARTICLE_CIRCLE_SHAPE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_CIRCLE_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_CIRCLE_SHAPE_DESC ScriptPARTICLE_CIRCLE_SHAPE_DESC::FromInterop(const __PARTICLE_CIRCLE_SHAPE_DESCInterop& value)
	{
		PARTICLE_CIRCLE_SHAPE_DESC output;
		output.Radius = value.Radius;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

	__PARTICLE_CIRCLE_SHAPE_DESCInterop ScriptPARTICLE_CIRCLE_SHAPE_DESC::ToInterop(const PARTICLE_CIRCLE_SHAPE_DESC& value)
	{
		__PARTICLE_CIRCLE_SHAPE_DESCInterop output;
		output.Radius = value.Radius;
		output.Thickness = value.Thickness;
		output.Arc = value.Arc;
		output.Mode = value.Mode;

		return output;
	}

}
