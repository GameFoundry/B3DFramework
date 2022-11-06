//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptVECTOR_FIELD_DESC::InitRuntimeData()
	{ }

	MonoObject*ScriptVECTOR_FIELD_DESC::Box(const __VECTOR_FIELD_DESCInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__VECTOR_FIELD_DESCInterop ScriptVECTOR_FIELD_DESC::Unbox(MonoObject* value)
	{
		return *(__VECTOR_FIELD_DESCInterop*)MonoUtil::Unbox(value);
	}

	VECTOR_FIELD_DESC ScriptVECTOR_FIELD_DESC::FromInterop(const __VECTOR_FIELD_DESCInterop& value)
	{
		VECTOR_FIELD_DESC output;
		output.CountX = value.CountX;
		output.CountY = value.CountY;
		output.CountZ = value.CountZ;
		output.Bounds = value.Bounds;

		return output;
	}

	__VECTOR_FIELD_DESCInterop ScriptVECTOR_FIELD_DESC::ToInterop(const VECTOR_FIELD_DESC& value)
	{
		__VECTOR_FIELD_DESCInterop output;
		output.CountX = value.CountX;
		output.CountY = value.CountY;
		output.CountZ = value.CountZ;
		output.Bounds = value.Bounds;

		return output;
	}

}
