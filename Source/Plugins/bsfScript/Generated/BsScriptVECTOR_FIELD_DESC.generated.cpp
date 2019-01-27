//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVECTOR_FIELD_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVECTOR_FIELD_DESC::ScriptVECTOR_FIELD_DESC(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVECTOR_FIELD_DESC::initRuntimeData()
	{ }

	MonoObject*ScriptVECTOR_FIELD_DESC::box(const __VECTOR_FIELD_DESCInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__VECTOR_FIELD_DESCInterop ScriptVECTOR_FIELD_DESC::unbox(MonoObject* value)
	{
		return *(__VECTOR_FIELD_DESCInterop*)MonoUtil::unbox(value);
	}

	VECTOR_FIELD_DESC ScriptVECTOR_FIELD_DESC::fromInterop(const __VECTOR_FIELD_DESCInterop& value)
	{
		VECTOR_FIELD_DESC output;
		output.countX = value.countX;
		output.countY = value.countY;
		output.countZ = value.countZ;
		output.bounds = value.bounds;

		return output;
	}

	__VECTOR_FIELD_DESCInterop ScriptVECTOR_FIELD_DESC::toInterop(const VECTOR_FIELD_DESC& value)
	{
		__VECTOR_FIELD_DESCInterop output;
		output.countX = value.countX;
		output.countY = value.countY;
		output.countZ = value.countZ;
		output.bounds = value.bounds;

		return output;
	}

}
