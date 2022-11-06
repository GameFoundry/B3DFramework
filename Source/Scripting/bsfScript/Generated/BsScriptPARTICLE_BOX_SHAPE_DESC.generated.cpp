//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_BOX_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptPARTICLE_BOX_SHAPE_DESC::ScriptPARTICLE_BOX_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_BOX_SHAPE_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_BOX_SHAPE_DESC::Box(const __PARTICLE_BOX_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_BOX_SHAPE_DESCInterop ScriptPARTICLE_BOX_SHAPE_DESC::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_BOX_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_BOX_SHAPE_DESC ScriptPARTICLE_BOX_SHAPE_DESC::FromInterop(const __PARTICLE_BOX_SHAPE_DESCInterop& value)
	{
		PARTICLE_BOX_SHAPE_DESC output;
		output.Type = value.Type;
		output.Extents = value.Extents;

		return output;
	}

	__PARTICLE_BOX_SHAPE_DESCInterop ScriptPARTICLE_BOX_SHAPE_DESC::ToInterop(const PARTICLE_BOX_SHAPE_DESC& value)
	{
		__PARTICLE_BOX_SHAPE_DESCInterop output;
		output.Type = value.Type;
		output.Extents = value.Extents;

		return output;
	}

}
