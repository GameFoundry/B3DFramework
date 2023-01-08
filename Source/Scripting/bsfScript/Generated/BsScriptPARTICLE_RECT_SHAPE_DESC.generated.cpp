//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_RECT_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptParticleRectShapeOptions::ScriptParticleRectShapeOptions(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptParticleRectShapeOptions::InitRuntimeData()
	{ }

	MonoObject*ScriptParticleRectShapeOptions::Box(const __PARTICLE_RECT_SHAPE_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__PARTICLE_RECT_SHAPE_DESCInterop ScriptParticleRectShapeOptions::Unbox(MonoObject* value)
	{
		return *(__PARTICLE_RECT_SHAPE_DESCInterop*)MonoUtil::Unbox(value);
	}

	PARTICLE_RECT_SHAPE_DESC ScriptParticleRectShapeOptions::FromInterop(const __PARTICLE_RECT_SHAPE_DESCInterop& value)
	{
		PARTICLE_RECT_SHAPE_DESC output;
		output.Extents = value.Extents;

		return output;
	}

	__PARTICLE_RECT_SHAPE_DESCInterop ScriptParticleRectShapeOptions::ToInterop(const PARTICLE_RECT_SHAPE_DESC& value)
	{
		__PARTICLE_RECT_SHAPE_DESCInterop output;
		output.Extents = value.Extents;

		return output;
	}

}
