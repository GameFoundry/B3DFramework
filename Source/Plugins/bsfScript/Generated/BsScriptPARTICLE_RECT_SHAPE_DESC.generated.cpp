//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPARTICLE_RECT_SHAPE_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptPARTICLE_RECT_SHAPE_DESC::ScriptPARTICLE_RECT_SHAPE_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPARTICLE_RECT_SHAPE_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptPARTICLE_RECT_SHAPE_DESC::box(const __PARTICLE_RECT_SHAPE_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PARTICLE_RECT_SHAPE_DESCInterop ScriptPARTICLE_RECT_SHAPE_DESC::unbox(MonoObject* value)
	{
		return *(__PARTICLE_RECT_SHAPE_DESCInterop*)MonoUtil::unbox(value);
	}

	PARTICLE_RECT_SHAPE_DESC ScriptPARTICLE_RECT_SHAPE_DESC::fromInterop(const __PARTICLE_RECT_SHAPE_DESCInterop& value)
	{
		PARTICLE_RECT_SHAPE_DESC output;
		output.extents = value.extents;

		return output;
	}

	__PARTICLE_RECT_SHAPE_DESCInterop ScriptPARTICLE_RECT_SHAPE_DESC::toInterop(const PARTICLE_RECT_SHAPE_DESC& value)
	{
		__PARTICLE_RECT_SHAPE_DESCInterop output;
		output.extents = value.extents;

		return output;
	}

}
